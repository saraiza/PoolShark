
#include "LogCache.h"


using namespace Logging;

LogCache::LogCache(int iMaxCacheEntries /* = 0 */)
{
	m_iMaxEntries =  iMaxCacheEntries;
}



void LogCache::SetActive(bool val)
{
	m_bActive = val; 
}


void LogCache::SetMaxEntries(int val)
{
	m_iMaxEntries = val;
}


void LogCache::Log(const QString& s)
{
	if (!m_bActive)
		return;

	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);

	m_slCache.append(s);
	if (m_slCache.count() > m_iMaxEntries)
		m_slCache.removeFirst();	// We don't adjust on the fly
}

QStringList LogCache::GetCachedEntries()
{
	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);
	return m_slCache;
}
