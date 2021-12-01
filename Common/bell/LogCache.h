#pragma once
#include "Logging.h"
#include "CriticalSection.h"

namespace Logging {

/**
@brief A log cacher for a log chain.

Stores the log entries in a cache for later use.

*/

class LogCache : public ILog
{
public:

    // default is no caching. (to log everything, set it to a negative number)
	LogCache(int iMaxCacheEntries = 0);

	void Start() override {}
    virtual void Log(const QString& line) override;
	virtual void Flush() override { } // do nothing
	virtual QString GetLogFileName() override { return QString(); };
	virtual void Hint(Hints hint) override {};


    // Returns all entries that are currently cached
    QStringList GetCachedEntries();


    inline bool isActive() const { return m_bActive; }
	void SetActive(bool val);

	inline int maxEntries() const { return m_iMaxEntries; }
	void SetMaxEntries(int val);

private:
    int m_iMaxEntries = 5;	///< Not a reasonable default
    bool m_bActive = true;
    CriticalSection m_cs;
    QStringList m_slCache;
};


}
