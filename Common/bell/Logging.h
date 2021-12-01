#pragma once
/** @file Logging.h

	@brief #include for the logging system.

	Logging.h contains all the macros you need to send messages to the logging system.

*/


#include "Exception.h"
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>

/**
@brief This namespace contains all things related to our logging system.

Logging
=======

We have our own custom logging system at 908 Devices.  Before going further, here is a quick usage example
snippet:

~~~~~~~~~~~~~~~~~~~~~{.c}
DECLARE_LOG_SRC("MyClass", LOGCAT_Common);

void MyClass::foo()
{
	LOGERR("%s says hello", "Steve");
}
~~~~~~~~~~~~~~~~~~~~~

If you don't feel like reading further, then all you really need to know is to use the logging
and exeption macros. LOGINFO(), LOGWRN(), LOGERR(), EXERR(), EXWRN(), and EXINFO(). There
are many examples in the code, follow them.

The main goal of the logging design is to separate the act of logging from what happens
to the log messages. All log messages are sent to a global object through the
logging macros.

Exceptions
==========
We throw exceptions all over the place in our code. Most are for error conditions.

Error Codes
-----------
Every Error codes are displayed to the user, and then sometimes phoned back in to us
through the support system.  For that reason, we have tried to make them as short
as possible and easy to say over the phone.  We used to have a more complex system
for this, but now we just have a randomized code string and we just rely on a search
of the codebase to find where the code is referring to. This has worked out really
well.

Error code strings are limited to these characters: ABCDEFGHJKLMNPRSTUVWXYZ123456789

*/
namespace Logging {

/// The 'level' of a log entry.
enum LOGLEVEL : int
{
	LL_None = 0,
	LL_Error = 1,
	LL_Warn = 2,
	LL_Info = 3,
	LL_Debug = 4,

	// This is a special case. If you use this, it will ALWAYS get printed, but only in a debug build
	LL_DevOnly = 0
};

enum Hints {
	Scanning,
	NotScanning
};

/// The ILogger defines the actual object that log messages will be written to. You won't actually use
/// this interface directly to write log messages, there are macros that make it easier (see below).
class ILogger
{
public:
	// The primary log method in an ILogger class.  Each ILogger class must provide their own implementation
	// NOTE: If an inner logger exists, the implementation MUST call the Log method of the inner logger.
	virtual void Log(
                        int iLevel,
                        Qt::HANDLE threadId,
                        const char* pszSource,
                        const char* pszCategory,
                        const QString& msg) = 0;

	virtual void Start() = 0;	///< Start the log thing. The implementation should start the downstream log thing first.
	virtual QString GetLogFileName() = 0;
	virtual void Hint(Hints hint) = 0;

};



/// Stores formatted log data.
/**
ILog takes formatted log data and stores or outputs it. Splitter and caching implemetations
would likely implement ILog instead of ILogger for simplicity and efficiency.
*/
class ILog
{
public:
	/// Write a single log entry to the 'log'
	virtual void Log(const QString& line) = 0;

	/// Flush the data that might be cached. If you don't support that, forward down the chain (if there is one)
	virtual void Flush() = 0;

	virtual void Start() = 0;	///< Start the log thing. The implementation should start the downstream log thing first.

    virtual QString GetLogFileName() = 0;
	virtual void Hint(Hints hint) = 0;

};



/**
@brief The singleton logging system

This class just has static methods to protect and sync setup
of the logging system.
*/
class LoggingSystem
{
public:
	static void Start(ILogger *pLogger);
	static void Reset();	///< Restore to the default logger (not really necessary to call)
	static ILogger* GetLogger();

	static bool IsStarted();
	static LOGLEVEL GetMaxLevel();
	static void SetMaxLevel(LOGLEVEL maxLevel);

	static void Hint(Hints hint);

	static QString GetLogFileName();
private:
	static LOGLEVEL ms_maxLevel;
	static ILogger *ms_pLogger;	///< The top-level start of the logging chain for the app
	static bool ms_bStarted;
};



/// Implements the Log Source DB
/** This class builds a database of known logging sources in a particular
 module.  With this, we can provide the developer with a list of known
 sources from the current codebase grouped by categories. Loggers can
 then use this to show the developer what he/she can filter with. */
#define MAX_SOURCES		1200
class LogSourceDB
{
public:
	typedef struct tagENTRY
	{
		const char* pszSrc;
		const char* pszCategory;
	} ENTRY;

	LogSourceDB(const char* pszSource, const char* pszCategory);

