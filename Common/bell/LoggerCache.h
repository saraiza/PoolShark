#pragma once
#include "Logging.h"
#include "LogEntry.h"
#include "CriticalSection.h"
#include <QList>


namespace Logging {

/**
@brief A log cacher for a log chain.

Stores the log entries in a cache for later use.

*/

class LoggerCache :
    public ILogger
{
public:

    // default is no caching. (to log everything, set it to a negative number)
    LoggerCache(int iMaxCacheEntries = 0);
    virtual ~LoggerCache(void);

	void Start() override {}
    virtual void Log( int iLevel, Qt::HANDLE threadId, const char* pszSource, const char* pszCategory, const QString& msg );
	virtual QString GetLogFileName() override { return QString(); };
	virtual void Hint(Hints hint) override {};

    // Returns all entries that are currently cached and clears the cache
    QList<LogEntry> GetCachedEntries();


    bool isActive() const { return m_bActive; }
    void SetActive(bool val)  { m_bActive = val; }

    int maxEntries() const { return m_iMaxEntries; }
    void SetMaxEntries(int val) { m_iMaxEntries = val; }

private:
    int m_iMaxEntries;
    bool m_bActive;
    CriticalSection m_cs;
	QList<LogEntry> m_lstCache;

};


}
