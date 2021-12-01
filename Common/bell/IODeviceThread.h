#pragma once

#include <QObject>
#include <QIODevice>
#include <QPointer>
#include <QThread>
#include "LambdaDispatch.h"

/**
@brief Wrapper around QIODevice to use it from a single thread

Some QIODevice classes can't be used from multiple threads. This can cause
you much grief when implementing protocols.  This class remotes ALL interactions
with the QIODevice to the internal Task thread.

Provide the QIODevice instance in the Start method.
*/
class IODeviceThread : public QIODevice
{
	Q_OBJECT
public:
	IODeviceThread(const QString& sName, QObject *parent = nullptr);
	virtual ~IODeviceThread();

	void Start(QIODevice *pDevice, QFlags<QIODevice::OpenModeFlag> flags = QIODevice::ReadWrite);
	void Stop();

	// QIODevice implementation. We remote some additional special methods.
	bool isOpen() const;
	virtual qint64 size() const;
	virtual qint64 bytesAvailable() const override;
	virtual qint64 bytesToWrite() const override;
	virtual bool waitForBytesWritten(int msecs) override;
	virtual bool waitForReadyRead(int msecs) override;

private:
	// QIODevice implemnetation
	virtual qint64 readData(char *data, qint64 maxlen) override;
	virtual qint64 writeData(const char *data, qint64 len) override;

	QPointer<QIODevice> m_pInner;		///< The actual device we are talking to
	QThread m_thread;
	LambdaDispatch m_dispatcher;
	QString m_sName;
};
