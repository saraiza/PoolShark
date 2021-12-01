
#include "Logging.h"
#include "TraceLog.h"
#include "TextLogFormatter.h"
#include "Exception.h"
//#include "Ver.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(disable : 4996)

using namespace Logging;

// These are used to gather up source/category info.
static LogSourceDB::ENTRY ms_sources[MAX_SOURCES];
static int ms_iSourceCount = 0;

// We provide a default logger that sends things through the debugger's output window.
// This can be changed by the user when they call LoggingSystem::Start();
static TraceLog g_traceLog;
static TextLogFormatter g_textFormatter(&g_traceLog);


/**************************************************************************/
ILogger* Logging::LoggingSystem::ms_pLogger = &g_textFormatter;
bool Logging::LoggingSystem::ms_bStarted = false;
LOGLEVEL Logging::LoggingSystem::ms_maxLevel = LL_Info;

void LoggingSystem::Start(ILogger *pLogger)
{
	// Grab the top-level logger. If they don't provide one, use the default.
	if(pLogger)
		ms_pLogger = pLogger;	

	ms_bStarted = true;
	ms_pLogger->Start();
}

void LoggingSystem::Reset()
{
    ms_pLogger = &g_textFormatter;
}

ILogger* LoggingSystem::GetLogger()
{
	return ms_pLogger;
}

bool LoggingSystem::IsStarted()
{
	return ms_bStarted;
}

LOGLEVEL LoggingSystem::GetMaxLevel()
{
	return ms_maxLevel;
}

void LoggingSystem::SetMaxLevel(LOGLEVEL maxLevel)
{
	ms_maxLevel = maxLevel;
}

void LoggingSystem::Hint(Hints hint)
{
	if (ms_pLogger)
		ms_pLogger->Hint(hint);
}

QString  LoggingSystem::GetLogFileName()
{
	QString sFileName;
	if (ms_pLogger)
		sFileName = ms_pLogger->GetLogFileName();
	return sFileName;
}
/**************************************************************************/



LogSourceDB::LogSourceDB(const char* pszSource, const char* pszCategory)
{
	if(ms_iSourceCount >= MAX_SOURCES)
	{
		Q_ASSERT(false);	// We've added more classes than we planned for, change MAX_SOURCES
		qDebug() << "**ERROR - TOO MANY LOG SOURCES - " + QString::number(ms_iSourceCount);
		Q_ASSERT(false);
		return;
	}

	ms_sources[ms_iSourceCount].pszSrc = pszSource;
	ms_sources[ms_iSourceCount].pszCategory = pszCategory;
	++ms_iSourceCount; 
}


int LogSourceDB::GetSourceCount()
{
	return ms_iSourceCount;
}

LogSourceDB::ENTRY* LogSourceDB::GetSources()
{
	return &ms_sources[0];
}





/** Helper method used by the logging macros to generate
the printf-style strings.
*/
bool Logging::_LgMsgHlp(LOGLEVEL level,
						const char* pszMethod,
						const char* pszSource,
						const char* pszCategory,
						uint lineNumber,
						const char* pszFormat, ...)
{
    Q_UNUSED(pszMethod);
    Q_UNUSED(lineNumber);
	if(level > LoggingSystem::GetMaxLevel())
		return false;	// Skip making the entry

	// Process the string formatting
	va_list args;
	va_start(args, pszFormat);
    char szMsg[2048];
#ifdef Q_OS_WIN
    _vsnprintf(szMsg, sizeof(szMsg)/sizeof(szMsg[0]), pszFormat, args);
#else
    vsnprintf(szMsg, sizeof(szMsg)/sizeof(szMsg[0]), pszFormat, args);
#endif
	va_end(args);
	szMsg[sizeof(szMsg)-1] = 0; // null terminate to be safe

#ifdef LOG_METHOD_NAME
	// Log the message
	g_pLogger->Log(level, QThread::currentThreadId(), pszSource, pszCategory, QString("%1 : %2").arg(pszMethod, szMsg));
#else
	// Log the message
	LoggingSystem::GetLogger()->Log(level, QThread::currentThreadId(), pszSource, pszCategory, szMsg);
#endif

	// We return false to make this easy to use.
	return false;
}

bool Logging::_LgMsgHlp(LOGLEVEL level,
	const char* pszMethod,
	const char* pszSource,
	const char* pszCategory,
	uint lineNumber,
	const QString& msg)
{
	// %s because what would happen if the string had printf-style escapes?
	return _LgMsgHlp(level, pszMethod, pszSource, pszCategory, lineNumber, "%s", qPrintable(msg));
}

namespace
{
typedef QVector<QString> TModuleNames;
// To make sure g_moduleNames is initialized when RegRegisterNameForExceptionId get it, wrap it as a static
// in a function.
// Returns a sorted vector of module names. I would normally use a std::set for this, but we are staying away from
// stl containers. QSet uses a hash table and is not suitable for this.
TModuleNames& GetRegisteredModuleNames(){
	static TModuleNames g_moduleNames;
	return g_moduleNames;
}

inline quint16 ModuleIndexToModuleId(quint16 index)
{
	return index + 1;
}

inline quint16 ModuleIdToModuleIndex(quint16 id)
{
	return id - 1;
}

union ExceptionId {
	struct {
		quint16 m_lineNumber;
		quint16 m_moduleId;
		quint32 m_svnBuildNumber;
	} m_components;
	quint64 m_id;
};

}

namespace Logging
{
RegisterNameForExceptionId::RegisterNameForExceptionId(QString name)
{
	TModuleNames& moduleNames(GetRegisteredModuleNames());
	// Insert the new value into the collection while keeping the collection sorted
	TModuleNames::iterator it = std::lower_bound(moduleNames.begin(), moduleNames.end(), name);
	if (it != moduleNames.end() && *it == name)
		return;  // do not insert dups
	moduleNames.insert(it, name);
}

quint64 GetExceptionId(QString moduleName, quint16 lineNumber)
{
	quint16 moduleId = 0;
	{
		TModuleNames& moduleNames(GetRegisteredModuleNames());
		TModuleNames::iterator it = std::lower_bound(moduleNames.begin(), moduleNames.end(), moduleName);
		if (it != moduleNames.end() && *it == moduleName)
			moduleId = ModuleIndexToModuleId(std::distance(moduleNames.begin(), it));
	}
	ExceptionId eid;
	eid.m_components.m_moduleId = moduleId;
	eid.m_components.m_lineNumber = lineNumber;

	return eid.m_id;
}

QPair<QString, quint16> GetNameAndLineNumberFromExceptionId(quint64 exceptionId)
{
	ExceptionId eid;
	eid.m_id = exceptionId;
	quint16 lineNumber = eid.m_components.m_lineNumber;
	quint16 moduleId = eid.m_components.m_moduleId;
	TModuleNames& moduleNames(GetRegisteredModuleNames());
	QString moduleName;
	if (moduleId > 0 && moduleNames.size() > moduleId)
		moduleName = moduleNames[ModuleIdToModuleIndex(moduleId)];
	return qMakePair(moduleName, lineNumber);
}

}
