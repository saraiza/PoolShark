#include <QUuid>
#include <QDataStream>
#include <QImage>
#include "ArchiveBinaryV1.h"
#include "Logging.h"

DECLARE_LOG_SRC("ArchiveBinaryV1", LOGCAT_Common)


// The actual Debug Tag values
#define SERMIG_DEBUG_TAG		0xFABBABE5



ArchiveBinaryV1::ArchiveBinaryV1(QIODevice *pStream, SerMig::Options options)
	: Archive(pStream->openMode().testFlag(QIODevice::WriteOnly), options)
	, m_dataStream(pStream)
{
	m_dataStream.setByteOrder(QDataStream::LittleEndian);
	if(!options.testFlag(SerMig::OPT_Naked))
		InitDebugTags();
}

QDataStream::FloatingPointPrecision ArchiveBinaryV1::floatingPointPrecision() const
{
	return m_dataStream.floatingPointPrecision();
}

void ArchiveBinaryV1::setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec)
{
	m_dataStream.setFloatingPointPrecision(newPrec);
}



void ArchiveBinaryV1::WriteRawData(const void *pData, int iLen)
{
	m_dataStream.writeRawData((const char*)pData, iLen);
}

int ArchiveBinaryV1::ReadRawData(void *pData, int iLen)
{
	return m_dataStream.readRawData((char*)pData, iLen);
}


bool ArchiveBinaryV1::DebugTag()
{
	if (isStoring())
	{
		*this << (uint)SERMIG_DEBUG_TAG;
		return true;
	}
	else
	{
		uint ui;
		*this >> ui;
		return ui == SERMIG_DEBUG_TAG;
	}
}

bool ArchiveBinaryV1::Start()
{
	// No id-tag for binary
	return true;
}


bool ArchiveBinaryV1::atEnd() const
{
	return m_dataStream.atEnd();
}


// Low-level native types
void ArchiveBinaryV1::operator>>(qint8 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(qint8 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(quint8 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(quint8 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(qint16 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(qint16 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(quint16 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(quint16 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(qint32 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(qint32 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(quint32 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(quint32 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(qint64 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(qint64 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(quint64 &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(quint64 i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(bool &i)
{
	m_dataStream >> i;
}

void ArchiveBinaryV1::operator<<(bool i)
{
	m_dataStream << i;
}

void ArchiveBinaryV1::operator>>(float &f)
{
	m_dataStream >> f;
}

void ArchiveBinaryV1::operator<<(float f)
{
	m_dataStream << f;
}

void ArchiveBinaryV1::operator>>(double &f)
{
	m_dataStream >> f;
}
void ArchiveBinaryV1::operator<<(double f)
{
	m_dataStream << f;
}




void ArchiveBinaryV1::operator>>(QChar &s)
{
	m_dataStream >> s;
}

void ArchiveBinaryV1::operator<<(const QChar& s)
{
	m_dataStream << s;
}


void ArchiveBinaryV1::operator>>(QString &s)
{
	m_dataStream >> s;
}

void ArchiveBinaryV1::operator<<(const QString& s)
{
	m_dataStream << s;
}

void ArchiveBinaryV1::operator>>(QByteArray &ba)
{
	m_dataStream >> ba;
}

void ArchiveBinaryV1::operator<<(const QByteArray& ba)
{
	m_dataStream << ba;
}

void ArchiveBinaryV1::operator>>(QVariant &var)
{
	m_dataStream >> var;
}

void ArchiveBinaryV1::operator<<(const QVariant& var)
{
	m_dataStream << var;
}


void ArchiveBinaryV1::operator>>(QDateTime &thing)
{
	m_dataStream >> thing;
}
void ArchiveBinaryV1::operator<<(const QDateTime& thing)
{
	m_dataStream << thing;
}
void ArchiveBinaryV1::operator>>(QDate &thing)
{
	m_dataStream >> thing;
}
void ArchiveBinaryV1::operator<<(const QDate& thing)
{
	m_dataStream << thing;
}
void ArchiveBinaryV1::operator>>(QTime &thing)
{
	m_dataStream >> thing;
}
void ArchiveBinaryV1::operator<<(const QTime& thing)
{
	m_dataStream << thing;
}


void ArchiveBinaryV1::operator>>(QTimeZone &thing)
{
	m_dataStream >> thing;
}

void ArchiveBinaryV1::operator<<(const QTimeZone& thing)
{
	m_dataStream << thing;
}



void ArchiveBinaryV1::operator<<(const QUuid& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QUuid &thing)
{
	m_dataStream >> thing;
}


void ArchiveBinaryV1::operator<<(const QPoint& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QPoint &thing)
{
	m_dataStream >> thing;
}



void ArchiveBinaryV1::operator<<(const QPointF& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QPointF &thing)
{
	m_dataStream >> thing;
}


void ArchiveBinaryV1::operator<<(const QImage& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QImage &thing)
{
	m_dataStream >> thing;
}


void ArchiveBinaryV1::operator<<(const QColor& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QColor &thing)
{
	m_dataStream >> thing;
}





void ArchiveBinaryV1::operator<<(const QTransform& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QTransform &thing)
{
	m_dataStream >> thing;
}






void ArchiveBinaryV1::operator<<(const QRect& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QRect &thing)
{
	m_dataStream >> thing;
}


void ArchiveBinaryV1::operator<<(const QRectF& thing)
{
	m_dataStream << thing;
}

void ArchiveBinaryV1::operator>>(QRectF &thing)
{
	m_dataStream >> thing;
}
