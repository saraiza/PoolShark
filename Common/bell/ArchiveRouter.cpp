
#include "ArchiveRouter.h"
#include "Logging.h"


DECLARE_LOG_SRC("ArchiveRouter", LOGCAT_Common)


// The actual Debug Tag values
#define SERMIG_DEBUG_TAG		0xFABBABE5



ArchiveRouter::ArchiveRouter()
    : Archive(false, SerMig::OPT_None)
{
}

QDataStream::FloatingPointPrecision ArchiveRouter::floatingPointPrecision() const
{
	return m_prec;
}

void ArchiveRouter::setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec)
{
	for (Archive *pAr : m_mapInners.values())
		pAr->setFloatingPointPrecision(newPrec);

	m_prec = newPrec;

}




void ArchiveRouter::AddInner(int iKey, Archive* pAr)
{
	Q_ASSERT(pAr);
	Q_ASSERT(!m_mapInners.contains(iKey));
	m_mapInners[iKey] = pAr;
	m_pActive = pAr;	// Always have an active archive
}

void ArchiveRouter::SetActive(int iKey)
{
	Q_ASSERT(m_mapInners.contains(iKey));
	m_pActive = m_mapInners[iKey];
}


void ArchiveRouter::WriteRawData(const void *pData, int iLen)
{
	m_pActive->WriteRawData((const char*)pData, iLen);
}

int ArchiveRouter::ReadRawData(void *pData, int iLen)
{
	return m_pActive->ReadRawData((char*)pData, iLen);
}


bool ArchiveRouter::DebugTag()
{
	if (m_pActive->isStoring())
	{
		*m_pActive << (uint)SERMIG_DEBUG_TAG;
		return true;
	}
	else
	{
		uint ui;
		*m_pActive >> ui;
		return ui == SERMIG_DEBUG_TAG;
	}
}

bool ArchiveRouter::Start()
{
	for (Archive *pAr : m_mapInners.values())
		pAr->Start();
	return true;
}


bool ArchiveRouter::atEnd() const
{
	return m_pActive->atEnd();
}


// Low-level native types
void ArchiveRouter::operator>>(qint8 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(qint8 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(quint8 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(quint8 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(qint16 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(qint16 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(quint16 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(quint16 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(qint32 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(qint32 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(quint32 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(quint32 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(qint64 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(qint64 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(quint64 &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(quint64 i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(bool &i)
{
	*m_pActive >> i;
}

void ArchiveRouter::operator<<(bool i)
{
	*m_pActive << i;
}

void ArchiveRouter::operator>>(float &f)
{
	*m_pActive >> f;
}

void ArchiveRouter::operator<<(float f)
{
	*m_pActive << f;
}

void ArchiveRouter::operator>>(double &f)
{
	*m_pActive >> f;
}
void ArchiveRouter::operator<<(double f)
{
	*m_pActive << f;
}




void ArchiveRouter::operator>>(QChar &s)
{
	*m_pActive >> s;
}

void ArchiveRouter::operator<<(const QChar& s)
{
	*m_pActive << s;
}


void ArchiveRouter::operator>>(QString &s)
{
	*m_pActive >> s;
}

void ArchiveRouter::operator<<(const QString& s)
{
	*m_pActive << s;
}

void ArchiveRouter::operator>>(QByteArray &ba)
{
	*m_pActive >> ba;
}

void ArchiveRouter::operator<<(const QByteArray& ba)
{
	*m_pActive << ba;
}

void ArchiveRouter::operator>>(QVariant &var)
{
	*m_pActive >> var;
}

void ArchiveRouter::operator<<(const QVariant& var)
{
	*m_pActive << var;
}


void ArchiveRouter::operator>>(QDateTime &thing)
{
	*m_pActive >> thing;
}
void ArchiveRouter::operator<<(const QDateTime& thing)
{
	*m_pActive << thing;
}
void ArchiveRouter::operator>>(QDate &thing)
{
	*m_pActive >> thing;
}
void ArchiveRouter::operator<<(const QDate& thing)
{
	*m_pActive << thing;
}
void ArchiveRouter::operator>>(QTime &thing)
{
	*m_pActive >> thing;
}
void ArchiveRouter::operator<<(const QTime& thing)
{
	*m_pActive << thing;
}


void ArchiveRouter::operator>>(QTimeZone &thing)
{
	*m_pActive >> thing;
}

void ArchiveRouter::operator<<(const QTimeZone& thing)
{
	*m_pActive << thing;
}



void ArchiveRouter::operator<<(const QUuid& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QUuid &thing)
{
	*m_pActive >> thing;
}


void ArchiveRouter::operator<<(const QPoint& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QPoint &thing)
{
	*m_pActive >> thing;
}



void ArchiveRouter::operator<<(const QPointF& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QPointF &thing)
{
	*m_pActive >> thing;
}


void ArchiveRouter::operator<<(const QImage& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QImage &thing)
{
	*m_pActive >> thing;
}


void ArchiveRouter::operator<<(const QColor& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QColor &thing)
{
	*m_pActive >> thing;
}





void ArchiveRouter::operator<<(const QTransform& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QTransform &thing)
{
	*m_pActive >> thing;
}






void ArchiveRouter::operator<<(const QRect& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QRect &thing)
{
	*m_pActive >> thing;
}


void ArchiveRouter::operator<<(const QRectF& thing)
{
	*m_pActive << thing;
}

void ArchiveRouter::operator>>(QRectF &thing)
{
	*m_pActive >> thing;
}
