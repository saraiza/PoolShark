#include "Task.h"
#include "Logging.h"
#include "Finally.h"
#include "Macros.h"
#include <QThread>
#include <QCoreApplication>
#include <QMetaMethod>


#ifdef Q_OS_WIN
#include <windows.h>
#endif

DECLARE_LOG_SRC("Task", LOGCAT_Common);


#ifdef QT_DEBUG
    #define THREAD_MAX_BIRTH_TIME_MS    30000
#else
    #define THREAD_MAX_BIRTH_TIME_MS    5000
#endif




class ThreadRunner : public QThread
{
public:
	ThreadRunner(std::function<void(void)> func) : m_func(func) {
		Q_ASSERT(m_func);
	}

	/// Expose the protected QThread::exec method
	int exec() {
		return QThread::exec();
	}

private:
	std::function<void(void)> m_func;
	virtual void run() override {
		m_func();
	}
};



Task::Task(QString sTaskName, AutoRethrowMethod autoRethrowMethod, QObject *parent)
    : QObject(parent)
	, m_sTaskName(sTaskName)
	, m_autoRethrowDelegate(autoRethrowMethod == AutoRethrow, parent)
{
}

Task::~Task(void)
{
	//We need to make sure the thread exits before we clean up the class members
	//May seem like a duplication of the ExceptionProxyThread destructor, but it is not.
	StopSync();
}

int Task::exec()
{
	Q_ASSERT(m_pThread);
	ThreadRunner *pThreadRunner = (ThreadRunner*)m_pThread;
	return pThreadRunner->exec();
}

void Task::TEST_SetThreadCreateDelayMs(int iMs)
{
	m_test.iThreadCreateDelayMs = iMs;
}

void Task::TEST_SetThreadDieDelayMs(int iMs)
{
	m_test.iThreadDieDelayMs = iMs;
}

void Task::TEST_SetThreadDebugId(int iId)
{
	m_test.iDebugId = iId;
}


void Task::DisableExceptionHandlingAssert()
{
	LOGINFO("Turning off ThreadException handling check for class '%s'", qPrintable(m_sTaskName));
	m_bCheckThreadExceptionConnects = false;
}

void Task::Start()
{
	// This is a super special check to make sure main.cpp
	// remembered to setup logging before starting threads
	// Make sure to call Task::Init in your main prior
	// to starting threads
	Q_ASSERT(Logging::LoggingSystem::IsStarted());

	// This is anoter super special check to make sure main.cpp
	// remembered to declare QApplication before starting threads
	// Threads that use signals and slots will stop working otherwise
    Q_ASSERT(QCoreApplication::instance() != nullptr);

	QMutexLocker lock(&m_csStart);

	// One-time operation to setup the delegate QObject for the auto-rethrow feature
	if (!m_bAutoRethrowDelegateMovedToAppThread)
	{
		// This will make sure all unhandled exceptions are
		// thrown on the main event loop, stacking tasks
		// will generally block events
		if (qApp)
		{
			m_autoRethrowDelegate.moveToThread(qApp->thread());
		}
		else
		{
			// For loggerthread and logthread, both get initialized before a valid QApplication,
			// for now just disable rethrowing errors in those cases
			qWarning() << "WARNING, your thread will not work with auto rethrow";
		}
		m_bAutoRethrowDelegateMovedToAppThread = true;
	}

	Q_ASSERT(IsRunning() == m_evtRunning.IsSet());

	// if the thread is already started, your doing it wrong
	if (IsRunning())
	{
		LOGWRN("Task %s %u already running", qPrintable(m_sTaskName), this);
		return;
	}

	m_evtStopReq.ResetEvent();
	emit Starting();

	m_pThread = new ThreadRunner(std::bind(&Task::Thread, this));
	m_pThread->setObjectName(m_sTaskName);
	VERIFY(connect(m_pThread, &QThread::finished, m_pThread, &QObject::deleteLater));

	if (m_bCheckThreadExceptionConnects && !m_autoRethrowDelegate.m_bAutoRethrow)
	{
		// If you hit this assert, then you have forgotten about exception handling for your
		// threaded object. Don't forget about errors! They get lonely and need your attention. Maybe a hug.
		static const QMetaMethod sigTE = QMetaMethod::fromSignal(&Task::ThreadException);
		Q_ASSERT(isSignalConnected(sigTE));
	}

	// Start the task
	TaskStarting();
	m_pThread->start();
    if (m_evtRunning.Wait(THREAD_MAX_BIRTH_TIME_MS))
        return;

	// If the task doesn't start, it might be in an unrecoverable
	// state.. or something else idk, but we should check this.
	// This will most likely never be tossed unless something really went wrong
	m_pThread->terminate(); // this is unsafe as it will skip any clean up of the task
	delete m_pThread;
	m_pThread = nullptr;
	EXERR("Y2QB", "Task %s did not start", qPrintable(m_sTaskName));
}

