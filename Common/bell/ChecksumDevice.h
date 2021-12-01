#ifndef CHECKSUMDEVICE_H
#define CHECKSUMDEVICE_H

#include <QIODevice>
#include <QCryptographicHash>

/**
 * @brief Layered IODevice that hashes data as it flies by.
 *
 * This class was created as part of adding the checksum feature
 * to the SerMig class.  ChecksumDevice is intended to be inserted
 * above the actual QIODevice instance being used.
 */
class ChecksumDevice : public QIODevice
{
public:

	bool open(QFlags<QIODevice::OpenModeFlag> flags) override;

    ChecksumDevice(QIODevice *pInner, QCryptographicHash::Algorithm algo = QCryptographicHash::Md5);

    /// Get the result after the run is complete
    QByteArray Result() const;

    // Forward all virtual functions down to the inner.
    virtual qint64	readData(char * data, qint64 maxSize) override;
    virtual qint64	readLineData(char * data, qint64 maxSize) override;
    virtual qint64	writeData(const char * data, qint64 maxSize) override;

private:
    QIODevice *m_pInner = NULL;
    QCryptographicHash m_hasher;
	uint m_uiBytes = 0;
};

#endif // CHECKSUMDEVICE_H
