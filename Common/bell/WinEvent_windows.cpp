#include "WinEvent.h"
#include <windows.h>
#include "CriticalSection.h"
#include "Defs.h"

static int created = 0;
static int destroyed = 0;

class IWinEventPrivate
{
public:
	HANDLE m_handle;
};

WinEvent::WinEvent(void)
{
	m_pPrivate = new IWinEventPrivate();
	m_pPrivate->m_handle = 0;
	++created;
}

WinEvent::~WinEvent(void)
{
	++destroyed;
	::CloseHandle(m_pPrivate->m_handle);
	delete m_pPrivate;
	m_pPrivate = nullptr;
}

void WinEvent::SetEvent()
{
	::SetEvent(m_pPrivate->m_handle);
}

void WinEvent::ResetEvent()
{
	::ResetEvent(m_pPrivate->m_handle);
}


bool WinEvent::Wait(uint timeoutMs) const
{
	DWORD dwWaitResult = ::WaitForSingleObject(m_pPrivate->m_handle, timeoutMs);
	bool ret = WAIT_TIMEOUT != dwWaitResult;
	return ret;
}

int WinEvent::Wait(const IWinEvent &hEvt0, uint timeoutMs)
{
	HANDLE hHandles[1];
	hHandles[0] = hEvt0.Handle()->m_handle;

	DWORD dwWaitResult = ::WaitForMultipleObjects(1, &hHandles[0], FALSE, timeoutMs);
	if (WAIT_TIMEOUT == dwWaitResult)
		return -1;

	return dwWaitResult - WAIT_OBJECT_0;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, uint timeoutMs)
{
	HANDLE hHandles[2];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;

	DWORD dwWaitResult = ::WaitForMultipleObjects(2, &hHandles[0], FALSE, timeoutMs);
	if (WAIT_TIMEOUT == dwWaitResult)
		return -1;

	return dwWaitResult - WAIT_OBJECT_0;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, uint timeoutMs)
{
	HANDLE hHandles[3];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
	hHandles[2] = hEvt2.Handle()->m_handle;

	DWORD dwWaitResult = ::WaitForMultipleObjects(3, &hHandles[0], FALSE, timeoutMs);
	if (WAIT_TIMEOUT == dwWaitResult)
		return -1;

	return dwWaitResult - WAIT_OBJECT_0;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, uint timeoutMs)
{
	HANDLE hHandles[4];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
	hHandles[2] = hEvt2.Handle()->m_handle;
	hHandles[3] = hEvt3.Handle()->m_handle;

	DWORD dwWaitResult = ::WaitForMultipleObjects(4, &hHandles[0], FALSE, timeoutMs);
	if (WAIT_TIMEOUT == dwWaitResult)
		return -1;

	return dwWaitResult - WAIT_OBJECT_0;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, const IWinEvent &hEvt4, uint timeoutMs)
{
	HANDLE hHandles[5];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
	hHandles[2] = hEvt2.Handle()->m_handle;
	hHandles[3] = hEvt3.Handle()->m_handle;
	hHandles[4] = hEvt4.Handle()->m_handle;

	DWORD dwWaitResult = ::WaitForMultipleObjects(5, &hHandles[0], FALSE, timeoutMs);
	if (WAIT_TIMEOUT == dwWaitResult)
		return -1;

	return dwWaitResult - WAIT_OBJECT_0;	// Return the index
}

int WinEvent::Wait(QList<const IWinEvent*> listEvents, uint timeoutMs)
{
	QVector<HANDLE> vectHandles;
	for (const IWinEvent* pEvt : listEvents)
		vectHandles += pEvt->Handle()->m_handle;

	DWORD dwWaitResult = ::WaitForMultipleObjects(vectHandles.count(), &vectHandles[0], FALSE, timeoutMs);
	if (WAIT_TIMEOUT == dwWaitResult)
		return -1;

	return dwWaitResult - WAIT_OBJECT_0;	// Return the index
}




IWinEventPrivate *WinEvent::Handle() const
{
	return m_pPrivate;
}

quint64 WinEvent::PlatformHandle() const
{
    return (quint64)m_pPrivate->m_handle;
}

/************************************************************************/

WinEventAutoReset::WinEventAutoReset(QString strOptionalEvtName)
{
	m_strEvtName = strOptionalEvtName;
	LPCTSTR pStr = NULL;
	if (!m_strEvtName.isEmpty())
	{
		pStr = (LPCTSTR)m_strEvtName.unicode();
	}
	m_pPrivate->m_handle = ::CreateEvent(NULL, FALSE, FALSE, pStr);
}

WinEventAutoReset::WinEventAutoReset(const WinEventAutoReset& other)
{
	m_strEvtName = other.m_strEvtName;
	LPCTSTR pStr = NULL;
	if (!m_strEvtName.isEmpty())
	{
		pStr = (LPCTSTR)m_strEvtName.unicode();
	}
	m_pPrivate->m_handle = ::CreateEvent(NULL, FALSE, FALSE, pStr);
}


/************************************************************************/
WinEventManualReset::WinEventManualReset(QString strOptionalEvtName)
{
	m_strEvtName = strOptionalEvtName;
	LPCTSTR pStr = NULL;
	if (!m_strEvtName.isEmpty())
	{
		pStr = (LPCTSTR)m_strEvtName.unicode();
	}
	m_pPrivate->m_handle = ::CreateEvent(NULL, TRUE, FALSE, pStr);
}


WinEventManualReset::WinEventManualReset(const WinEventManualReset& other)
{
	m_strEvtName = other.m_strEvtName;
	LPCTSTR pStr = NULL;
	if (!m_strEvtName.isEmpty())
	{
		pStr = (LPCTSTR)m_strEvtName.unicode();
	}
	m_pPrivate->m_handle = ::CreateEvent(NULL, TRUE, FALSE, pStr);
}

bool WinEventManualReset::IsSet() const
{
	return Wait(0);
}

