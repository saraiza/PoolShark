#pragma once
#include "Logging.h"

#include <QReadWriteLock>

namespace Logging {

/**
@brief Splitter connection for a log chain.

Allows the user to fork the log tree and send log messages to
multiple logs.
*/
class LoggerSplitter : public ILogger
{
public:
	LoggerSplitter(void);
	explicit LoggerSplitter(ILogger *pInner1);
	explicit LoggerSplitter(ILogger *pInner1, ILogger *pInner2);
	explicit LoggerSplitter(ILogger *pInner1, ILogger *pInner2, ILogger *pInner3);
	explicit LoggerSplitter(ILogger *pInner1, ILogger *pInner2, ILogger *pInner3, ILogger *pInner4);
	virtual ~LoggerSplitter(void);

	void Start() override;
	virtual QString GetLogFileName() override { return QString(); };
	virtual void Hint(Hints hint) override {};

	ILogger* Add(ILogger *pInner);
	void Remove(ILogger *pInner);
	void Clear(void);

	virtual void Log(
					int iLevel,
					Qt::HANDLE threadId,
					const char* pszSource,
					const char* pszCategory,
                    const QString& msg) override;

private:
	QVector<ILogger*> m_loggers;
	void ResetCache();	// Reload the optimization

	// Just a small optimization
    int m_iCount = 0;
    ILogger **m_ppLogs = nullptr;	// Pointer to a block of log pointers
	QReadWriteLock m_lock;
};


} // namespace 
