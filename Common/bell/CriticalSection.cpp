
#include "CriticalSection.h"


CriticalSection::CriticalSection(void)
{

}

CriticalSection::CriticalSection(const CriticalSection& other)
{
}

CriticalSection::~CriticalSection(void)
{

}

CriticalSection &CriticalSection::operator=(const CriticalSection &other)
{
	return *this;
}

void CriticalSection::Lock(const char * owner)
{
#ifndef ADD_DEADLOCK_DETECTION
    m_mux.lock();
#else
	bool bLocked = false;
	if(owner)
	{
		bLocked = m_mux.tryLock(DEADLOCK_TIMEOUT_MS);
		//Q_ASSERT(bLocked);
		if (!bLocked)
		{
			LOGERR("Possible deadlock at '%s'.  Current lock owner: '%s' | Thread 0x%x (%d)", owner, qPrintable(m_curOwner), m_thdOwner, m_thdOwner);
			m_mux.lock();
		}
	}
	else
	{
		m_mux.lock();
	}

	m_curOwner = (NULL == owner) ? "" : owner;
	m_thdOwner = QThread::currentThreadId(); // still store which thread has the CS, in case another locker gets locked up.

#endif // ADD_DEADLOCK_DETECTION
	//::OutputDebugString(L"+");
}


void CriticalSection::Unlock()
{
	//::OutputDebugString(L"-");
#ifndef ADD_DEADLOCK_DETECTION
    m_mux.unlock();
#else
	m_curOwner = "";
	m_thdOwner = 0;
	m_mux.unlock();
#endif // ADD_DEADLOCK_DETECTION
}


CriticalSectionLocker::CriticalSectionLocker(CriticalSection& cs, const char* deadlock_info) 
{
	m_pCs = &cs;
	m_pCs->Lock(deadlock_info);
}


CriticalSectionLocker::~CriticalSectionLocker() 
{
	Unlock();
}

void CriticalSectionLocker::Unlock() 
{
	if(NULL == m_pCs)
		return;
	m_pCs->Unlock();
	m_pCs = NULL;
}
