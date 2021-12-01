
#include "IODeviceThread.h"
#include "LambdaTask.h"
#include "Logging.h"
#include "Macros.h"
//#include "Util.h"
#include <QMetaMethod>
#include <QFile>

DECLARE_LOG_SRC("IODeviceThread", LOGCAT_Common)

IODeviceThread::IODeviceThread(const QString& sName, QObject *parent)
	: QIODevice(parent)
	, m_sName(sName)
{
	m_dispatcher.moveToThread(&m_thread);
	m_thread.start();
}

IODeviceThread::~IODeviceThread()
{
	Stop();
}

void IODeviceThread::Start(QIODevice * pDevice, QFlags<QIODevice::OpenModeFlag> flags)
{
	Stop();

	m_pInner = pDevice;

	m_thread.start();
	QString sName = m_sName;
	m_dispatcher.RunLambda([sName]() {
		//Util::SetDebugThreadName(sName);
	});

	Q_ASSERT(!m_pInner.isNull());
	Q_ASSERT(m_pInner->isOpen());
	Q_ASSERT(m_pInner->parent() == nullptr);
	m_pInner->moveToThread(&m_thread);

	// Connect us up all the signals. Let's do this explicitly instead of
	// programatically. There are internal signals that can be ignored,
	// we are reflecting only QIODevice signals.
	VERIFY(connect(m_pInner, &QIODevice::aboutToClose,			this, &IODeviceThread::aboutToClose));
	VERIFY(connect(m_pInner, &QIODevice::bytesWritten,			this, &IODeviceThread::bytesWritten));
	VERIFY(connect(m_pInner, &QIODevice::channelBytesWritten,	this, &IODeviceThread::channelBytesWritten));
	VERIFY(connect(m_pInner, &QIODevice::channelReadyRead,		this, &IODeviceThread::channelReadyRead));
	VERIFY(connect(m_pInner, &QIODevice::readChannelFinished,	this, &IODeviceThread::readChannelFinished));
	VERIFY(connect(m_pInner, &QIODevice::readyRead,				this, &IODeviceThread::readyRead));


	if (!open(flags | QIODevice::Unbuffered)) // Always open this wrapper as unbuffered
		EXERR("2CRE", "Failed to open this QIODevice");
}

void IODeviceThread::Stop()
{
	m_thread.exit();
	m_thread.wait(-1);

	if(!m_pInner.isNull())
		delete m_pInner;
}

bool IODeviceThread::isOpen() const
{
	if (!m_pInner)
		return false;

	bool bOpen;
	m_dispatcher.RunLambda([this, &bOpen] {
		bOpen = m_pInner->isOpen();
	});
	return bOpen;
}

qint64 IODeviceThread::readData(char *data, qint64 maxlen)
{
	qint64 bytesRead = 0;
	m_dispatcher.RunLambda([this, data, maxlen, &bytesRead] {
		bytesRead = m_pInner->read(data, maxlen);
	});
	return bytesRead;
}

qint64 IODeviceThread::writeData(const char *data, qint64 len)
{
	qint64 bytesWritten = 0;
	m_dispatcher.RunLambda([this, data, len, &bytesWritten] {
		bytesWritten = m_pInner->write(data, len);
	});
	return bytesWritten;
}


qint64 IODeviceThread::bytesAvailable() const
{
	qint64 bytes = 0;
	m_dispatcher.RunLambda([this,  &bytes] {
		bytes = m_pInner->bytesAvailable();
	});
	return bytes;
}

qint64 IODeviceThread::size() const
{
	qint64 bytes = 0;
	m_dispatcher.RunLambda([this, &bytes] {
		bytes = m_pInner->size();
	});
	return bytes;
}


qint64 IODeviceThread::bytesToWrite() const
{
	qint64 bytes = 0;
	m_dispatcher.RunLambda([this,  &bytes] {
		bytes = m_pInner->bytesToWrite();
	});
	return bytes;
}


bool IODeviceThread::waitForBytesWritten(int msecs)
{
	bool bBool = false;
	m_dispatcher.RunLambda([this, msecs, &bBool] {
		bBool = m_pInner->waitForBytesWritten(msecs);
	});
	return bBool;
}


bool IODeviceThread::waitForReadyRead(int msecs)
{
	bool bBool = false;
	m_dispatcher.RunLambda([this, msecs, &bBool] {
		bBool = m_pInner->waitForReadyRead(msecs);
	});
	return bBool;
}
