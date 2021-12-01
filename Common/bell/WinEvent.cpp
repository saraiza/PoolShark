
#include "WinEvent.h"
#include "CriticalSection.h"

WinEventManualResetPair::WinEventManualResetPair()
{
	m_evtIsNot.SetEvent();
}

void WinEventManualResetPair::SetEvent()
{
	// these two have to be set in tandem
	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);
	m_evtIs.SetEvent();
	m_evtIsNot.ResetEvent();
}

void WinEventManualResetPair::ResetEvent()
{
	// these two have to be set in tandem
	CriticalSectionLocker lock(m_cs, CS_CHECK_DEADLOCK);
	m_evtIs.ResetEvent();
	m_evtIsNot.SetEvent();
}


bool WinEventManualResetPair::Wait(uint timeoutMs) const
{
	return m_evtIs.Wait(timeoutMs);
}

bool WinEventManualResetPair::WaitNot(uint timeoutMs) const
{
	return m_evtIsNot.Wait(timeoutMs);
}


bool WinEventManualResetPair::IsSet() const
{
	return m_evtIs.Wait(0);
}

WinEventManualReset& WinEventManualResetPair::GetEventIs()
{
	return m_evtIs;
}

const WinEventManualReset& WinEventManualResetPair::GetEventIs() const
{
    return m_evtIs;
}

WinEventManualReset& WinEventManualResetPair::GetEventIsNot()
{
	return m_evtIsNot;
}

const WinEventManualReset& WinEventManualResetPair::GetEventIsNot() const
{
    return m_evtIsNot;
}


