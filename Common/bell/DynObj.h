#pragma once

#include <QString>
#include <QObject>
#include <QtGlobal>

/**
@brief Dynamic object creation and registration

The DynObj utility class makes it easy for your classes to participate in Qt's 
dynamic object creation.  You must register your class with the DYNO_REGISTER macro
in the cpp file, then you may create instances of your class by string name. Your
class must inherit from DynObj (just to pull in the className() method def).
During program init, you must call DynObj::RegisterTypes(). This keeps it from 
being called at object construction time. Here is an example.

~~~~~~~~~~~~~~~~~~~~~{.c}
//// Dog.h
class Dog : public IAnimal, public DynObj
{
public:
	Dog();
	QString MakeNoise();
};

//// Dog.cpp

DYNO_REGISTER(Dog, "Dog")

Dog::Dog()
{
}
~~~~~~~~~~~~~~~~~~~~~

Under the hood, DynObj uses Qt's qRegisterMetaType<MyClass>("MyClass") function to
associate a string with the class instance creator.  A funny thing about the way that Qt
implemented this system, classes are not required to be derived from QObject.  This 
turned out to be a nice bit of freedom for us.

I separated this class out from SerMig because I began to see that it was useful
to do polymorphism in SerMig, but not require those class types to be serializable. It
also simplifies the SerMig.h header file somewhat.
*/
class DynObj
{
public:
	/// You must explicitely call this to register your dynamic types with Qt.
	static void RegisterTypes();

	template<class TClass>
	static TClass* Create(const QString& className)
	{
		return (TClass*)_Create(className);
	}


    /// Implementation created by DYNO_REGISTER macro#include <QtGlobal>
	//virtual QString className() = 0;

	
// Utility class used for registering MetaType with Qt

	// To keep this light, we use function pointers. (Can't use the 'this' pointer from the constructor of a virtual class)
	class DynObjMetaRegistrar
	{
	public:
		static int s_iRegFuncMaxCount;
		static int* s_regFuncs[];		// A fixed size array of function pointers
		static int s_iRegFuncCount;		// The number or registration functions in the table
		static bool s_bRegisterCalled;	// Used to generate an assert to instruct the user about registration
		DynObjMetaRegistrar(void* pfunc)
		{
			Q_ASSERT(s_iRegFuncCount < s_iRegFuncMaxCount);
			s_regFuncs[s_iRegFuncCount++] = (int*)pfunc;
		}
	};

	
	static void _LogInfo(const char *pszMsg);

private:
	/// Call this to create an instance by string name
	static void* _Create(const QString& className);
};


#define DYNO_REGISTER(TClass, regName) \
void _DynObjReg_##TClass() \
{ \
	qRegisterMetaType<TClass>(regName); \
	qDebug() << regName; \
} \
DynObj::DynObjMetaRegistrar static gs_registrar_##TClass((void*)&_DynObjReg_##TClass);


