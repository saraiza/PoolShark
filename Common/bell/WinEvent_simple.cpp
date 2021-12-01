#include "WinEvent.h"
#include <QThread>
#include <QElapsedTimer>
//#include "Defs.h"


#define POLL_MS			50


class IWinEventPrivate
{
public:
	/// Poor-man's win event
	volatile bool m_bSet = false;
};

WinEvent::WinEvent(void)
{
	m_pPrivate = new IWinEventPrivate();
}

WinEvent::~WinEvent(void)
{
	delete m_pPrivate;
	m_pPrivate = nullptr;
}

void WinEvent::SetEvent()
{
	m_pPrivate->m_bSet = true;
}

void WinEvent::ResetEvent()
{
	m_pPrivate->m_bSet = false;
}


bool WinEvent::Wait(uint timeoutMs) const
{
	// No kernel help, just polling
    if (-1 == (int)timeoutMs) // no timeout
	{
		while (true)
		{
			if (m_pPrivate->m_bSet)
				return true;
			QThread::msleep(POLL_MS);
		}
	}

	// We are using a timeout
	QElapsedTimer tmr;
	tmr.start();
	do {
		if (m_pPrivate->m_bSet)
			return true;
		QThread::msleep(POLL_MS);
	} while (tmr.elapsed() < timeoutMs);
	
	return false;
}

int WinEvent::Wait(const IWinEvent &hEvt0, uint timeoutMs)
{
	QElapsedTimer tmr;
	tmr.start();
	do {
		if (hEvt0.Handle()->m_bSet)
			return 0;
		QThread::msleep(POLL_MS);
	} while (tmr.elapsed() < timeoutMs);
	
	return -1;
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, uint timeoutMs)
{
	QElapsedTimer tmr;
	tmr.start();
	do {
		if (hEvt0.Handle()->m_bSet)
			return 0;
		if (hEvt1.Handle()->m_bSet)
			return 1;
		QThread::msleep(POLL_MS);
	} while (tmr.elapsed() < timeoutMs);

	return -1;
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, uint timeoutMs)
{
	QElapsedTimer tmr;
	tmr.start();
	do {
		if (hEvt0.Handle()->m_bSet)
			return 0;
		if (hEvt1.Handle()->m_bSet)
			return 1;
		if (hEvt2.Handle()->m_bSet)
			return 2;
		QThread::msleep(POLL_MS);
    } while (tmr.elapsed() < timeoutMs);

	return -1;
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, uint timeoutMs)
{
	QElapsedTimer tmr;
	tmr.start();
	do {
		if (hEvt0.Handle()->m_bSet)
			return 0;
		if (hEvt1.Handle()->m_bSet)
			return 1;
		if (hEvt2.Handle()->m_bSet)
			return 2;
		if (hEvt3.Handle()->m_bSet)
			return 3;
		QThread::msleep(POLL_MS);
	} while (tmr.elapsed() < timeoutMs);

	return -1;
}

int WinEvent::Wait(const IWinEvent &hEvt0, const IWinEvent &hEvt1, const IWinEvent &hEvt2, const IWinEvent &hEvt3, const IWinEvent &hEvt4, uint timeoutMs)
{
	QElapsedTimer tmr;
	tmr.start();
	do {
		if (hEvt0.Handle()->m_bSet)
			return 0;
		if (hEvt1.Handle()->m_bSet)
			return 1;
		if (hEvt2.Handle()->m_bSet)
			return 2;
		if (hEvt3.Handle()->m_bSet)
			return 3;
		if (hEvt4.Handle()->m_bSet)
			return 4;
		QThread::msleep(POLL_MS);
	} while (tmr.elapsed() < timeoutMs);

	return -1;
}

int WinEvent::Wait(QList<const IWinEvent*> listEvents, uint timeoutMs)
{
	QElapsedTimer tmr;
	tmr.start();
	do {
		for(int i=0; i<listEvents.count(); ++i)
        if (listEvents[i]->Handle()->m_bSet)
			return i;
		QThread::msleep(POLL_MS);
	} while (tmr.elapsed() < timeoutMs);

	return -1;
}




IWinEventPrivate *WinEvent::Handle() const
{
	return m_pPrivate;
}

quint64 WinEvent::PlatformHandle() const
{
    return (quint64)0;
}

/************************************************************************/

WinEventAutoReset::WinEventAutoReset(QString strOptionalEvtName)
{
    Q_UNUSED(strOptionalEvtName);
}

WinEventAutoReset::WinEventAutoReset(const WinEventAutoReset& other)
{
    Q_UNUSED(other);
}


/************************************************************************/
WinEventManualReset::WinEventManualReset(QString strOptionalEvtName)
{
    Q_UNUSED(strOptionalEvtName);
}


WinEventManualReset::WinEventManualReset(const WinEventManualReset& other)
{
    Q_UNUSED(other);
}

bool WinEventManualReset::IsSet() const
{
	return Wait(0);
}

