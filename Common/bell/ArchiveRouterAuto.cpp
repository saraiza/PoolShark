
#include "ArchiveRouterAuto.h"
#include "Logging.h"


DECLARE_LOG_SRC("ArchiveRouterAuto", LOGCAT_Common)


// The actual Debug Tag values
#define SERMIG_DEBUG_TAG		0xFABBABE5



ArchiveRouterAuto::ArchiveRouterAuto()
    : Archive(true, SerMig::OPT_None)
{
}

QDataStream::FloatingPointPrecision ArchiveRouterAuto::floatingPointPrecision() const
{
	return m_pArR->floatingPointPrecision();
}

void ArchiveRouterAuto::setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec)
{
	m_pArR->setFloatingPointPrecision(newPrec);
	m_pArW->setFloatingPointPrecision(newPrec);
}




void ArchiveRouterAuto::SetInner(Archive* pArR, Archive* pArW)
{
	Q_ASSERT(pArR);
	Q_ASSERT(pArW);
	m_pArR = pArR;
	m_pArW = pArW;
}


void ArchiveRouterAuto::WriteRawData(const void *pData, int iLen)
{
	m_pArW->WriteRawData((const char*)pData, iLen);
}

int ArchiveRouterAuto::ReadRawData(void *pData, int iLen)
{
	return m_pArR->ReadRawData((char*)pData, iLen);
}


bool ArchiveRouterAuto::DebugTag()
{
	Q_ASSERT(false);
	// Can't use this feature through this class
	return false;
}

bool ArchiveRouterAuto::Start()
{
	m_pArW->Start();
	m_pArR->Start();
	return true;
}


bool ArchiveRouterAuto::atEnd() const
{
	return false; // We can't know which one
}


// Low-level native types
void ArchiveRouterAuto::operator>>(qint8 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(qint8 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(quint8 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(quint8 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(qint16 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(qint16 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(quint16 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(quint16 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(qint32 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(qint32 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(quint32 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(quint32 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(qint64 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(qint64 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(quint64 &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(quint64 i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(bool &i)
{
	*m_pArR >> i;
}

void ArchiveRouterAuto::operator<<(bool i)
{
	*m_pArW << i;
}

void ArchiveRouterAuto::operator>>(float &f)
{
	*m_pArR >> f;
}

void ArchiveRouterAuto::operator<<(float f)
{
	*m_pArW << f;
}

void ArchiveRouterAuto::operator>>(double &f)
{
	*m_pArR >> f;
}
void ArchiveRouterAuto::operator<<(double f)
{
	*m_pArW << f;
}




void ArchiveRouterAuto::operator>>(QChar &s)
{
	*m_pArR >> s;
}

void ArchiveRouterAuto::operator<<(const QChar& s)
{
	*m_pArW << s;
}


void ArchiveRouterAuto::operator>>(QString &s)
{
	*m_pArR >> s;
}

void ArchiveRouterAuto::operator<<(const QString& s)
{
	*m_pArW << s;
}

void ArchiveRouterAuto::operator>>(QByteArray &ba)
{
	*m_pArR >> ba;
}

void ArchiveRouterAuto::operator<<(const QByteArray& ba)
{
	*m_pArW << ba;
}

void ArchiveRouterAuto::operator>>(QVariant &var)
{
	*m_pArR >> var;
}

void ArchiveRouterAuto::operator<<(const QVariant& var)
{
	*m_pArW << var;
}


void ArchiveRouterAuto::operator>>(QDateTime &thing)
{
	*m_pArR >> thing;
}
void ArchiveRouterAuto::operator<<(const QDateTime& thing)
{
	*m_pArW << thing;
}
void ArchiveRouterAuto::operator>>(QDate &thing)
{
	*m_pArR >> thing;
}
void ArchiveRouterAuto::operator<<(const QDate& thing)
{
	*m_pArW << thing;
}
void ArchiveRouterAuto::operator>>(QTime &thing)
{
	*m_pArR >> thing;
}
void ArchiveRouterAuto::operator<<(const QTime& thing)
{
	*m_pArW << thing;
}


void ArchiveRouterAuto::operator>>(QTimeZone &thing)
{
	*m_pArR >> thing;
}

void ArchiveRouterAuto::operator<<(const QTimeZone& thing)
{
	*m_pArW << thing;
}



void ArchiveRouterAuto::operator<<(const QUuid& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QUuid &thing)
{
	*m_pArR >> thing;
}


void ArchiveRouterAuto::operator<<(const QPoint& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QPoint &thing)
{
	*m_pArR >> thing;
}



void ArchiveRouterAuto::operator<<(const QPointF& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QPointF &thing)
{
	*m_pArR >> thing;
}


void ArchiveRouterAuto::operator<<(const QImage& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QImage &thing)
{
	*m_pArR >> thing;
}


void ArchiveRouterAuto::operator<<(const QColor& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QColor &thing)
{
	*m_pArR >> thing;
}





void ArchiveRouterAuto::operator<<(const QTransform& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QTransform &thing)
{
	*m_pArR >> thing;
}






void ArchiveRouterAuto::operator<<(const QRect& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QRect &thing)
{
	*m_pArR >> thing;
}


void ArchiveRouterAuto::operator<<(const QRectF& thing)
{
	*m_pArW << thing;
}

void ArchiveRouterAuto::operator>>(QRectF &thing)
{
	*m_pArR >> thing;
}
