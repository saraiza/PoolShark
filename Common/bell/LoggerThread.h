#pragma once
#include "Logging.h"
#include "Task.h"
#include "ConcurrentQueue.h"
#include "LogEntry.h"

namespace Logging
{
	/**
	@brief Log threading

	This provides an asynchronous log message buffer.
	*/
	class LoggerThread : public Task, public ILogger
	{
	public:
		LoggerThread();
		virtual ~LoggerThread();

		void Add(ILogger* pLogger);	///< The downstream logger

		void Start() override;
		void Stop();
		virtual QString GetLogFileName() override { return QString(); };
		virtual void Hint(Hints hint) override {};

		void Log(
			int iLevel,
			Qt::HANDLE threadId,
			const char* pszSource,
			const char* pszCategory,
			const QString& msg) override;

	private:
		void RunTask() override;
		ILogger* m_pNextLogger = nullptr; 

		ConcurrentQueue<LogEntry> m_msgQueue;
		void ServiceQueue();
	};
}

