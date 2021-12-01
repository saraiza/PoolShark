#pragma once
#include <QObject>
#include "WinEvent.h"
#include "Exception.h"
#include <QMutex>
#include "Defs.h"

/**
@brief Handles the exception auto-rethrow feature

This class is a separate QObject instance that we always move to the main thread.
We assume that auto-rethrow will always want to go there.
*/
class AutoRethrowDelegate : public QObject
{
	Q_OBJECT
public:
	AutoRethrowDelegate(bool bEnableAutoRethrow, QObject *pParent);
	void SetAutoRethrow(bool bEnable);

public slots:
	/// This special slot is hooked up by Task to give this object
	/// a chance to handle AutoRethrow.
	virtual void _OnThreadException(const ExceptionContainer &ex);

public:
	bool m_bAutoRethrow = false;
};

/**
@brief Base class for creating a relatively time-consuming job/process management object.

EzStarterOld, GDI Tuner, SyScheck, are all examples of classes that fit this behavior.  This class
attempts to unify the plumbing behind writing a new task and getting to where you can implement
the details within a single run method.

Exceptions can happen in the internal thread. Unless you catch the exceptions in your
RunThread implementation, your thread will die.

Some of the wait methods have been inlined at the request of the profiler.
*/
class Task : public QObject
{
	Q_OBJECT
public:

	class ExceptionStopReq : public Exception
	{
	public:
		ExceptionStopReq()
			: Exception("Stop Requested")
		{
		}

		/// This constructor needs to be used to get correct error codes with source and line number.
		ExceptionStopReq(QString sSource, int iLineNum)
			: Exception("Stop Requested", sSource, iLineNum)
		{
		}
	};


	/// Task requires that you declare upfront how it should handle exceptions.
	/// This enum is passed into the constructor to force you to explicitly declare.
	enum AutoRethrowMethod {
		AutoRethrow,		///< Automatically rethrow all exceptions in the main thread
		NoAutoRethrow,		///< Don't automatically rethrow (but the code will still make sure you connect to the ThreadException signal)
	};

	Task(QString sTaskName, AutoRethrowMethod autoRethrowMethod, QObject *parent = 0);
	virtual ~Task(void) override;
	void SetAutoRethrow(AutoRethrowMethod method);

	/// Internally, Task will throw an assert if it detects that you have not connected
	/// a slot to the ThreadException signal (unless you are using AutoRethrow).  There are
	/// special cases where it's ok to not do that. In these cases, you may call this method
	/// to disable that check.  DO NOT DO THIS JUST BECAUSE YOU ARE TOO LAZY TO HOOKUP
	/// EXCEPTION/ERROR HANDLING!
	void DisableExceptionHandlingAssert();
	const ExceptionContainer& LastException() const;

	virtual void Start();			///< Start the task. Derived classes may override.
	virtual void StopSync();		///< Stop the task and synchronously wait for it to finish. Derived classes may override.
	virtual void WaitForFinished();
	virtual void StopAsync();		///< Fire a stop request, but don't wait. You can call Stop() afterwards to wait for it to finish.

	/// Run the task completely synchronously. TaskCleanup() is still called on error. Your
	/// derived class must not have an endless while-loop to use this feature.
	void RunSync();

	bool IsRunning();
	bool IsSucceeded() const;

	/// Expose the running event so that other tasks can wait
	/// for this task to either start running, or stop running.
	/// The event is const, so external users cannot set or re-set it.
	const WinEventManualResetPair &RunningEvent() const;

	/// Expose the stop event for external blah blah...
	const WinEventManualReset &StopReqEvent() const;

	/// Inject extra delays to force a failure. FOR TESTING
	void TEST_SetThreadCreateDelayMs(int iMs);
	void TEST_SetThreadDieDelayMs(int iMs);
	void TEST_SetThreadDebugId(int iId);

signals:
	// Public
	void Starting();            ///< The task is tarting, but not running yet
	void Started();             ///< The task is running, IsRunning() will return true
	void Completed();			///< ALWAYS fired, error, abort, or success!
	void Aborted();				///< Only fired if the task was aborted
	void Succeeded();			///< Only fired if the task completed without error
	void ThreadException(ExceptionContainer ex);	///< For backwards compatibility

protected slots:
	void OnStarted();
	void OnStopped();

protected:
	virtual void RunTask() = 0;		///< Your derived class must implement this

	virtual void TaskStarting() {}	///< Thread is just about to be launched
	virtual void TaskCleanup() {}	///< Derived classes may implement this for cleanup after completion or error

	/// If an abort request has been submitted, this will throw the exception. This
	/// method is public because there are times when an external bit of code 'knows' that it
	/// is running in the thread of a parent task and wants to play nicely with abort/cancel.
	/// Calling SleepMs(0) does this too.
	void CheckAbort();

	/// Derived classes should call this in RunTask to delay/pause.
	void SleepMs(int iMs);

	/// Returns 0 to match a fired event, -1 for a timeout
	int Wait(const IWinEvent& hEvt0, int iMS = NO_TIMEOUT);
	int Wait(const IWinEvent& hEvt0, const IWinEvent& hEvt1, int iMS = NO_TIMEOUT);
	int Wait(const IWinEvent& hEvt0, const IWinEvent& hEvt1, const IWinEvent& hEvt2, int iMS = NO_TIMEOUT);
	int Wait(const IWinEvent& hEvt0, const IWinEvent& hEvt1, const IWinEvent& hEvt2, const IWinEvent& hEvt3, int iMS = NO_TIMEOUT);

	Qt::HANDLE ThreadId() const;

	int exec();	///< Redirects to QThread::exec for running an event loop in your thread

    QThread* m_pThread = nullptr;

private:
	QString m_sTaskName;
	ExceptionContainer m_exLastException;	///< The last exception caught

	volatile bool m_bSucceeded = false;


	void Thread();

	Qt::HANDLE m_threadHandle = nullptr;

	QMutex m_csStart;

	/// TESTING ONLY! This structure is for injecting testable behavior for our unit tests.
	struct {
		int iThreadCreateDelayMs = 0;
		int iThreadDieDelayMs = 0;
		int iDebugId = 0;
	} m_test;

	WinEventManualReset m_evtStopReq;
	mutable WinEventManualResetPair m_evtRunning;		///< state of the task

	AutoRethrowDelegate m_autoRethrowDelegate;			///< Separate instance of a QObject that ALWAYS lives on GUI thread. This object handles auto-rethrow.
	bool m_bCheckThreadExceptionConnects = true;		///< Controls if we check for connected slots to the ThreadException signal
	bool m_bAutoRethrowDelegateMovedToAppThread = false;	///< Move it just once
};