	static int GetSourceCount();
	static ENTRY* GetSources();
};

class RegisterNameForExceptionId
{
public:
	RegisterNameForExceptionId(QString name);
};

/// We don't want to expose raw exception errors to the user. Instead, we show the user a exception error derived
/// from the module name and line number. These two functions map between ids and module names/line numbers.
quint64 GetExceptionId(QString moduleName, quint16 lineNumber);
QPair<QString, quint16> GetNameAndLineNumberFromExceptionId(quint64 exceptionId);

/*********************************************************************/
/***************************** MACROS ********************************/
/*********************************************************************/

// Every log message provides a source. You must declare the
// logging source using this macro
#define DECLARE_LOG_SRC(src, category) \
	static const char* gs_psz_DECLARE_LOG_SRC_Macro_is_missing = src; \
	static const char* gs_pszLocalLogCategory = category; \
	static LogSourceDB gs_logSource(src, category); \
	static RegisterNameForExceptionId gs_registerNameForExceptionId(src);

// I use the funny gs_psz_DECLARE_LOG_SRC_Macro_is_missing name to give you a
//  compile error that tells you what the problem is and what to do. Please
//  don't simplify that variable name!
////////////////////////////////////////////////////////////////////////

/// internal helper functions for macros
bool _LgMsgHlp(LOGLEVEL level,
			const char* pszMethod, const char* pszSource,
			const char* pszCategory, uint lineNumber,
			const char* pszFormat, ...);

// Overloaded QString versions
bool _LgMsgHlp(LOGLEVEL level,
			const char* pszMethod, const char* pszSource,
			const char* pszCategory, uint lineNumber,
			const QString& msg);

class LoggerQDebug : public QDebug
{
public:
	LoggerQDebug(LOGLEVEL level,
		const char* pszMethod, const char* pszSource,
		const char* pszCategory, uint lineNumber)
		: QDebug(&m_sMsg)
		, m_level(level)
		, m_sMethod(pszMethod)
		, m_sSource(pszSource)
		, m_sCategory(pszCategory)
		, m_uiLineNumber(lineNumber) {}

	~LoggerQDebug() { _LgMsgHlp(m_level, m_sMethod, m_sSource, m_sCategory, m_uiLineNumber, m_sMsg); }

private:
	QString m_sMsg;

