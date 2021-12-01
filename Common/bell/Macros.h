#pragma once
#include <QCoreApplication>

// We will always provide our own verify
#ifdef VERIFY
#undef VERIFY
#endif

#if !defined(VERIFY)
#define VERIFY(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#endif

// The TODO macro allows you to insert reminders and questions directly
// in the code.  The comments are output to the build window during
// the build.
// example:
//    TODO("Can we simplify this code?")
//
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : TODO - "
#define __LOCERR__ __FILE__ "(" __STR1__(__LINE__) ") : error: "

#ifdef Q_OS_WIN
#define TODO(x) __pragma(message(__LOC__ x))
#else
#define TODO(x)
#endif


#ifndef NO_TIMEOUT
#define NO_TIMEOUT	-1
#endif//NO_TIMEOUT

// Some common stuff

#define COMPANY_NAME				"908 Devices"
#define COMPANY_NAME_NOSPACES		"908Devices"


#ifdef Q_OS_WIN
//Allow use of static_assert in VS2008
#if _MSC_VER < 1600
#define static_assert(x,y) Q_ASSERT_X(x, "Assert", y)
#endif
#endif


#define PRETTY_FUNC_STR(x) #x
#ifdef Q_OS_WIN
#define PRETTY_FUNC PRETTY_FUNC_STR(__FUNCSIG__)
#else
#define PRETTY_FUNC PRETTY_FUNC_STR(__PRETTY_FUNCTION__)
#endif

template <typename ToCheck, size_t ExpectedSize, size_t RealSize>
inline void expected_actual() {
	static_assert(ExpectedSize == RealSize, "SAFETY CHECK! Type size changed '" PRETTY_FUNC "' - Check the serializer / copy constructor.");
}

/// Use this macro to force you to update your copy constructor when a member is added.
/// Failure to do this can lead to very difficult bugs.
#if defined(_M_X64) && defined(Q_OS_WIN)
#define VERIFY_SIZE(Type, ExpectedSize) expected_actual<Type, ExpectedSize, sizeof(Type)>()
#else
#define VERIFY_SIZE(classname, siz)
#endif



#define qObject(x) dynamic_cast<QObject*>(x)


// Up to here worked



/// Handles function overloads when connecting a signals/slot, for example
/// connect(pSpinBox, &QDoubleSpinBox::valueChanged,
///     this, &SomeObject::doSomething); failes because valueChanged is
/// overloaded. Use CASTFUNC to get around this
/// connect(pSpinBox, CASTFUNC(QDoubleSpinBox, &QDoubleSpinBox::valueChanged, double),
///     this, &SomeObject::doSomething);
#define CASTFUNC(object, function, ...) static_cast<void (object::*)(__VA_ARGS__)>(function)



//
#define TYPE(...) __VA_ARGS__	// eats commas when necessary

#define CONCAT__(x,y) x##y
#define CONCAT_(x,y) CONCAT__(x,y)

#define MAKE_UNIQUE_NAME(name) CONCAT_(name, __COUNTER__)

/// These are our per-product translation macros for C++. Use these to comparmentalize your strings
/// and avoid having, e.g. AVCAD strings appear in a .ts file meant for MX908.
/// There are qml/javascript equivalents in Odo/Top.qml.
/// If your string is generic and not product specific, use the tr() and qsTr() macros as before.

#define TR_ZCI(...) QCoreApplication::translate(this->metaObject()->className(), __VA_ARGS__)
#define TR_AVCAD(...) QCoreApplication::translate(this->metaObject()->className(), __VA_ARGS__)
#define TR_AVCAD_STATIC(...) tr(__VA_ARGS__)
#define TR_M908(...) QCoreApplication::translate(this->metaObject()->className(), __VA_ARGS__)
#define TR_M908_STATIC(...) tr(__VA_ARGS__)
#define TR_REBEL(...) QCoreApplication::translate(this->metaObject()->className(), __VA_ARGS__)

