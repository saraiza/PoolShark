
#include "LoggerSplitter.h"



LoggerSplitter::LoggerSplitter(void)
{
}

LoggerSplitter::~LoggerSplitter(void)
{
	Clear();
}


LoggerSplitter::LoggerSplitter(ILogger *pInner1)
{
	Add(pInner1);
}

LoggerSplitter::LoggerSplitter(ILogger *pInner1, ILogger *pInner2)
{
	Add(pInner1);
	Add(pInner2);
}

LoggerSplitter::LoggerSplitter(ILogger *pInner1, ILogger *pInner2, ILogger *pInner3)
{
	Add(pInner1);
	Add(pInner2);
	Add(pInner3);
}

LoggerSplitter::LoggerSplitter(ILogger *pInner1, ILogger *pInner2, ILogger *pInner3, ILogger *pInner4)
{
	Add(pInner1);
	Add(pInner2);
	Add(pInner3);
	Add(pInner4);
}

void LoggerSplitter::ResetCache()
{
	m_ppLogs = m_loggers.data();
	m_iCount = m_loggers.count();
}


ILogger* LoggerSplitter::Add(ILogger *pInner)
{
	QWriteLocker locker(&m_lock);
	m_loggers.push_back(pInner);
		
	ResetCache();
	return pInner;
}


void LoggerSplitter::Start()
{
	QReadLocker locker(&m_lock);

	for (int i = 0; i < m_iCount; ++i)
		m_ppLogs[i]->Start();
}


void LoggerSplitter::Remove(ILogger *pInner)
{
	QWriteLocker locker(&m_lock);
	for(int i=0; i<m_loggers.count(); ++i)
	{
		if(pInner == m_loggers[i])
		{
			m_loggers.remove(i);
			ResetCache();
			return;
		} 
	}

	// It's not in there! But let's just assume the caller is being diligent or paranoid
	// and calling this too many times.
	//Q_ASSERT(false);
}

void LoggerSplitter::Clear(void)
{
	QWriteLocker locker(&m_lock);
	m_loggers.clear();
	ResetCache();
}


void LoggerSplitter::Log(
					int iLevel,
					Qt::HANDLE threadId,
					const char* pszSource,
					const char* pszCategory,
					const QString& msg)
{
	QReadLocker locker((QReadWriteLock*)&m_lock);	// De-const
	
	for(int i=0; i<m_iCount; ++i)
		m_ppLogs[i]->Log(iLevel, threadId, pszSource, pszCategory, msg);
}


