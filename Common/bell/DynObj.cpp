#include "DynObj.h"
#include "Logging.h"



DECLARE_LOG_SRC("DynObj", LOGCAT_Common);



int* DynObj::DynObjMetaRegistrar::s_regFuncs[400];
int DynObj::DynObjMetaRegistrar::s_iRegFuncMaxCount = sizeof(s_regFuncs)/sizeof(s_regFuncs[0]);
int DynObj::DynObjMetaRegistrar::s_iRegFuncCount = 0;
bool DynObj::DynObjMetaRegistrar::s_bRegisterCalled = false;



void DynObj::RegisterTypes()
{
	if (DynObjMetaRegistrar::s_bRegisterCalled)
		return;	// Only do once

	// Create a function pointer variable on the stack
	void (*pRegFunc)();

	for(int i=0; i<DynObjMetaRegistrar::s_iRegFuncCount; ++i)
	{
		// Call each registration function
		pRegFunc = (void (*)(void))DynObjMetaRegistrar::s_regFuncs[i];	// Yes, this is wack!
		(*pRegFunc)();
	}
	DynObjMetaRegistrar::s_bRegisterCalled = true;
}


/// Internal implementation
void* DynObj::_Create(const QString& className)
{
	// If you intend to do any dynamic object creation, then you must
	// register the DynObj-derived classes by calling DynObj::RegisterTypes().
	// Calling Bell::Init() will take care of this for you.
	Q_ASSERT(DynObjMetaRegistrar::s_bRegisterCalled);

	// Create an instance of the class (by name)
	QByteArray ascii = className.toUtf8();
	int typeId = QMetaType::type(ascii.data());
	if(0 == typeId)
	{
		LOGERR("Type '%s' not registered for DynObj creation", qPrintable(className));
		return NULL;
	}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	// Qt 5.x
	void *pObj = QMetaType::create(typeId);
#else
	// Qt 4.8.x
	void *pObj =	QMetaType::construct(typeId);
#endif
	if(nullptr == pObj)
	{
		LOGWRN("Could not create instance of type '%s'", qPrintable(className));
		return nullptr;
	}
	return pObj;
}



void DynObj::_LogInfo(const char *pszMsg)
{
#ifdef _DEBUG
	LOGINFO(pszMsg);
#endif
}

