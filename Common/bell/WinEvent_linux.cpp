
#include "WinEvent.h"
#include "CriticalSection.h"
#include "Logging.h"

#define PULSE
#include <pevents.h>


DECLARE_LOG_SRC("WinEvent", LOGCAT_Common)

class IWinEventPrivate
{
public:
    neosmart::neosmart_event_t m_handle;
};

WinEvent::WinEvent(void)
{
	m_pPrivate = new IWinEventPrivate();
	m_pPrivate->m_handle = 0;
}

WinEvent::~WinEvent(void)
{
    DestroyEvent(m_pPrivate->m_handle);
	delete m_pPrivate;
	m_pPrivate = NULL;
}

void WinEvent::SetEvent()
{
    neosmart::SetEvent(m_pPrivate->m_handle);
}

void WinEvent::ResetEvent()
{
    neosmart::ResetEvent(m_pPrivate->m_handle);
}


bool WinEvent::Wait(uint timeoutMs) const
{
    return 0 == Wait(*this, timeoutMs);
}

int WinEvent::Wait(const IWinEvent &hEvt0, uint timeoutMs)
{
	void* hHandles[1];
    hHandles[0] = hEvt0.Handle()->m_handle;

    int waitObject;

    unsigned int dwWaitResult = neosmart::WaitForMultipleEvents(&hHandles[0], 1, false, timeoutMs, waitObject);
    if (ETIMEDOUT == dwWaitResult)
        return -1;

    return waitObject;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, uint timeoutMs)
{
	void* hHandles[2];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
    int waitObject;

    unsigned int dwWaitResult = neosmart::WaitForMultipleEvents(&hHandles[0], 2, false, timeoutMs,waitObject);
    if (ETIMEDOUT == dwWaitResult)
		return -1;

    return waitObject;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, uint timeoutMs)
{
	void* hHandles[3];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
	hHandles[2] = hEvt2.Handle()->m_handle;
    int waitObject;

    unsigned int dwWaitResult = neosmart::WaitForMultipleEvents(&hHandles[0], 3, false, timeoutMs, waitObject);
    if (ETIMEDOUT == dwWaitResult)
		return -1;

    return waitObject;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, uint timeoutMs)
{
	void* hHandles[4];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
	hHandles[2] = hEvt2.Handle()->m_handle;
	hHandles[3] = hEvt3.Handle()->m_handle;

    int waitObject;

    unsigned int dwWaitResult = neosmart::WaitForMultipleEvents(&hHandles[0], 4, false, timeoutMs, waitObject);
    if (ETIMEDOUT == dwWaitResult)
		return -1;

    return waitObject;	// Return the index
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, const IWinEvent &hEvt4, uint timeoutMs)
{
	void* hHandles[5];
	hHandles[0] = hEvt0.Handle()->m_handle;
	hHandles[1] = hEvt1.Handle()->m_handle;
	hHandles[2] = hEvt2.Handle()->m_handle;
	hHandles[3] = hEvt3.Handle()->m_handle;
	hHandles[4] = hEvt4.Handle()->m_handle;

    int waitObject;

    unsigned int dwWaitResult = neosmart::WaitForMultipleEvents(&hHandles[0], 5, false, timeoutMs, waitObject);
    if (ETIMEDOUT == dwWaitResult)
		return -1;

    return waitObject;	// Return the index
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
    m_pPrivate->m_handle = neosmart::CreateEvent();
}

WinEventAutoReset::WinEventAutoReset(const WinEventAutoReset& other)
{
	m_strEvtName = other.m_strEvtName;
    m_pPrivate->m_handle = neosmart::CreateEvent();
}


/************************************************************************/
WinEventManualReset::WinEventManualReset(QString strOptionalEvtName)
{
    m_strEvtName = strOptionalEvtName;
    m_pPrivate->m_handle = neosmart::CreateEvent(true);
}

WinEventManualReset::WinEventManualReset(const WinEventManualReset& other)
{
    m_strEvtName = other.m_strEvtName;
    m_pPrivate->m_handle = neosmart::CreateEvent(true);
}

bool WinEventManualReset::IsSet() const
{
	return Wait(0);
}

