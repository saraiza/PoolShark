#pragma once

#include <QDataStream>
#include <qstringlist.h>
#include <exception>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#ifdef Q_OS_WIN
#include "StackTrace.h"
#endif
#include "SystemDetails.h"

// Hacky work around template / className removed DynObj fun
#define DECLARE_EXCEPTION_NAME(exname) \
	static QString Type() { return QStringLiteral("ex" #exname); } \
	virtual QString GetType() const { return QStringLiteral("ex" #exname); }



/**
@brief The base class of all exceptions.

This class is thrown by reference everywere. You would not likely need
to use this class directly. Instead, use the Logging.h EXERR/EXWRN/EXINFO/EXERR_T
macros to throw an exception and log the message.

Every exception has text. Not all of this text would make sense to a customer/user.
But if an unexpected program condition did cause an exception, the user may see it.
For this reason, there is an additional UserMsg() method.  Only exceptions thrown
with the bUserMsg flag set true will return the actual text from UserMsg(). Otherwise,
it will return something like 'Internal Error'. It would be great to figure
out how to auto-generate a log message ID so that an internal error shown to
the user is easy to remotely correlate to a log message.
*/

class Exception : public std::exception
{
public:
	Exception(unsigned int stackIgnore = 1);
	Exception(const char* msg, unsigned int stackIgnore = 1);
	Exception(const char** msg, unsigned int stackIgnore = 1);
	Exception(QString msg, unsigned int stackIgnore = 1);
	Exception(QString msg, QString sSource, int iLineNum, unsigned int stackIgnore = 1); //< Need to use this constructor to get correct error codes.

	enum ErrorCodesResult{
		ERROR_CODES_SUCCESS,
		ERROR_CODES_INVALID_SIZE,
		ERROR_CODES_CHECKSUM_FAIL,
	};

    enum Severity {
        ERROR_FATAL,
        ERROR_NON_FATAL,
    };

	QString Msg() const;		///< The message of the exception, filled in at throw-time
	QString ErrorCode() const;  ///< Error code of the exception.
	QString Origin() const;
	uint LineNumber() const;
	QString ModuleName() const;
	QString File() const;
	QString Trace() const;		///< Stack trace that generated the exception
	QString Timestamp() const;
	QString ToString() const;
	QString ToDetailedString() const;
    QString TypeName() const;
    QString Severity() const;
    bool IsFatalException() const;

	void SetMsg(const QString& sMsg);
	void SetMsg(const QString& sMsg, const QString& sErrCode);

	/// Setup the extra info to the exception for various reasons
	void setInfo(QString origin, QString file, uint lineNumber, QString moduleName);

	const char* what() const throw() override;

	DECLARE_EXCEPTION_NAME(Exception)

	operator QString() const { return GetType(); }

	friend QDataStream& operator<< (QDataStream& s, const Exception& other);
	friend QDataStream& operator>> (QDataStream& s, Exception& other);

protected:
	QString m_sMsg;
	QString m_sOrigin;
	QString m_sModuleName;
	uint m_uiLineNumber;
	QByteArray m_baAsciiMsg;
	QString m_sErrorCode;
	QString m_sFilename;
	QString m_sTimestamp;
    // we need this because the ExceptionContainer converts a typed exception to an "Exception" and we need to know the original type when we extract the exception 
    // from the container so we can go to UserMessages to get a message string
    QString m_sTypeName;       
    enum Severity m_eSeverity = Severity::ERROR_NON_FATAL;
	SystemDetails m_sysDetails;

#ifdef Q_OS_WIN
	StackTrace m_trace;
#endif
};


// The base exception class for all typed exceptions
class TypedException : public Exception
{
public:
    TypedException() {};
    TypedException(QString msg) : Exception(msg) {};

    static bool IsTypedException(const Exception& ex);
    DECLARE_EXCEPTION_NAME(TypedException) 
};

// The base exception class for all typed exceptions
// we do this so we know if we can look to find a message in UserMessage
class FatalException : public TypedException
{
public:
    FatalException() { m_eSeverity = Severity::ERROR_FATAL; };
    FatalException(QString msg) : TypedException(msg) { m_eSeverity = Severity::ERROR_FATAL; };

    static bool IsFatalException(const Exception& ex);
    DECLARE_EXCEPTION_NAME(FatalException)
};


// (Why? Because we are using it with QtRPC2)
// these don't need to be serializable, so we just have straight up declare
// the << and >> operators for datastreams
// Hendrick: I thought of making this object a public SerMig but there is an
//           issue with the order of includes due to ArchiveException et al
//           so there would be a circular reference that I don't think it's
//           worth dealing with today
inline QDataStream& operator<< (QDataStream& s, const Exception& other)
{
	s << other.m_sMsg;
	s << other.m_sOrigin;
	s << other.m_sModuleName;
	s << other.m_uiLineNumber;
	s << other.m_baAsciiMsg;
	return s;
}

inline QDataStream& operator>> (QDataStream& s, Exception& other)
{
	s >> other.m_sMsg;
	s >> other.m_sOrigin;
	s >> other.m_sModuleName;
	s >> other.m_uiLineNumber;
	s >> other.m_baAsciiMsg;
	return s;
}


/**
@brief Container class for passing Exceptions cross threads
*/
class ExceptionContainer
{
public:
	ExceptionContainer() = default;

	static ExceptionContainer CurrentException();

	QString Msg() const;
	void Rethrow() const;
	std::exception_ptr GetException() const;

	template <typename T>
	static ExceptionContainer CreateExceptionContainer(const T& ex)
	{
		static_assert(std::is_base_of<Exception, T>::value, "CreateException needs to be used with a type derived from Exception");
		auto ep = std::make_exception_ptr<T>(ex);
		return ExceptionContainer(ep);
	}

	template <typename T>
	static ExceptionContainer CreateExceptionContainer(const QString & msg)
	{
		static_assert(std::is_base_of<Exception, T>::value, "CreateException needs to be used with a type derived from Exception");
		auto ep = std::make_exception_ptr<T>(msg);
		return ExceptionContainer(ep);
	}

    bool IsFatalException() const;

	/// Test the container to see if it is holding a specific exception type
	template <typename TClass>
	bool IsType() const
	{
		try
		{
			Rethrow();
		}
		catch (const TClass&)
		{
			return true;
		}
		catch (...)
		{
		}
		return false;
	}

private:
	ExceptionContainer(std::exception_ptr ex);
	std::exception_ptr m_Exception;
};




//------------------------------
//	MACROS
//------------------------------

#define DECLARE_EXCEPTION_TYPE(type) \
class type : public TypedException \
{ \
public: \
	type() {m_sTypeName = #type ;} \
	type(QString msg) : TypedException(msg) {m_sTypeName = #type ;} \
	DECLARE_EXCEPTION_NAME(type) \
};

#define DECLARE_FATAL_EXCEPTION_TYPE(type) \
class type : public FatalException \
{ \
public: \
	type() {m_sTypeName = #type ;} \
	type(QString msg) : FatalException(msg) {m_sTypeName = #type ;} \
	DECLARE_EXCEPTION_NAME(type) \
};

