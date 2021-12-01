#include "SerMig.h"
#include <QMetaType>
#include <QBuffer>
#include "ChecksumDevice.h"
#include <stdio.h>
#include "Progress.h"
#include "Logging.h"

#ifdef WIN32
#include <io.h>
#include <windows.h>
#endif


DECLARE_LOG_SRC("SerMig", LOGCAT_Common);



/********************* SerMig *************************/


SerMig::SerMig(void)
{
}

SerMig::~SerMig(void)
{
}



void SerMig::Serialize(Archive& ar)
{
	// Redirect to the macro-built implementation
	// Doing this indirection avoids any infinite
	// call loops.
	_Serialize(ar);	
}


void SerMig::_Serialize_Begin(Archive& ar, FORMATVER fmtVerWrite)
{
	if (ar.isStoring()) 
	{
		ar.Tag(className());
		ar.Indent();
		ar << fmtVerWrite;
		ar << ar.AddObjW(this);
	}
	else 
	{
		ar.Tag(className()); 
			ar.Indent();
		ar >> m_formatVer;
		int id = ar.ReadInt();
		ar.AddObjR(id, this);
	}
}


void SerMig::_Serialize_End(Archive& ar)
{
	ar.Unindent();
	ar._DebugTagEnd(this);
}



void SerMig::Serialize_LEGACY(Archive& ar)
{
	if(ar.isStoring())
		SerMig::LegacyWR(ar, this);
	else
		SerMig::LegacyRD(ar, this);
}




QByteArray SerMig::toBlob(Options options) const
{
	Q_ASSERT(!options.testFlag(SerMig::OPT_Checksum)); // Checksum option is not supported when serializing to a blob
	Q_ASSERT(!options.testFlag(SerMig::OPT_EnsureFlushed)); // It doesn't make sense to flush to a blob

	// Setup the streaming into the byte array
	QByteArray blob;
	blob.reserve(128); // Attempt to reduce mallocs
	
	QBuffer buffer(&blob);
	buffer.open(QBuffer::WriteOnly);	// Should always succeed

	// Create the proper archive type. Start() is called from within.
	Archive *pAr = Archive::Create(&buffer, options);
	QScopedPointer<Archive> spAr(pAr);	// Auto-delete when it goes out of scope

	// A little const hackness here.  We know that we are not touching the object, so this is
	// a const method. But we are using Serialize() to do so, which can do both. Cast away the const-ness!
	SerMig *pSerMig = const_cast<SerMig*>(this);
	pSerMig->Serialize(*pAr);
	buffer.close();	

	return blob;
}

void SerMig::fromBlob(const QByteArray& blob, Options options)
{
	// Setup the streaming from the byte array
	QBuffer buffer((QByteArray*)&blob);	// It's ok to cast-away constness
	buffer.open(QBuffer::ReadOnly);	// Should always succeed	

	Archive *pAr = Archive::Create(&buffer, options);
	QScopedPointer<Archive> spAr(pAr);	// Auto-delete when it goes out of scope

	Serialize(*pAr);
}



void SerMig::toFile(const QString& filename, Options options) const
{
	QFile buff(filename);
	QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Truncate;
	if (options.testFlag(SerMig::OPT_EnsureFlushed))
		mode |= QIODevice::Unbuffered;
	if (!buff.open(mode))
		throw ArchiveException(QString("Could not open file '%1' for writing").arg(filename));
	toFile(&buff, options);
}



#define FOOTER_SANITY          0xBABECAFE
#define FOOTER_VERSION          1

#pragma pack(push, 1)
struct FOOTER_CHECKSUM_V1
{
    uint checksumSize = 0;		///< The number of bytes in the checksum. The checksum immediately precedes this footer.
    uint version = 0;			///< Always FOOTER_VERSION, until we need more features later
    uint sanity = 0;			///< Always FOOTER_SANITY
};
#pragma pack(pop)


