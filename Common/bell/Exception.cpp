#include "Exception.h"
#include <QByteArray>
//#include <Ver.h>
#include "Logging.h"
#include <QMetaType>
#include <Qt>
#include <QDateTime>
#include <QSysInfo>
//#include <QHostInfo>

DECLARE_LOG_SRC("Exception", LOGCAT_Common)




Exception::Exception(unsigned int stackIgnore)
	: m_sMsg(QString())
	, m_uiLineNumber(0)
	, m_sTimestamp(QDateTime::currentDateTime().toString(Qt::ISODate))
#ifdef Q_OS_WIN
	, m_trace(StackTrace(stackIgnore + 1)) // ignore this classes creation in the stack trace
#endif
{
}

Exception::Exception(QString msg, unsigned int stackIgnore) : Exception(stackIgnore + 1)
{
	m_sMsg = msg;
}

Exception::Exception(QString msg, QString sSource, int iLineNum, unsigned int stackIgnore) : Exception(msg, stackIgnore + 1)
{
	m_sModuleName = sSource;
	m_uiLineNumber = iLineNum;
}

Exception::Exception(const char* msg, unsigned int stackIgnore /*= 1*/) : Exception(stackIgnore + 1)
{
	m_sMsg = msg;
}

Exception::Exception(const char** msg, unsigned int stackIgnore /*= 1*/) : Exception(stackIgnore + 1)
{
	m_sMsg = *msg;
}

void Exception::setInfo(QString origin, QString file, uint lineNumber, QString moduleName)
{
	m_sOrigin = origin;
	m_sFilename = file;
	m_uiLineNumber = lineNumber;
	m_sModuleName = moduleName;
}

QString Exception::Msg() const
{
	return m_sMsg;
}


QString Exception::Origin() const
{
	return m_sOrigin;
}

uint Exception::LineNumber() const
{
	return m_uiLineNumber;
}

QString Exception::ModuleName() const
{
	return m_sModuleName;
}

QString Exception::TypeName() const
{
    return m_sTypeName;
}

QString Exception::ToString() const
{
	return QString("Exception: %1 | From: %2 (line: %3)").arg(
					m_sMsg,
					m_sOrigin,
					QString::number(m_uiLineNumber));
}

//--------------------------------------------------------------------------------------------------
//	ToDetailedString ( public )
//--------------------------------------------------------------------------------------------------
QString Exception::ToDetailedString() const
{
	QString details;
	details.append("EXCEPTION DETAILS:\n");
	details.append(QString("    Timetamp     : %1\n").arg(Timestamp()));
	details.append(QString("    Msg          : %1\n").arg(Msg()));
	details.append(QString("    Origin       : %1\n").arg(Origin()));
	details.append(QString("    File         : %1\n").arg(File()));
	details.append(QString("    Line         : %1\n").arg(QString::number(LineNumber())));
	details.append(QString("    Error Code   : %1\n").arg(m_sErrorCode));
	details.append("\n");
#ifdef Q_OS_WIN
    details.append(m_sysDetails);
	details.append(QString("STACK TRACE:\n%1\n").arg(Trace()));
#endif
	return details;
}

QString Exception::ErrorCode() const
{
	return m_sErrorCode;
}

void Exception::SetMsg(const QString& sMsg, const QString& sErrCode)
{
	m_sMsg = sMsg;
	m_sErrorCode = sErrCode;
}

void Exception::SetMsg(const QString& sMsg)
{
	m_sMsg = sMsg;
}

QString Exception::Trace() const
{
#ifdef Q_OS_WIN
	return m_trace;
#else
	return "";
#endif
}

QString Exception::File() const
{
	return m_sFilename;
}

QString Exception::Timestamp() const
{
	return m_sTimestamp;
}

const char* Exception::what() const throw()
{
	((Exception *)this)->m_baAsciiMsg = QString("%1\n On: %2:%3").
		arg(m_sMsg, m_sOrigin, QString::number(m_uiLineNumber)).toLatin1();

	return m_baAsciiMsg.constData();
}

bool TypedException::IsTypedException(const Exception& ex)
{
    return (dynamic_cast<const TypedException *>(&ex) != nullptr);
}

bool Exception::IsFatalException() const
{
    return (m_eSeverity == Severity::ERROR_FATAL);
}

/*******************************************************************************/

ExceptionContainer::ExceptionContainer(std::exception_ptr ex)
{
	m_Exception = ex;

	try
	{
		Rethrow();
	}
	catch(Exception& ex) // This can't be a const ref because class name isn't a const reference
	{
		LOGINFO("Creating ExceptionContainer for %s", qPrintable(ex.GetType()));
	}
    catch (std::exception&)
    {
        // we do not expect a std::exception
        Q_ASSERT(false);
    }
	catch(...)
	{
		// This exception is not a type ExceptionContainer supports
		Q_ASSERT(false);
	}
}

QString ExceptionContainer::Msg() const
{
	try
	{
		Rethrow();
	}
	catch(const Exception& ex)
	{
		return ex.Msg();
	}
	catch(...)
	{
		Q_ASSERT(0);
		return QString();
	}

	return QString();
}


void ExceptionContainer::Rethrow() const
{
	std::rethrow_exception(m_Exception);
}

std::exception_ptr ExceptionContainer::GetException() const
{
	return m_Exception;
}

ExceptionContainer ExceptionContainer::CurrentException()
{
	std::exception_ptr ptr = std::current_exception();
	return ExceptionContainer(ptr);
}

bool ExceptionContainer::IsFatalException() const
{
    try
    {
        Rethrow();
        return true;    // unreachable - suppresses a warning
    }
    catch (const Exception& ex)
    {
        return ex.IsFatalException();
    }
}

