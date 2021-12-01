#include "ArchiveTextV1.h"
#include "Logging.h"
#include <QTextStream>
#include <QUuid>
#include <QImage>
#include <QRegularExpression>
#include <QStringConverter>


DECLARE_LOG_SRC("ArchiveTextV1", LOGCAT_Common)


// Format strings. Binary has none.
// Keep these the same length
#define FORMATSTRING_TEXT_V1	"SM-TEXT-V1"
//#define FORMATSTRING_LEN		10

#define INDENT					4
#define CRLF					"\r\n"	// All line endings use Windows-style 


ArchiveTextV1::ArchiveTextV1(QIODevice *pStream, SerMig::Options options)
	: Archive(pStream->openMode().testFlag(QIODevice::WriteOnly), options)
{
	m_pStream = pStream;
}

ArchiveTextV1::~ArchiveTextV1()
{
	if (m_pTextStream)
	{
		delete m_pTextStream;
		m_pTextStream = nullptr;
	}
}


QDataStream::FloatingPointPrecision ArchiveTextV1::floatingPointPrecision() const
{
	return QDataStream::FloatingPointPrecision::SinglePrecision;
}

void ArchiveTextV1::setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec)
{
}



bool ArchiveTextV1::atEnd() const
{
	return m_pTextStream->atEnd();
}


bool ArchiveTextV1::Start()
{
	Q_ASSERT(nullptr == m_pTextStream);

	if (isStoring())
	{
		m_pTextStream = new QTextStream(m_pStream);
		m_pTextStream->setEncoding(QStringConverter::Encoding::Utf8);
		*m_pTextStream << FORMATSTRING_TEXT_V1 << CRLF;
		return true;
	}

	// Reading, peek at the formattype string.
	const int iTagLen = sizeof(FORMATSTRING_TEXT_V1);
	char szType[iTagLen];
	szType[sizeof(szType) - 1] = 0;	// Pre null-terminate
	m_pStream->peek(szType, iTagLen - 1);	// Only read in the text, no cr
	if (0 != strcmp(szType, FORMATSTRING_TEXT_V1))
		return false;	// Not our type

	// This is our format type! Setup the text stream, we can now begin
	// using it. saraiza: setting up the stream in the constuctor was
	// causing access violations in the situation where a ArchiveText
	// was created, deleted, then ArchiveBinary was created and used.
	m_pTextStream = new QTextStream(m_pStream);
	m_pTextStream->setEncoding(QStringConverter::Encoding::Utf8);

	// It is our type, truly remove the first line	
	QString sFmt = _ReadLine();
	return true;
}


void ArchiveTextV1::Indent()
{
	if (isLoading())
		return;	 // Nothing to do when reading
	m_indent.i += INDENT;
	m_indent.s.fill(' ', m_indent.i);
}

void ArchiveTextV1::Unindent()
{
	if (isLoading())
		return;	 // Nothing to do when reading
	Q_ASSERT(m_indent.i > 0);
	m_indent.i -= INDENT;
	m_indent.s.fill(' ', m_indent.i);
}



void ArchiveTextV1::Write(const QString& s)
{
	if (m_sLabel.isEmpty())
	{
		*m_pTextStream << m_indent.s << s << CRLF;
		return;
	}

	// There is to be a label
	*m_pTextStream << m_indent.s << m_sLabel << ": " << s << CRLF;
	m_sLabel.clear();
}


QString ArchiveTextV1::_ReadLine()
{
	++m_iLineNum;
	return m_pTextStream->readLine();
}

QString ArchiveTextV1::Read()
{
	return ReadNoTrim().trimmed();
}

