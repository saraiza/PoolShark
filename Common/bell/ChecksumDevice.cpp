#include "ChecksumDevice.h"
#include <QIODevice>

bool ChecksumDevice::open(QFlags<QIODevice::OpenModeFlag> flags)
{
	// Special case where we always want the flags to include Unbuffered
	// the internal device can do the buffering. If we don't do this
	// QIODevice will attempt to read the full block size of the device
	// however if this is smaller than the file, the hash will expect
	// maxsize = block size, and result < blocksize...
	// Q_ASSERT(result == maxSize); will be false
	return QIODevice::open(flags | QIODevice::Unbuffered);
}

ChecksumDevice::ChecksumDevice(QIODevice *pInner, QCryptographicHash::Algorithm algo)
 : m_hasher(algo)
{
    m_pInner = pInner;
}

QByteArray ChecksumDevice::Result() const
{
    return m_hasher.result();
}

qint64 ChecksumDevice::readData(char * data, qint64 maxSize)
{
	qint64 result = m_pInner->read(data, maxSize);
	if (result > 0)
		m_uiBytes += result;
	Q_ASSERT(result == maxSize);
	m_hasher.addData(data, result);
	return result;
}

qint64 ChecksumDevice::readLineData(char * data, qint64 maxSize)
{
	qint64 result = m_pInner->readLine(data, maxSize);
	if (result > 0)
		m_uiBytes += result;
	Q_ASSERT(result == maxSize);
	m_hasher.addData(data, result);
	return result;
}

qint64 ChecksumDevice::writeData(const char * data, qint64 maxSize)
{
    // Give the data to our hasher
    m_hasher.addData(data, maxSize);
	m_uiBytes += maxSize;

    // Now we can forward it to the inner (real) device.
	auto numbytes = m_pInner->write(data, maxSize);
	Q_ASSERT(numbytes == maxSize);	// write error, corrupting your serialized data :(
    return numbytes;
}

