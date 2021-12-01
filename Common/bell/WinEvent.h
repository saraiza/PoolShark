#pragma once
#include "CriticalSection.h"
#include <QString>
#include <climits>


class IWinEventPrivate;

/// Pure interface (almost)
class IWinEvent
{
public:
	/// Handy override to simply usage (should have done this years ago)
	void Set(bool bSet) {
		if (bSet)
			SetEvent();
		else
			ResetEvent();
	}

	virtual void SetEvent() = 0;
	virtual void ResetEvent() = 0;
    virtual bool Wait(uint timeoutMs = (uint)ULONG_MAX) const = 0;
	virtual IWinEventPrivate *Handle() const = 0;
};

/**
@brief A wrapper around a Windows Event.

I could not figure out how the "Qt Way" to do some of the thread
things I'm accustomed to doing under windows. So for now, I'm 
using the windoze way. 

Read MSDN for documentation on this one.

Some of the methods have been inlined for performance reasons. Results of
some profiling.
*/
class WinEvent : public IWinEvent
{
public:
	WinEvent(void);
	virtual ~WinEvent(void);

	void SetEvent() override;
	void ResetEvent() override;

	/// Wait for the event to be signaled. Returns true if event was signaled, false
	/// on a timeout.
        bool Wait(uint timeoutMs = (uint)ULONG_MAX) const override;

	static int Wait(const IWinEvent &hEvt0, uint timeoutMs);
        static int Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, uint timeoutMs = (uint)ULONG_MAX);
        static int Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, uint timeoutMs = (uint)ULONG_MAX);
        static int Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, uint timeoutMs = (uint)ULONG_MAX);
        static int Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, const IWinEvent &hEvt4, uint timeoutMs = (uint)ULONG_MAX);

	/// A more general way to wait on an arbitrary number of events. Much close to the native WaitForMultipleObjects OS function in windows. :)
        static int Wait(QList<const IWinEvent*> listEvents, uint timeoutMs = (uint)ULONG_MAX);

	IWinEventPrivate *Handle() const override;

    quint64 PlatformHandle() const;

protected:
	IWinEventPrivate *m_pPrivate = nullptr;
	QString m_strEvtName;

private:

};


class WinEventAutoReset : public WinEvent
{
public:
	WinEventAutoReset(QString strOptionalEvtName = "");
	WinEventAutoReset(const WinEventAutoReset& other);

    virtual ~WinEventAutoReset() = default;

private:
	QString m_strEvtName;
};

class WinEventManualReset : public WinEvent
{
public:
	WinEventManualReset(QString strOptionalEvtName = "");
	WinEventManualReset(const WinEventManualReset& other);

    bool IsSet() const;

private:
	QString m_strEvtName;
};

/**
@brief Manages a matched pair of opposite events
*/
class WinEventManualResetPair : public IWinEvent
{
public:
	WinEventManualResetPair();
    ~WinEventManualResetPair() = default;

	void SetEvent() override;
	void ResetEvent() override;
    bool Wait(uint timeoutMs = (uint)ULONG_MAX) const override;
    virtual bool WaitNot(uint timeoutMs = (uint)ULONG_MAX) const;

    bool IsSet() const;
	
    WinEventManualReset& GetEventIs();
    const WinEventManualReset& GetEventIs() const;
    WinEventManualReset& GetEventIsNot();
    const WinEventManualReset& GetEventIsNot() const;

	IWinEventPrivate *Handle() const override { return m_evtIs.Handle(); }

private:
	WinEventManualReset m_evtIs;
	WinEventManualReset m_evtIsNot;
	CriticalSection m_cs;
};



/**
@brief Utility class for resetting and setting an event from the stack.

Create an instance of this on the stack to reset the event. The destructor
will automatically set the event.  It will go both directions using the bSet 
parameter.
*/
class EventResetter
{
public:
	EventResetter(IWinEvent *pEvt)
	{
		m_pEvent = pEvt;
		m_bSetThenReset = true; // need to initialize it ?
	}
	EventResetter(IWinEvent *pEvt, bool bSetThenReset)
	{
		m_pEvent = pEvt;
		m_bSetThenReset = bSetThenReset;
		if (m_bSetThenReset)
			m_pEvent->SetEvent();
		else
			m_pEvent->ResetEvent();
	}
	~EventResetter()
	{
		if (m_pEvent)
		{
			if (m_bSetThenReset)
				m_pEvent->ResetEvent();
			else
				m_pEvent->SetEvent();
			m_pEvent = nullptr;
		}
	}

	/// Set the event, then reset it in the destructor
	void Set()
	{
		m_bSetThenReset = true;
		m_pEvent->SetEvent();
	}

	/// Reset the event, then set it in the destructor
	void Reset()
	{
		m_bSetThenReset = false;
		m_pEvent->ResetEvent();
	}


	/// Don't mess with the event anymore
	void Detach() { m_pEvent = nullptr; }

private:
	IWinEvent *m_pEvent = nullptr;
	bool m_bSetThenReset;
};
