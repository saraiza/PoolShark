#pragma once
#include "Logging.h"

#include <QReadWriteLock>

namespace Logging {

/**
@brief Splitter connection for a log chain.

Allows the user to fork the log tree and send log messages to
multiple logs.
*/
class LogSplitter : public ILog
{
public:
	LogSplitter(void);
	LogSplitter(ILog *pInner1);
	LogSplitter(ILog *pInner1, ILog *pInner2);
	LogSplitter(ILog *pInner1, ILog *pInner2, ILog *pInner3);
	LogSplitter(ILog *pInner1, ILog *pInner2, ILog *pInner3, ILog *pInner4);
	virtual ~LogSplitter(void);

	ILog* Add(ILog *pInner);
	void Remove(ILog *pInner);
	void Clear(void);

	virtual void Log(const QString& msg) override;
	virtual void Flush() override;
	void Start() override;
	virtual QString GetLogFileName() override;
	virtual void Hint(Hints hint) override;


private:
	QVector<ILog*> m_logs;

	// Just a small optimization
    int m_iCount = 0;
    ILog **m_ppLogs = nullptr;	// Pointer to a block of log pointers
	QReadWriteLock m_lock;
	QThread * m_thread = nullptr;
};


} // namespace