void Task::StopAsync()
{
	m_evtStopReq.SetEvent();
}

void Task::StopSync()
{
	if(IsRunning() && m_pThread)
		m_pThread->quit();	///< Only does something for threads using an event loop (exec).
	m_evtStopReq.SetEvent();
	WaitForFinished();
}

void Task::WaitForFinished()
{
	// Don't wait for the same thread, that would cause a deadlock. This can
	// happen if it is the thread that wants to send a stop request to itself.
	auto me = ThreadId();
	auto CurrentThread = QThread::currentThreadId(); // QThread works with std::thread, it makes a Q(Adopted)Thread instace
	if (me == CurrentThread)
		return;

	m_evtRunning.WaitNot();
}

const WinEventManualResetPair &Task::RunningEvent() const
{
	return m_evtRunning;
}

void Task::CheckAbort()
{
	if (m_evtStopReq.Wait(0))
		throw ExceptionStopReq("Task", __LINE__);
}

bool Task::IsRunning()
{
	return m_evtRunning.IsSet();
}


bool Task::IsSucceeded() const
{
	return m_bSucceeded;
}

void Task::Thread()
{
	if (m_test.iThreadCreateDelayMs)
        QThread::msleep(m_test.iThreadCreateDelayMs);

	OnStarted();
	FINALLY(OnStopped());
	LOGINFO("Thread %s started id (0x%08X/%d)", qPrintable(m_sTaskName), m_threadHandle, m_threadHandle);

	// Always setup the auto-rethrow connection before staring the thread. We used to do this early on, but doing it
	// last-moment makes it almost impossible for the user to accidentally mess up our internally-needed signal-slot
	// connection
	if(m_autoRethrowDelegate.m_bAutoRethrow)
		VERIFY(connect(this, &Task::ThreadException, &m_autoRethrowDelegate, &AutoRethrowDelegate::_OnThreadException));

	// Always disconnect from that signal. No need for a verify (that would be bad).
	FINALLY(disconnect(this, &Task::ThreadException, &m_autoRethrowDelegate, &AutoRethrowDelegate::_OnThreadException));
	
	try
	{
		// Call the derived classes' implementation
		//  Initial state (such as m_bSucceeded is initialized in OnStarted() )
		RunTask();
		m_bSucceeded = true;
	}
	catch (const ExceptionStopReq&)
	{
		emit Aborted();
		LOGINFO("Stop request for Task '%s'", qPrintable(m_sTaskName));
	}
	catch (...)
	{
		m_exLastException = ExceptionContainer::CurrentException();
		emit ThreadException(m_exLastException);
	}

	try
	{
		bool bStopSet = m_evtStopReq.IsSet();
		m_evtStopReq.ResetEvent();	// Just in case cleanup wants to call Wait()

		TaskCleanup();

		//Ensure the stop event is re-set after cleanup
		if (bStopSet)
			m_evtStopReq.SetEvent();
	}
	catch (const ExceptionStopReq&)
	{
		emit Aborted();
		LOGINFO("Stop request for Task '%s'", qPrintable(m_sTaskName));
	}
	catch (...)
	{
		LOGWRN("Exception occured while cleaning up task");
		emit ThreadException(ExceptionContainer::CurrentException());
	}

	// Inject some test/debug behavior
	if (m_test.iThreadDieDelayMs)
        QThread::msleep(m_test.iThreadDieDelayMs);

	if (m_bSucceeded)
		emit Succeeded();
}

void Task::RunSync()
{
	Q_ASSERT(!IsRunning());
	m_evtRunning.SetEvent();
	FINALLY(m_evtRunning.ResetEvent());

	// This is a super special check to make sure main.cpp
	// remembered to setup logging before starting threads
	// Make sure to call Task::Init in your main prior
	// to starting threads
	Q_ASSERT(Logging::LoggingSystem::IsStarted());

	// This is anoter super special check to make sure main.cpp
	// remembered to declare QApplication before starting threads
	// Threads that use signals and slots will stop working otherwise
	Q_ASSERT(qApp != nullptr);

	{
		QMutexLocker lock(&m_csStart);
		m_evtStopReq.ResetEvent();
		emit Starting();
	}


	bool bException = false;
	try
	{
		// Call the derived classes' implementation
		//  Initial state (such as m_bSucceeded is initialized in OnStarted() )
		OnStarted();
		RunTask();
		m_bSucceeded = true;
	}
	catch (const ExceptionStopReq&)
	{
		emit Aborted();
		bException = true;
		m_exLastException = ExceptionContainer::CurrentException();
		LOGINFO("Stop request for Task '%s'", qPrintable(m_sTaskName));
	}
	catch (...)
	{
		bException = true;
		m_exLastException = ExceptionContainer::CurrentException();
		emit ThreadException(m_exLastException);
	}


	try
	{
		bool bStopSet = m_evtStopReq.IsSet();
		m_evtStopReq.ResetEvent();	// Just in case cleanup wants to call Wait()

		TaskCleanup();

		//Ensure the stop event is re-set after cleanup
		if (bStopSet)
			m_evtStopReq.SetEvent();
	}
	catch (const ExceptionStopReq&)
	{
		emit Aborted();
		LOGINFO("Stop request for Task '%s'", qPrintable(m_sTaskName));
	}
	catch (...)
	{
		LOGWRN("Exception occured while cleaning up task");
		emit ThreadException(ExceptionContainer::CurrentException());
	}

	// Rethrow the exception to the caller. We are living in the caller's thread.
	if(bException)
		m_exLastException.Rethrow();
}