QString ArchiveTextV1::ReadNoTrim()
{
	if (m_sLabel.isEmpty())
		return _ReadLine();

	// There is to be a label
	QString sLine = _ReadLine();

	// Split it out
	int iPos = sLine.indexOf(':');
	Q_ASSERT(-1 != iPos);
	if(-1 == iPos)
		EXERR("MKAY", "Archive label '%s' expected, but none found", qPrintable(m_sLabel));
	QString sLabel = sLine.left(iPos).trimmed();
	if (sLabel != m_sLabel)
		EXERR("MKAV", "Archive label mismatch. '%s' found, '%s' expected", qPrintable(sLabel), qPrintable(m_sLabel));
	QString sValue = sLine.right(sLine.count() - iPos - 1);
	m_sLabel.clear();
	return sValue;
}

Archive& ArchiveTextV1::label(const QString& sLabel)
{
	if (m_sLabel.isEmpty())
		m_sLabel = sLabel;
	else
		m_sLabel += ";" + sLabel;

	return *this;
}


void ArchiveTextV1::Tag(const QString& sTag)
{
	if (isStoring())
	{
		Write(sTag);
		return;
	}

	QString s = Read();
	Q_ASSERT(s == sTag);
	if(s!= sTag)
		EXERR("99AP", "Tag mismatch, read '%s' but expected '%s'", qPrintable(s), qPrintable(sTag));
}


void ArchiveTextV1::WriteRawData(const void *pData, int iLen)
{
	QByteArray ba((const char*)pData, iLen);
	QByteArray baBase64 = ba.toBase64();
	QString sBase64 = baBase64;

	// raw data can be big and bulky. Break it up into
	// multiple lines. Otherwise, text editors might call
	// this binary data if an individual line is very long.
	// No fun for diffing.
	QString sLine = QString("%1:").arg(sBase64.length());
	const int iMaxChars = 100;
	while (!sBase64.isEmpty())
	{
		int iLineLen = qMin(iMaxChars, sBase64.size());
		sLine += sBase64.left(iLineLen);
		Write(sLine);
		sLine.clear();	// playing games with the first line
		sBase64.remove(0, iLineLen);
	}
}


int ArchiveTextV1::ReadRawData(void *pData, int len)
{
	// Read the entire line (there might be more that we need to read)
	QString sLine = Read();

	// Get the count from the line
	int iPos = sLine.indexOf(':');
	Q_ASSERT(iPos != -1);
	QString sCount = sLine.left(iPos);
	bool bOk;
	int iStrLen = sCount.toInt(&bOk);
	if (!bOk)
		EXERR("9ERT", "Invalid count for string");

	// Keep the right side of the line
	QString sBase64 = sLine.right(sLine.count() - iPos - 1);

	// How much more is there to read?
	int iRemaining = iStrLen - sBase64.count();
	Q_ASSERT(iRemaining >= 0);
	while (iRemaining)
	{
		// Read one line at a time
		QString sMore = Read();
		sBase64 += sMore;
		iRemaining -= sMore.count();
		Q_ASSERT(iRemaining >= 0);
	}


	QByteArray baBase64 = sBase64.toUtf8();
	QByteArray ba = QByteArray::fromBase64(baBase64);
	if(ba.count() != len)
		EXERR("99JP", "raw data size mismatch");
	memcpy(pData, ba.data(), len);
	return len;
}



bool ArchiveTextV1::DebugTag()
{
	static QString sTagConst("DBGTAG");
	if (isStoring())
	{
		*this << sTagConst;
		return true;
	}
	else
	{
		QString sTag;
		*this >> sTag;
		bool bTagOk = (sTag == sTagConst);
		return bTagOk;
	}
}



// Low-level native types


