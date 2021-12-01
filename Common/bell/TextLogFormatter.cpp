
#include "TextLogFormatter.h"


TextLogFormatter::TextLogFormatter()
{
}


TextLogFormatter::TextLogFormatter(ILog *pInnerLog)
{
	m_pInnerLog = pInnerLog;
}


void TextLogFormatter::Start()
{
	m_pInnerLog->Start();
}

QString TextLogFormatter::GetLogFileName()
{
	return m_pInnerLog->GetLogFileName();
}

void TextLogFormatter::Hint(Hints hint)
{
	m_pInnerLog->Hint(hint);
}

void TextLogFormatter::SetLog(ILog *pInnerLog) 
{ 
	m_pInnerLog = pInnerLog;
}

//static
const char* TextLogFormatter::GetLevelString(int iLevel)
{
	switch(iLevel)
	{
	case LL_None:
#ifdef	_DEBUG
		return "NONE";
#else
		return "DEV"; // to indicate that this is a Developer's only message.
#endif
	case LL_Error:
		return "ERR ";
	case LL_Warn:
		return "WARN";
	case LL_Info:
		return "INFO";
	case LL_Debug:
		return "DBG ";
	}
	return "????";
}

QString TextLogFormatter::Timestamp()
{
	//return QDateTime::currentDateTime().toString(Qt::ISODate);
	return QDateTime::currentDateTime().toString("yyyyMMdd-hh:mm:ss:zzz");
}

void TextLogFormatter::Log(int iLevel, Qt::HANDLE threadId, const char* pszSource,
						const char* pszCategory, const char* pszMsg)
{
	if(Logging::LoggingSystem::GetMaxLevel() < iLevel)
		return;

	QString ts = Timestamp();
	QString s = QString("%5 th:%6 %1:%2:%3: %4").arg(
							GetLevelString((LOGLEVEL)iLevel),
							QString(pszCategory),
							QString(pszSource),
							QString(pszMsg),
							ts,
							QString::number((unsigned long long)threadId));
	m_pInnerLog->Log(s);
}


void TextLogFormatter::Log(
					int iLevel,
					Qt::HANDLE threadId,
					const char* pszSource,
					const char* pszCategory,
					const QString& msg)
{	
	if(Logging::LoggingSystem::GetMaxLevel() < iLevel)
		return;

	QString s = QString("%5 th:%6 %1:%2:%3: %4").arg(
							GetLevelString((LOGLEVEL)iLevel),
							QString(pszCategory),
							QString(pszSource),
							msg,
							Timestamp(),
							QString::number((unsigned long long)threadId));
	m_pInnerLog->Log(s);
}

// Zulu time zone copied from TimeZones.cpp
TextLogFormatterWTimeZone::TextLogFormatterWTimeZone()
    : TextLogFormatter()
{
}


TextLogFormatterWTimeZone::TextLogFormatterWTimeZone(ILog *pInnerLog)
    : TextLogFormatter(pInnerLog)
{
}

void TextLogFormatterWTimeZone::setTimeZone(const QTimeZone tzone)
{
    m_tzone = tzone;
}

QString TextLogFormatterWTimeZone::Timestamp()
{
	QDateTime dt = QDateTime::currentDateTime();
    return QString("%1 %2").arg(dt.toString("yyyyMMdd-hh:mm:ss:zzz"), m_tzone.abbreviation(dt));
}