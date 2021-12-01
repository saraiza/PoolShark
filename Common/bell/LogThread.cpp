#include "LogThread.h"
//#include "Util.h"


LogThread::LogThread(QObject *pParent)
	: Task("LogThread", Task::AutoRethrow, pParent)
{
}


LogThread::~LogThread()
{
	Stop();
}


void LogThread::SetNext(ILog *pNextLog)
{
	m_pNextLog = pNextLog;
}

void LogThread::Init(std::chrono::milliseconds pollInterval)
{
	Q_ASSERT(m_pNextLog);

	m_pollInterval_ms = pollInterval;
}

void LogThread::Start()
{
	Q_ASSERT(m_pNextLog);
	m_pNextLog->Start();
	Task::Start();
}

void LogThread::Stop()
{
	Task::StopSync();
}

void LogThread::AutoFlush(std::chrono::milliseconds flushInterval)
{
	m_flushInterval_ms = flushInterval;
}

void LogThread::Log(const QString& line)
{
	m_msgQueue.Push(line);
}

void LogThread::Flush()
{
	// Pass it down the chain
	Q_ASSERT(m_pNextLog);
	m_pNextLog->Flush();
}

void LogThread::ServiceQueue()
{
	// Timed out, so we can process the cached entries
	// Push the data to the downstream log
	// popping a concurrentQueue is atomic and thread-safe
	QVector<QString> vectMsg = m_msgQueue.GetData();
	for(const QString& msg : vectMsg)
		m_pNextLog->Log(msg);
}

void LogThread::RunTask()
{
	Q_ASSERT(m_pNextLog);

	//Util::SetDebugThreadName("LogThread");
	QThread::currentThread()->setPriority(QThread::LowPriority);

	QElapsedTimer timerFlush;
	timerFlush.start();

	while(true)
	{
		// Pause. This also handles thread stopping
		Wait(m_msgQueue.GetHasData(), NO_TIMEOUT);

		ServiceQueue();

		// Handle auto-flushing
		if(0ms != m_flushInterval_ms)
		{
			if(timerFlush.elapsed() > m_flushInterval_ms.count())
			{
				// Push a flush request down the chain
				timerFlush.restart();
				m_pNextLog->Flush();
			}
		}
	}

	// One last time
	ServiceQueue();
	m_pNextLog->Flush();
	QThread::currentThread()->setPriority(QThread::NormalPriority);
}
