#pragma once

//#define ADD_DEADLOCK_DETECTION
#define DEADLOCK_TIMEOUT_MS  500 // 1/2 second

#include <QMutex>

#ifdef ADD_DEADLOCK_DETECTION
#define CS_CHECK_DEADLOCK  __FUNCTION__ " | " __FILE__ " ("__STR1__(__LINE__)")"
#else
#define CS_CHECK_DEADLOCK  NULL
#endif // ADD_DEADLOCK_DETECTION

/**
@brief A wrapper around a windows CRITICAL_SECTION.

Qt has a QMutex, but a critical section is much more efficient on
a windows machine.  If we need to run this on non-windows, we
can provide an internal QMutex implementation. 

I won't think about that today, I'll think about that tomorrow.
	- Scarlett O'Hara
*/
class CriticalSection
{
public:
    CriticalSection(void);
    CriticalSection(const CriticalSection& other);
    ~CriticalSection(void);
	CriticalSection &operator=(const CriticalSection &other);

	/// Locks the CS.  If an owner is not provided, we don't check for the lock to be deadlocked
	void Lock(const char* owner = nullptr);
	void Unlock();

private:

    QMutex m_mux;
#ifdef ADD_DEADLOCK_DETECTION
	QString m_curOwner;
    Qt::HANDLE m_thdOwner;
#endif // ADD_DEADLOCK_DETECTION


};



/**
@brief An automatic Critical section locker/unlocker

Note about deadlocks:
	In order to add Deadlock detection to this code, we call it with some info that is defined in
	the CS_CHECK_DEADLOCK macro.  This adds helpful info regarding where the deadlock is.

	So whenever you use this, add it to the section of your code like this:
	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);

	This of course assumes that in the area of the code you are protecting, your lock is expected
	not have to wait for a long period of time.  If instead you do expect it to take long (ie, a long
	in the wait is NOT a deadlock, then use the simple case:

	CriticalSectionLocker lock(m_cs);

*/
class CriticalSectionLocker
{
public:

	CriticalSectionLocker(CriticalSection& cs, const char* deadlock_info = nullptr);
	~CriticalSectionLocker();
	void Unlock();
private:
	CriticalSection *m_pCs = nullptr;
};
