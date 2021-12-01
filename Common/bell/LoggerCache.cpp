
#include "LoggerCache.h"


using namespace Logging;

LoggerCache::LoggerCache(int iMaxCacheEntries /* = 0 */)
{
	m_iMaxEntries =  iMaxCacheEntries;
	m_bActive = iMaxCacheEntries > 0;
}


LoggerCache::~LoggerCache(void)
{
}

void LoggerCache::Log( int iLevel, Qt::HANDLE threadId, const char* pszSource, const char* pszCategory, const QString& msg )
{
	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);

	if(m_bActive && m_iMaxEntries > 0)
	{
		LogEntry entry(iLevel, QDateTime::currentDateTime(), threadId, pszSource, pszCategory, msg);
		m_lstCache.append(entry);
		if (m_iMaxEntries && m_lstCache.count() > m_iMaxEntries)
			m_lstCache.removeFirst();
	}
}

QList<LogEntry> LoggerCache::GetCachedEntries()
{
	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);
	QList<LogEntry> result(m_lstCache);
	m_lstCache.clear();
	return result;
}