void SerMig::toFile(QIODevice *pDevice, Options options) const
{
    bool bForceDebugTagWrite = options.testFlag(OPT_DebugTags);
    bool bChecksum = options.testFlag(OPT_Checksum) || options.testFlag(OPT_ChecksumVerifyOnWrite);
	bool bText = options.testFlag(OPT_Text);
	bool bFlush = options.testFlag(OPT_EnsureFlushed);

	// Create the proper archive type. Start() is called from within.
	Archive *pAr = Archive::Create(pDevice, options);
	QScopedPointer<Archive> spAr(pAr);	// Auto-delete when it goes out of scope

    // A little const hackness here.  We know that we are not touching the object, so this is
    // a const method. But we are using Serialize() to do so, which can do both. Cast away the const-ness!
    SerMig *pSerMig = const_cast<SerMig*>(this);
	pSerMig->Serialize(*pAr);

#ifdef WIN32
	QFileDevice* pFileDev = dynamic_cast<QFileDevice*>(pDevice);
	if (bFlush && pFileDev != nullptr)
	{
		HANDLE hFile = (HANDLE)_get_osfhandle(pFileDev->handle());
		FlushFileBuffers(hFile);
	}
#endif
}


/*


	// Checksum feature, insert the checksum device in the chain
	ChecksumDevice csd(pDevice);
	csd.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
	ArchiveBinaryV1 arW(&csd, bForceDebugTagWrite);
	pSerMig->Serialize(arW);

	// Grab the checksum
	QByteArray checksum = csd.Result();

	// Build the footer
	FOOTER_CHECKSUM_V1 footer;
	footer.version = FOOTER_VERSION;
	footer.sanity = FOOTER_SANITY;
	footer.checksumSize = checksum.size();

	// Flush the archive just in case, we are going to write directly to the device under it
	// (don't know how)

	// Write the checksum data (just the data, not a serialized QByteArray)
	auto checksumSize = pDevice->write(checksum);
	Q_ASSERT(footer.checksumSize == checksumSize);

	// Write the footer
	auto footerSize = pDevice->write((const char*)&footer, sizeof(footer));
	Q_ASSERT(footerSize == sizeof(footer));

	pDevice->close();

	// Perform checksum verification after writing?
	if (options.testFlag(OPT_ChecksumVerifyOnWrite))
	{
		Progress prog;
		QString sVerifyErr = _VerifyChecksum(pDevice, prog);
		if (!sVerifyErr.isEmpty())
			EXERR_T(ChecksumException(sVerifyErr));
	}
	// Checksum feature will imply binary


*/

void SerMig::fromFile(QIODevice *pDevice, Options options)
{
	// Create the proper archive type. Start() is called from within.
	Archive *pAr = Archive::Create(pDevice, options);
	QScopedPointer<Archive> spAr(pAr);	// Auto-delete when it goes out of scope

	Serialize(*pAr);
}



void SerMig::fromFile(const QString& filename, Options options)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
		throw ArchiveException(QString("Could not open file '%1' for reading").arg(filename));
	fromFile(&file, options);
	file.close();
}

/// static
QString SerMig::_VerifyChecksum(QIODevice *pDevice, Progress& prog)
{
    // Rewind to the end, less the footer and read the footer
    FOOTER_CHECKSUM_V1 footer;

	if (pDevice->isOpen())
		pDevice->close();	// might be in write-only mode. Just close and open it again. Also flushes any file buffers.

	if (!pDevice->open(QIODevice::ReadOnly))
		return QString("Could not open for verification");

	if (!pDevice->seek(pDevice->size() - sizeof(footer)))
		return QString("Missing checksum footer");
    int iBytesRead = pDevice->read((char*)&footer, sizeof(footer));

    // Verify a few things
	if (iBytesRead != sizeof(footer))
		return QString("Footer does not exist");
    if(footer.sanity != FOOTER_SANITY)
		return QString("Invalid serialization footer");
    if(footer.version != FOOTER_VERSION)
		return QString("Unknown serialization footer version");

    // Grab the checksum data
	auto fileSizeWithoutFooter = pDevice->size() - footer.checksumSize - sizeof(footer);
	pDevice->seek(fileSizeWithoutFooter);
	QByteArray checksum = pDevice->read(footer.checksumSize);
	Q_ASSERT(checksum.size() == footer.checksumSize);

    // Run the checksum on the raw file (without the appended checksum and footer).
    // We will read a bunch of data through it, but we ignore the data.
	// (we just care about the checksum)
    pDevice->seek(0);
    ChecksumDevice csd(pDevice);
	// Even though it's an intermediate shim, we still need to open it. Unbuffered prevents our
	// shim class from reading ahead and doing checksums on the checksum.
	csd.open(QIODevice::ReadOnly | QIODevice::Unbuffered);	
	quint64 fileSize = pDevice->size() - sizeof(footer) - footer.checksumSize;
	quint64 bytesLeft = pDevice->size() - sizeof(footer) - footer.checksumSize;
	quint64 bytesRead = 0;
    while(bytesLeft)
    {
		quint64 chunkSize = qMin(bytesLeft, (quint64)1048576);
		QByteArray ba = csd.read(chunkSize);
		Q_ASSERT(ba.count() == chunkSize);
		bytesLeft -= ba.count();
		bytesRead += ba.count();
		prog.SetProgress(100.0 * bytesRead / fileSize);
    }

	// We ran the data, grab the checksum
	QByteArray checksumActual = csd.Result();

	// Do they match?
	if (checksum != checksumActual)
		return QString("Checksum Error, probably corrupted media");
	
	// No error
	return QString();
}

