#pragma once
#include "Logging.h"
#include "ConcurrentQueue.h"
#include <mutex>
#include "Task.h"
#include <chrono>

using namespace std::chrono_literals;


namespace Logging {

/**
@brief Log threading

This provides an asynchronous log message buffer.
*/
class LogThread : private Task, public ILog
{
public:
	LogThread(QObject *pParent = nullptr);
	virtual ~LogThread();

	void SetNext(ILog *pNextLog);

	void Init(std::chrono::milliseconds iPollIntervalMs = 1000ms);
	void Stop();

	void AutoFlush(std::chrono::milliseconds flushInterval = 0ms);

	void Log(const QString& line) override;
	void Flush() override;
	void Start() override;
	virtual QString GetLogFileName() override { return QString(); };
	virtual void Hint(Hints hint) override {};


private:
	void ServiceQueue();
	void RunTask();

private:
	ILog *m_pNextLog = nullptr;

	std::chrono::milliseconds m_flushInterval_ms = 8000ms;
	std::chrono::milliseconds m_pollInterval_ms = 300ms;
	ConcurrentQueue<QString> m_msgQueue;
};

} // namespace