	LOGLEVEL m_level;
	const char *m_sMethod = nullptr;
	const char *m_sSource = nullptr;
	const char *m_sCategory = nullptr;
	uint m_uiLineNumber;
};

template<class T>
T _LgMsgException(T ex, const char* pszErrCode, const char* pszFormat, ...)
{
	// Process the string formatting
	va_list args;
	va_start(args, pszFormat);
	char szMsg[4096];

#ifdef Q_OS_WIN
	_vsnprintf_s(szMsg, sizeof(szMsg), (sizeof(szMsg) / sizeof(szMsg[0]) - 1), pszFormat, args);
#else
	vsnprintf(szMsg, sizeof(szMsg), pszFormat, args);
#endif

	va_end(args);
	szMsg[sizeof(szMsg) - 1] = 0; // null terminate to be safe

	ex.SetMsg(szMsg, pszErrCode);
	return ex;
};

template<class T>
T _LgMsgException(T ex, const char* pszErrCode, QString msg)
{
	ex.SetMsg(msg, pszErrCode);
	return ex;
}

// A version for providing your own Exception type to throw
template <class T>
void _LgMsgHlpExT(LOGLEVEL level,
	const char* pszMethod, const char* pszSource,
	const char* pszCategory,
	const char* pszFile, uint lineNumber,
	T ex)
{
	// Log the message if level is high enough
	if(level <= LoggingSystem::GetMaxLevel())
	{
#ifdef LOG_METHOD_NAME
		// Log the message
		LoggingSystem::GetLogger()->Log(level, QThread::currentThreadId(), pszSource, pszCategory, QString("%1 : %2").arg(pszMethod, ex.Msg()));
#else
	// Log the message
    LoggingSystem::GetLogger()->Log(level, QThread::currentThreadId(), pszSource, pszCategory, ex.Msg());
#endif
	}

	// Be we always throw an exception! This was called from an exception-throwing macro.
	ex.setInfo(pszMethod, pszFile, lineNumber, pszSource);
	throw ex;
}

template <class T>
void _LgMsgHlpExT(LOGLEVEL level,
	const char* pszMethod, const char* pszSource,
	const char* pszCategory,
	const char* pszFile, uint lineNumber,
	T ex,
	const QString& msg)
{
	// %s because what would happen if the string had printf-style escapes?
	_LgMsgHlpExT<T>(level, pszMethod, pszSource, pszCategory, pszFile, lineNumber, ex, "%s", qPrintable(msg));
}

////////////////////////////////
/// LOOK HERE //////////////////
// These are the actual logging macros.

// These use regular sprintf-style formatting with ANSI strings
#define LOGERR(msg,...)		_LgMsgHlp(LL_Error, __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__, msg, ##__VA_ARGS__)
#define LOGWRN(msg,...)		_LgMsgHlp(LL_Warn,  __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__, msg, ##__VA_ARGS__)
#define LOGINFO(msg,...)	_LgMsgHlp(LL_Info,  __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__, msg, ##__VA_ARGS__)
#define LOGDBG(msg,...)		_LgMsgHlp(LL_Debug, __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__, msg, ##__VA_ARGS__)

#define QLOGERR()			LoggerQDebug(LL_Error, __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__)
#define QLOGWRN()			LoggerQDebug(LL_Warn,  __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__)
#define QLOGINFO()			LoggerQDebug(LL_Info,  __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__)
#define QLOGDBG()			LoggerQDebug(LL_Debug, __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __LINE__)

#define INDIRECT_EXPAND(m, args) m args



// Prefer these macros to throw exceptions
#define EXERR_T(ex)		_LgMsgHlpExT(LL_Error, __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __FILE__, __LINE__, ex)
#define EXWRN_T(ex)		_LgMsgHlpExT(LL_Warn,  __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __FILE__, __LINE__, ex)
#define EXINFO_T(ex)	_LgMsgHlpExT(LL_Info,  __FUNCTION__, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, gs_pszLocalLogCategory, __FILE__, __LINE__, ex)

// OK to use these if you want
// the variadic arguments are a string in the dorm of a `const char*`, and other `const char*` printf arguments
#define EXERR(...)				EXERR_T( _LgMsgException(Exception(), __VA_ARGS__))
#define EXWRN(...)				EXWRN_T( _LgMsgException(Exception(), __VA_ARGS__))
#define EXINFO(...)				EXINFO_T(_LgMsgException(Exception(), __VA_ARGS__))

/// Use this macro instead of Q_ASSERT. It still disappears for release builds.
#ifdef _DEBUG
	#define LOGASSERT(cond) \
		if(!cond) \
		{ \
			Q_ASSERT(cond); \
			EXERR("XXXX", "Assertion Failed" #cond); \
		}
	#define LOG_VERIFY(cond) \
		((!(cond)) ? \
		qt_assert(#cond,__FILE__,__LINE__)\
		cond)
#else
	#define LOGASSERT(cond)
	#define LOGVERIFY(cond) \
		((!(cond)) ? \
		LOGERR("Assertion failed on: '"#cond"' in: " __FUNCTION__ " | " __FILE__ " (" __STR1__(__LINE__) ")")\
		cond)
#endif

class FuncLogger
{
public:
	FuncLogger(LOGLEVEL logLevel, bool bPrintTime, const char* pszSource, const char* pszCategory, const QString& msg, const char * pszMethod, uint lineNumber)
		: m_logLevel(logLevel), m_pszSource(pszSource), m_pszCategory(pszCategory), m_funcMsg(msg), m_pszMethod(pszMethod), m_lineNumber(lineNumber)
	{
		m_bPrintTime = bPrintTime;

		if(m_bPrintTime)
			m_timer.start();

		_LgMsgHlp(m_logLevel,
			m_pszMethod,
			m_pszSource,
			m_pszCategory,
			m_lineNumber,
			QString("++%1 %2").arg(m_pszMethod, m_funcMsg));
	}
	~FuncLogger()
	{
		_LgMsgHlp(m_logLevel,
			m_pszMethod,
			m_pszSource,
			m_pszCategory,
			m_lineNumber,
			(m_bPrintTime?
				QString("--%1 (%2 ms)").arg(m_pszMethod).arg(m_timer.elapsed()) :
				QString("--%1").arg(m_pszMethod)));

	}

private:
	LOGLEVEL m_logLevel;
	const char * m_pszSource = nullptr;
	const char * m_pszCategory = nullptr;
	const char * m_pszMethod = nullptr;
	QString m_funcMsg;
	uint m_lineNumber;
	bool m_bPrintTime;
	QElapsedTimer m_timer;

};

#ifdef _DEBUG
#define PRINT_TIME true
#else
#define PRINT_TIME true
#endif

#define LOGFUNCDBG_STR(str)		FuncLogger  log(LL_Debug, PRINT_TIME, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, \
												gs_pszLocalLogCategory, str, __FUNCTION__, __LINE__)
#define LOGFUNCINFO_STR(str)	FuncLogger  log(LL_Info, PRINT_TIME, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, \
												gs_pszLocalLogCategory, str, __FUNCTION__, __LINE__)
#define LOGFUNCDBG()	LOGFUNCDBG_STR ("")
#define LOGFUNCINFO()	LOGFUNCINFO_STR("")

#define LOGFUNCDBG_STR_TIME(str)		FuncLogger  log(LL_Debug, true, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, \
													gs_pszLocalLogCategory, str, __FUNCTION__, __LINE__)
#define LOGFUNCINFO_STR_TIME(str)	FuncLogger  log(LL_Info, true, gs_psz_DECLARE_LOG_SRC_Macro_is_missing, \
													gs_pszLocalLogCategory, str, __FUNCTION__, __LINE__)
#define LOGFUNCDBG_TIME()	LOGFUNCDBG_STR_TIME ("")
#define LOGFUNCINFO_TIME()	LOGFUNCINFO_STR_TIME("")

} // end namespace Logging



// Let's assume #including is all the caller should need to do
using namespace Logging;


// Predefined Categories - these may be used by specialized macros below
#define LOGCAT_Common			"Bell"

#ifdef _DEBUG
// Only turn this on when you need it
//#define LOG_METHOD_NAME
#endif

Q_DECLARE_METATYPE(LOGLEVEL);