// static 
QString SerMig::VerifyChecksum(const QString& filename, Progress& prog)
{
	QFile file(filename);
	return _VerifyChecksum(&file, prog);
}

void SerMig::toFileAtomic(const QString& filename, Options options) const
{
	// Make a new temporary file to write to
	QString sNewFile = filename + ".new";
	if (QFile::exists(sNewFile))
		QFile::remove(sNewFile);

	// Write the file
	toFile(sNewFile, options);

	// Rename the existing file to bak if it exists
	QString sBakFile = filename + ".bak";
	if (QFile::exists(filename))
	{
		if (QFile::exists(sBakFile))
			QFile::remove(sBakFile);
		QFile::rename(filename, sBakFile);
	}

	// The old file is renamed to bak, now we can move the new file over
	QFile::rename(sNewFile, filename);

	// Finally, we can remove the bak file (if it exists)
	if (QFile::exists(sBakFile))
		QFile::remove(sBakFile);
}


void SerMig::fromFileAtomic(const QString& filename, Options options)
{
	// If something went wrong in the last save, there will be a .bak file
	QString sBakFile = filename + ".bak";
	if (!QFile::exists(sBakFile))
	{
		// No special tricks required!
		fromFile(filename);
		return;
	}

	// Load from the bak file (we won't bother cleaning up, we will assume that a future write will do that)
	fromFile(sBakFile, options);
}

TODO("This should have been base64, not pctEncoding");

QString SerMig::toPctEncodedString() const
{
	QByteArray ba = this->toBlob();
	QString sDevCal = ba.toPercentEncoding();
	return sDevCal;
}

void SerMig::fromPctEncodedString( const QString& strTxt )
{
	QByteArray baUtf8 = strTxt.toUtf8();
	QByteArray ba = QByteArray::fromPercentEncoding(baUtf8);
	this->fromBlob(ba);
}

void SerMig::toPctEncodedTxtFile( const QString& filename, Options options /*= OPT_None*/ ) const
{
	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
		throw ArchiveException(QString("Could not open file '%1' for writing").arg(filename));

	QTextStream out(&file);
	out << this->toPctEncodedString();
	out.flush();
	file.close();
}

void SerMig::fromPctEncodedTxtFile( const QString& filename )
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		throw ArchiveException(QString("Could not open file '%1' for reading").arg(filename));

	QTextStream inStream(&file);
	QString strPctEnc = inStream.readAll();
	this->fromPctEncodedString(strPctEnc.trimmed());
}


/// static
void SerMig::LegacyRD(Archive& ar, SerMig *pObj)
{ 
	QByteArray blob; 
	ar >> blob; 
	pObj->fromBlob(blob);
} 

/// static
void SerMig::LegacyWR(Archive& ar, const SerMig *pObj)
{ 
	QByteArray blob = pObj->toBlob(SerMig::OPT_None);
	ar << blob; 
} 

/******************************************************************/

FloatPrecisionDefender::FloatPrecisionDefender(Archive* s, QDataStream::FloatingPointPrecision newPrec)
{
	m_stream = NULL;
	m_oldPrec = s->floatingPointPrecision();

	// If the old precision is the same, there's nothing to do here, so we don't bother
	// to cache the archive
	if (m_oldPrec != newPrec)
	{
		m_stream = s;
		m_stream->setFloatingPointPrecision(newPrec);
	}
}

FloatPrecisionDefender::~FloatPrecisionDefender()
{
	if (m_stream)
		m_stream->setFloatingPointPrecision(m_oldPrec);
}