const ExceptionContainer& Task::LastException() const
{
	return m_exLastException;
}

const WinEventManualReset & Task::StopReqEvent() const
{
	return m_evtStopReq;
}


/// Derived classes should call this in RunTask to delay/pause.
void Task::SleepMs(int iMs)
{
	bool bStopReq = m_evtStopReq.Wait(iMs);
	if (bStopReq)
		throw ExceptionStopReq("Task", __LINE__);
}

/// Returns 0 to match a fired event, -1 for a timeout
int Task::Wait(const IWinEvent &hEvt0, int iMS)
{
	int iResult = WinEvent::Wait(m_evtStopReq, hEvt0, iMS);
	switch (iResult)
	{
	case 0:
		throw ExceptionStopReq("Task", __LINE__);

	case 1:
		return 0;	// Index 0 from the caller's point of view

	case -1: // default timeout
	default:
		return -1;
	}
}


int Task::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, int iMS)
{
	int iResult = WinEvent::Wait(m_evtStopReq, hEvt0, hEvt1, iMS);
	switch (iResult)
	{
	case 0:
		throw ExceptionStopReq("Task", __LINE__);

	case 1:
		return 0;	// Index 0 from the caller's point of view
	case 2:
		return 1;	// Index 1 from the caller's point of view

	case -1: // default timeout
	default:
		return -1;
	}
}


int Task::Wait(const IWinEvent & hEvt0, const IWinEvent & hEvt1, const IWinEvent & hEvt2, int iMS)
{
	int iResult = WinEvent::Wait(m_evtStopReq, hEvt0, hEvt1, hEvt2, iMS);
	switch (iResult)
	{
	case 0:
		throw ExceptionStopReq("Task", __LINE__);

	case 1:
		return 0;	// Index 0 from the caller's point of view
	case 2:
		return 1;	// Index 1 from the caller's point of view
	case 3:
		return 2;	// Index 2 from the caller's point of view

	case -1: // default timeout
	default:
		return -1;
	}
}

int Task::Wait(const IWinEvent & hEvt0, const IWinEvent & hEvt1, const IWinEvent & hEvt2, const IWinEvent & hEvt3, int iMS)
{
	int iResult = WinEvent::Wait(m_evtStopReq, hEvt0, hEvt1, hEvt2, hEvt3, iMS);
	switch (iResult)
	{
	case 0:
		throw ExceptionStopReq("Task", __LINE__);

	case 1:
		return 0;	// Index 0 from the caller's point of view
	case 2:
		return 1;	// Index 1 from the caller's point of view
	case 3:
		return 2;	// Index 2 from the caller's point of view
	case 4:
		return 3;	// Index 3 from the caller's point of view

	case -1: // default timeout
	default:
		return -1;
	}
}

Qt::HANDLE Task::ThreadId() const 
{ 
	return m_threadHandle;
}


//void Task::OnThreadException(const ExceptionContainer & ex)
//{
//	m_autoRethrowDelegate.EmitException(ex);
//}

void Task::OnStarted()
{
	// Called by direct connection
	m_bSucceeded = false;
	m_threadHandle = QThread::currentThreadId();
	m_evtRunning.SetEvent();
	emit Started();
}

void Task::OnStopped()
{
	// Called by direct connection
	m_threadHandle = 0;
	m_evtRunning.ResetEvent();
	emit Completed();
}

void Task::SetAutoRethrow(AutoRethrowMethod method)
{
	Q_ASSERT(!IsRunning());
	m_autoRethrowDelegate.SetAutoRethrow(method == Task::AutoRethrow);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


AutoRethrowDelegate::AutoRethrowDelegate(bool bEnableAutoRethrow, QObject* parent)
	: QObject(parent)
{
	SetAutoRethrow(bEnableAutoRethrow);
}


void AutoRethrowDelegate::SetAutoRethrow(bool bEnable)
{
	m_bAutoRethrow = bEnable;
}

void AutoRethrowDelegate::_OnThreadException(const ExceptionContainer & ex)
{
	// This is only called if we connected to the signal. Controlled by m_bAutoRethrow.
	ex.Rethrow();
}
