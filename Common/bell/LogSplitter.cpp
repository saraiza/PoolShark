
#include "LogSplitter.h"

LogSplitter::LogSplitter(void)
{
}

LogSplitter::~LogSplitter(void)
{
	Clear();
}


LogSplitter::LogSplitter(ILog *pInner1)
{
	Add(pInner1);
}
LogSplitter::LogSplitter(ILog *pInner1, ILog *pInner2)
{
	Add(pInner1);
	Add(pInner2);
}

LogSplitter::LogSplitter(ILog *pInner1, ILog *pInner2, ILog *pInner3)
{
	Add(pInner1);
	Add(pInner2);
	Add(pInner3);
}

LogSplitter::LogSplitter(ILog *pInner1, ILog *pInner2, ILog *pInner3, ILog *pInner4)
{
	Add(pInner1);
	Add(pInner2);
	Add(pInner3);
	Add(pInner4);
}




ILog* LogSplitter::Add(ILog *pInner)
{
	QWriteLocker locker(&m_lock);
	m_logs.push_back(pInner);

	m_ppLogs = m_logs.data();
	m_iCount = m_logs.count();

	return pInner;
}


void LogSplitter::Remove(ILog *pInner)
{
	QWriteLocker locker(&m_lock);
	for(int i=0; i<m_logs.count(); ++i)
	{
		if(pInner == m_logs[i])
		{
			m_logs.remove(i);
			m_ppLogs = m_logs.data();
			m_iCount = m_logs.count();
			return;
		}
	}
	//Q_ASSERT(false);
}

void LogSplitter::Clear(void)
{
	QWriteLocker locker(&m_lock);
	m_logs.clear();
	m_ppLogs = NULL;
	m_iCount = 0;
}


void LogSplitter::Log(const QString& msg)
{
	QReadLocker locker((QReadWriteLock*)&m_lock);	// De-const

	for(int i=0; i<m_iCount; ++i)
		m_ppLogs[i]->Log(msg);
}

void LogSplitter::Flush()
{
	QReadLocker locker((QReadWriteLock*)&m_lock);	// De-const

	for(int i=0; i<m_iCount; ++i)
		m_ppLogs[i]->Flush();
}



void LogSplitter::Start() 
{
	QReadLocker locker((QReadWriteLock*)&m_lock);	// De-const

	// Fan it out
	for (int i = 0; i < m_iCount; ++i)
		m_ppLogs[i]->Start();
}


QString LogSplitter::GetLogFileName()
{
	QReadLocker locker((QReadWriteLock*)&m_lock);	// De-const

	// Fan it out
	for (int i = 0; i < m_iCount; ++i)
	{
		QString sLogFileName = m_ppLogs[i]->GetLogFileName();
		if (!sLogFileName.isEmpty())
			return sLogFileName;
	}
	return QString();
}

void LogSplitter::Hint(Hints hint)
{
	QReadLocker locker((QReadWriteLock*)&m_lock);	// De-const

// Fan it out
	for (int i = 0; i < m_iCount; ++i)
		m_ppLogs[i]->Hint(hint);
}