void ArchiveTextV1::operator<<(qint8 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(qint8 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toInt(&bOk);
	if(!bOk)
		EXERR("MKA0", "Invalid qint8 data %s", qPrintable(s));	
}

void ArchiveTextV1::operator<<(quint8 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(quint8 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toUInt(&bOk);
	if (!bOk)
		EXERR("MKA1", "Invalid quint8 data %s", qPrintable(s));
}

void ArchiveTextV1::operator<<(qint16 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(qint16 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toShort(&bOk);
	if (!bOk)
		EXERR("MKA2", "Invalid qint16 data %s", qPrintable(s));
}


void ArchiveTextV1::operator<<(quint16 i)
{
	Write(QString("%1").arg(i));
}


void ArchiveTextV1::operator>>(quint16 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toUShort(&bOk);
	if (!bOk)
		EXERR("MKA2", "Invalid quint16 data %s", qPrintable(s));
}

void ArchiveTextV1::operator<<(qint32 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(qint32 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toInt(&bOk);
	if (!bOk)
		EXERR("MKA3", "Invalid qint32 data '%s'", qPrintable(s));
}

void ArchiveTextV1::operator<<(quint32 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(quint32 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toUInt(&bOk);
	if (!bOk)
		EXERR("MKA4", "Invalid quint32 data '%s'", qPrintable(s));
}



void ArchiveTextV1::operator<<(qint64 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(qint64 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toLongLong(&bOk);
	if (!bOk)
		EXERR("MKA5", "Invalid qint64 data %s", qPrintable(s));
}

void ArchiveTextV1::operator<<(quint64 i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(quint64 &i)
{
	QString s = Read();
	bool bOk;
	i = s.toULongLong(&bOk);
	if (!bOk)
		EXERR("MKA5", "Invalid quint64 data %s", qPrintable(s));
}



void ArchiveTextV1::operator<<(bool i)
{
	Write(QString("%1").arg(i));
}

void ArchiveTextV1::operator>>(bool &i)
{
	QString s = Read();
	bool bOk;
	i = s.toInt(&bOk);
	if (!bOk)
		EXERR("MKA6", "Invalid bool data %s", qPrintable(s));
}


void ArchiveTextV1::operator<<(float f)
{
	// To avoid any loss, we use a hex representation
	// of the data in addition to the display string
	QByteArray ba((const char*)&f, sizeof(f));
	QString sHex = ba.toHex();
	Write(QString("%1 %2").arg(sHex).arg(f));
}

void ArchiveTextV1::operator>>(float &f)
{
	QString s = Read();
	QStringList sl = s.split(' ');
	if(2 != sl.count())
		EXERR("MKB7", "Invalid float data '%s'", qPrintable(s));
	QString sHex = sl.first();
	QByteArray baHex = QByteArray::fromHex(sHex.toLatin1());
	float* pF = (float*)baHex.constData();
	f = *pF;
}


void ArchiveTextV1::operator<<(double d)
{
	// To avoid any loss, we use a hex representation
	// of the data in addition to the display string
	QByteArray ba((const char*)&d, sizeof(d));
	QString sHex = ba.toHex();
	Write(QString("%1 %2").arg(sHex).arg(d));
}

void ArchiveTextV1::operator>>(double &d)
{
	QString s = Read();
	QStringList sl = s.split(' ');
	if (2 != sl.count())
		EXERR("MKA8", "Invalid double data '%s'", qPrintable(s));
	QString sHex = sl.first();
	QByteArray baHex = QByteArray::fromHex(sHex.toLatin1());
	double* pD = (double*)baHex.constData();
	d = *pD;
}


void ArchiveTextV1::operator<<(const QChar& s)
{
	Write(s);
}


void ArchiveTextV1::operator>>(QChar &s)
{
	QString sLine = Read();
	s = sLine.at(0);
}


bool ArchiveTextV1::IsMultiLine(const QString& s) const
{	
	// Rather than running through each string twice, we will do the 
	// scan manually.
	int iLen = s.count();
	const QChar *pPos = s.constData();
	const QChar *pEnd = pPos + iLen;
	bool bMultiline = false;
	while (pPos < pEnd)
	{
		if (*pPos == '\n' || *pPos == '\r')
			return true;
		++pPos;
	}

	return false;
}

void ArchiveTextV1::operator<<(const QString& s)
{
	// Since this is a text format and it generally
	// uses one line per 'thing', a string can get complicated
	// if it has any embedded '\r' or '\n' characters. To make this simple,
	// we will prepend the number of characters. There is a problem with
	// a string that spans a line, we use readLine() to get each line
	// and doing so prunes whitespace. To deal with that problem, strings
	// that have CRLFs will always be written AFTER the first line.

	int iLen = s.length();
	QString sPrepended = QString("%1:").arg(iLen);
	if (!IsMultiLine(s))
	{
		// It all fits on one line
		sPrepended += s;
		Write(sPrepended);
		return;
	}

	// Multi-line 
	Write(sPrepended);
	*m_pTextStream << s << CRLF;	// Always add a newline to make it more readable
}


void ArchiveTextV1::operator>>(QString &s)
{
	// Read the entire line (there might be more that we need to read)
	QString sLine = ReadNoTrim();  // We want this completely non-lossy, so don't trim

	// Get the count from the line
	int iPos = sLine.indexOf(':');
	Q_ASSERT(iPos != -1);
	QString sCount = sLine.left(iPos);
	bool bOk;
	int iStrLen = sCount.toInt(&bOk);
	if (!bOk)
		EXERR("93RT", "Invalid count for string");

	// Keep the right side of the line
	s = sLine.right(sLine.count() - iPos - 1);

	// How much more is there to read?
	int iRemaining = iStrLen - s.count();
	Q_ASSERT(iRemaining >= 0);
	if (0 == iRemaining)
		return;

	// Read the rest
	QString sMore = m_pTextStream->read(iRemaining);

	// Count the lines in there
	const QChar *pPos = sMore.constData();
	const QChar *pEnd = pPos + iRemaining;
	int iLines = 1;
	bool bInCRLF = false;
	while (pPos < pEnd)
	{
		bool bCRLF = (*pPos == '\r' || *pPos == '\n');
		if (bCRLF == bInCRLF)
		{
			// No change
			++pPos;
			continue;
		}

		if (bCRLF)
			++iLines;
		bInCRLF = bCRLF;
		++pPos;
	}
	m_iLineNum += iLines;

	// Append to the end
	s += sMore;

	// We always added a newline. 
	QString sLastCRLF = _ReadLine();
}




void ArchiveTextV1::operator<<(const QByteArray& ba)
{
	QByteArray baBase64 = ba.toBase64();
	Write(baBase64);
}

void ArchiveTextV1::operator>>(QByteArray &ba)
{
	QString sBase64 = Read();
	QByteArray baBase64 = sBase64.toUtf8();
	ba = QByteArray::fromBase64(baBase64);
}



void ArchiveTextV1::operator<<(const QVariant& var)
{
	WriteAsByteArray(var);
}

void ArchiveTextV1::operator>>(QVariant &var)
{
	ReadAsByteArray(var);
}




void ArchiveTextV1::operator<<(const QDateTime& thing)
{
	if(thing.isValid())
		WriteAsByteArray(thing, thing.toString(Qt::ISODate));
	else {
		// When invalid, we always use a 'stock' invalid time. 
		WriteAsByteArray(QDateTime(), "invalid");
	}
}

void ArchiveTextV1::operator>>(QDateTime &thing)
{
	ReadAsByteArray(thing);
}


void ArchiveTextV1::operator<<(const QDate& thing)
{
	if(thing.isValid())
		WriteAsByteArray(thing, thing.toString(Qt::ISODate));
	else
		WriteAsByteArray(thing, "invalid");
}

void ArchiveTextV1::operator>>(QDate &thing)
{
	ReadAsByteArray(thing);
}


void ArchiveTextV1::operator<<(const QTime& thing)
{
	if (thing.isValid())
		WriteAsByteArray(thing, thing.toString(Qt::ISODate));
	else
		WriteAsByteArray(thing, "invalid");
}

void ArchiveTextV1::operator>>(QTime &thing)
{
	ReadAsByteArray(thing);
}


void ArchiveTextV1::operator<<(const QTimeZone& thing)
{
	QString sDisplay = thing.id();
	WriteAsByteArray(thing, sDisplay);
}

void ArchiveTextV1::operator>>(QTimeZone &thing)
{
	ReadAsByteArray(thing);
}



void ArchiveTextV1::operator<<(const QUuid& thing)
{
	Write(thing.toString());
}

void ArchiveTextV1::operator>>(QUuid &thing)
{
	QString s = Read();
	thing = QUuid(s);
}


void ArchiveTextV1::operator<<(const QPoint& thing)
{
	QString s = QString("%1,%2").arg(thing.x()).arg(thing.y());
	Write(s);
}

void ArchiveTextV1::operator>>(QPoint &thing)
{
	QString s = Read();
	QStringList sl = s.split(',');
	if (2 != sl.count())
		EXERR("MXK3", "Invalid QPoint data %s", qPrintable(s));

	// Convert to integers
	bool bOkX, bOkY;
	int x = sl.at(0).toInt(&bOkX);
	int y = sl.at(1).toInt(&bOkY);
	if(!bOkX || !bOkY)
		EXERR("MXK4", "Invalid QPoint data %s", qPrintable(s));

	thing = QPoint(x, y);
}







void ArchiveTextV1::operator<<(const QPointF& thing)
{
	QString sDisplay = QString("%1, %2").arg(thing.x()).arg(thing.y());

	// To avoid any loss, we use a hex representation
	// of the data in addition to the display string
	float xy[2];
	xy[0] = thing.x();
	xy[1] = thing.y();
	QByteArray ba;
	ba.reserve(sizeof(xy));
	ba.append((const char*)&xy[0], sizeof(xy));

	QString sHex = ba.toHex();
	Write(QString("%1 %2").arg(sHex).arg(sDisplay));
}

void ArchiveTextV1::operator>>(QPointF &thing)
{
	QString s = Read();
	QStringList sl = s.split(' ');
	if (3 != sl.count())
		EXERR("MAK3", "Invalid QPointF data %s", qPrintable(s));

	QString sHex = sl.first();
	QByteArray baHex = QByteArray::fromHex(sHex.toLatin1());
	float* pX = (float*)baHex.constData();
	float* pY = pX + 1;

	thing = QPointF(*pX, *pY);
}




void ArchiveTextV1::operator<<(const QImage& thing)
{
	WriteAsByteArray(thing);
}

void ArchiveTextV1::operator>>(QImage &thing)
{
	ReadAsByteArray(thing);
}





void ArchiveTextV1::operator<<(const QColor& thing)
{
	Write(QString("clr(%1,%2,%3,%4)").arg(thing.red()).arg(thing.green()).arg(thing.blue()).arg(thing.alpha()));
}

void ArchiveTextV1::operator>>(QColor &thing)
{
	static QRegularExpression regex("clr\\((\\d+),(\\d+),(\\d+),(\\d+)\\)");
	QString s = Read();
	QRegularExpressionMatch m = regex.match(s);
	int iCount = regex.captureCount();
	if(iCount != 4)
		EXERR("MAK4", "Invalid QColor data %s", qPrintable(s));
	QVector<int> vectInts;
	QStringList sl = m.capturedTexts();
	sl.removeFirst();
	for (QString s1 : sl)
	{
		bool bOk;
		vectInts += s1.toInt(&bOk);
		if (!bOk)
			EXERR("MAKD", "Invalid QColor data %s", qPrintable(s));
	}
	thing = QColor(
		vectInts.at(0),
		vectInts.at(1),
		vectInts.at(2),
		vectInts.at(3));
}




void ArchiveTextV1::operator<<(const QTransform& thing)
{
	QString sRow1 = QString("%1,%2,%3").arg(thing.m11()).arg(thing.m12()).arg(thing.m13());
	QString sRow2 = QString("%1,%2,%3").arg(thing.m21()).arg(thing.m22()).arg(thing.m23());
	QString sRow3 = QString("%1,%2,%3").arg(thing.m31()).arg(thing.m32()).arg(thing.m33());
	QString sDisplay = QString("[%1; %2; %3]").arg(sRow1, sRow2, sRow3);
	
	float coeffs[9];
	coeffs[0] = thing.m11();
	coeffs[1] = thing.m12();
	coeffs[2] = thing.m13();
	coeffs[3] = thing.m21();
	coeffs[4] = thing.m22();
	coeffs[5] = thing.m23();
	coeffs[6] = thing.m31();
	coeffs[7] = thing.m32();
	coeffs[8] = thing.m33();

	QByteArray ba;
	ba.reserve(sizeof(coeffs));
	ba.append((const char*)&coeffs[0], sizeof(coeffs));

	QString sHex = ba.toHex();
	Write(QString("%1 %2").arg(sHex).arg(sDisplay));
}

void ArchiveTextV1::operator>>(QTransform &thing)
{
	QString s = Read();
	QStringList sl = s.split(' ');
	if (4 != sl.count())
		EXERR("MAK3", "Invalid QTransform data %s", qPrintable(s));

	QString sHex = sl.first();
	QByteArray baHex = QByteArray::fromHex(sHex.toLatin1());
	float* pF = (float*)baHex.constData();

	// Move data to the transform
	thing = QTransform(
				pF[0],
				pF[1],
				pF[2],
				pF[3],
				pF[4],
				pF[5],
				pF[6],
				pF[7],
				pF[8]
			);
}


void ArchiveTextV1::operator<<(const QRect& thing)
{
	Write(QString(" %1,%2, %3,%4").arg(thing.x()).arg(thing.y()).arg(thing.width()).arg(thing.height()));
}

void ArchiveTextV1::operator>>(QRect &thing)
{
	QString s = Read(); 
	QStringList sl = s.split(',');
	if (4 != sl.count())
		EXERR("M3KD", "Invalid QRect data %s", qPrintable(s));
	QVector<int> vect;
	for (QString sOne : sl)
	{
		bool bOk;
		int iVal = sOne.toInt(&bOk);
		if (!bOk)
			EXERR("DDKD", "Invalid QRect data %s", qPrintable(s));
		vect += iVal;
	}
	thing.setX(vect.at(0));
	thing.setY(vect.at(1));
	thing.setWidth(vect.at(2));
	thing.setHeight(vect.at(3));
}


void ArchiveTextV1::operator<<(const QRectF& thing)
{
	QString sDisplay = QString(" %1,%2, %3,%4").arg(thing.x()).arg(thing.y()).arg(thing.width()).arg(thing.height());
	
	// To avoid any loss, we use a hex representation
	// of the data in addition to the display string
	float xywh[4];
	xywh[0] = thing.x();
	xywh[1] = thing.y();
	xywh[2] = thing.width(); 
	xywh[3] = thing.height();
	QByteArray ba;
	ba.reserve(sizeof(xywh));
	ba.append((const char*)&xywh[0], sizeof(xywh));

	QString sHex = ba.toHex();
	Write(QString("%1 %2").arg(sHex).arg(sDisplay));
}

void ArchiveTextV1::operator>>(QRectF &thing)
{
	QString s = Read();
	QStringList sl = s.split(' ');
	if (sl.count() < 2)
		EXERR("M3KD", "Invalid QRectF data %s", qPrintable(s));

	QString sHex = sl.first();
	QByteArray baHex = QByteArray::fromHex(sHex.toLatin1());
	float* pF = (float*)baHex.constData();
	thing.setX(*pF++);
	thing.setY(*pF++);
	thing.setWidth(*pF++);
	thing.setHeight(*pF++);
}
