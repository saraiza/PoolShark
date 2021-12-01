
#include "SignalWaiter.h"
#include <QThread>
#include "Macros.h"


SignalWaiter::SignalWaiter(QObject *pSender, const char *pszSignal)
	: QObject(NULL)
{
#ifdef _DEBUG
	m_hWaitThreadId = QThread::currentThreadId();
#else
	m_hWaitThreadId = 0;
#endif
	// We must force this to be a direct connection. The slot must be called in the
	// sender's thread context.
	VERIFY(connect(pSender, pszSignal, this, SLOT(OnSignal()), Qt::DirectConnection) );
}


SignalWaiter::~SignalWaiter()
{
	//VERIFY(disconnect(parent(), pszSignal, this, SLOT(OnSignal())));
}


void SignalWaiter::OnSignal()
{
	// saraiza: I disabled this assert because the waiter can still be used in a single threaded situation.
	//Q_ASSERT(m_hWaitThreadId != QThread::currentThreadId());
	SetEvent();
}

void SignalWaiter::Reset()
{
	ResetEvent();
}
