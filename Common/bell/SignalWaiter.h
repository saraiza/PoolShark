#ifndef SIGNALWAITER_H
#define SIGNALWAITER_H

#include <QObject>
#include "WinEvent.h"

/**
@brief Signal reflector to make a signal synchronous.

This class is meant to be used on the stack in an arbitrary thread context. If you
have a thread that needs to wait on a signal from an object living in a different 
thread, create one of these on the stack and it will do the rest.

You MUST be in a separate thread. If you are not, there will be a deadlock.
*/
class SignalWaiter : public QObject, public WinEventManualReset
{
	Q_OBJECT

public:
	SignalWaiter(QObject *pSender, const char *pszSignal);
	~SignalWaiter();
	
	void Reset();

	WinEventManualReset *GetEvent() { return this; }

private slots:
	void OnSignal();

private:
	Qt::HANDLE m_hWaitThreadId;	///< Only used in a debug build. We check for the same-thread case.

};

#endif // SIGNALWAITER_H
