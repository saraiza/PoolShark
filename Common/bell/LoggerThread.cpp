#include "LoggerThread.h"
#include "Util.h"

using namespace Logging;

LoggerThread::LoggerThread()
	: Task("LoggerThread", Task::AutoRethrow)
{
}

LoggerThread::~LoggerThread(void)
{
	Stop();
}

void LoggerThread::Add(ILogger* pLogger)
{
	Q_ASSERT(nullptr == m_pNextLogger);
	m_pNextLogger = pLogger;
}


void LoggerThread::Start()
{
	Task::Start();
}

void LoggerThread::Stop()
{
	Task::StopSync();
}

void LoggerThread::Log(
						int iLevel,
						Qt::HANDLE threadId,
						const char* pszSource,
						const char* pszCategory,
						const QString& msg)
{
	LogEntry entry;
	entry.iLevel = iLevel;
	entry.timeStamp = QDateTime::currentDateTime();
	entry.threadId = threadId;
	entry.pszSource = pszSource;
	entry.pszCategory = pszCategory;
	entry.msg = msg;

	m_msgQueue.Push(entry);
}


void LoggerThread::RunTask()
{
	Q_ASSERT(m_pNextLogger);

	//Util::SetDebugThreadName("LoggerThread");
	QThread::currentThread()->setPriority(QThread::LowPriority);

	QElapsedTimer timerFlush;
	timerFlush.start();

	while (true)
	{
		// Pause. This also handles thread stopping
		Wait(m_msgQueue.GetHasData(), NO_TIMEOUT);

		ServiceQueue();
	}

	// One last time
	ServiceQueue();
	QThread::currentThread()->setPriority(QThread::NormalPriority);
}

void LoggerThread::ServiceQueue()
{
	// Timed out, so we can process the cached entries
	// Push the data to the downstream log
	// popping a concurrentQueue is atomic and thread-safe
	QVector<LogEntry> vectEntries = m_msgQueue.GetData();
	for(const LogEntry& entry : vectEntries)
		m_pNextLogger->Log(entry.iLevel, entry.threadId, entry.pszSource, entry.pszCategory, entry.msg);
}
