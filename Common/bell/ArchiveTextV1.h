#pragma once
#include "Archive.h"
#include <QDataStream>
#include <QImage>

class QTextStream;


/**
@brief ArchiveText writes as text

Part of the SerMig system. This is not meant to be human editable,
merely 'almost readable'.
*/
class ArchiveTextV1 : public Archive
{
public:
	ArchiveTextV1(QIODevice *pStream, SerMig::Options options = SerMig::OPT_None);
	virtual ~ArchiveTextV1() override;	

	virtual bool Start() override;

	virtual QDataStream::FloatingPointPrecision floatingPointPrecision() const override;
	virtual void setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec) override;


	virtual Archive& label(const QString& sLabel) override;
	virtual void Tag(const QString& sTag) override;
	virtual void Indent() override;
	virtual void Unindent() override;

	virtual void WriteRawData(const void *pData, int iLen) override;
	virtual int ReadRawData(void *pData, int iLen) override;
	
	virtual bool DebugTag() override;

	virtual bool atEnd() const override;

	// Low-level native types
	virtual void operator>>(qint8 &i) override;
	virtual void operator<<(qint8 i) override;
	virtual void operator>>(quint8 &i) override;
	virtual void operator<<(quint8 i) override;
	virtual void operator>>(qint16 &i) override;
	virtual void operator<<(qint16 i) override;
	virtual void operator>>(quint16 &i) override;
	virtual void operator<<(quint16 i) override;
	virtual void operator>>(qint32 &i) override;
	virtual void operator<<(qint32 i) override;
	virtual void operator>>(quint32 &i) override;
	virtual void operator<<(quint32 i) override;
	virtual void operator>>(qint64 &i) override;
	virtual void operator<<(qint64 i) override;
	virtual void operator>>(quint64 &i) override;
	virtual void operator<<(quint64 i) override;
	virtual void operator>>(bool &i) override;
	virtual void operator<<(bool i) override;
	virtual void operator>>(float &f) override;
	virtual void operator<<(float f) override;
	virtual void operator>>(double &f) override;
	virtual void operator<<(double f) override;

	// Assorted Qt classes and other common types
	virtual void operator<<(const QChar& thing) override;
	virtual void operator>>(QChar &thing) override;
	virtual void operator>>(QString &s) override;
	virtual void operator<<(const QString& s) override;
	virtual void operator>>(QByteArray &ba) override;
	virtual void operator<<(const QByteArray& ba) override;
	virtual void operator>>(QVariant &var) override;
	virtual void operator<<(const QVariant& var) override;
	virtual void operator>>(QDateTime &s) override;
	virtual void operator<<(const QDateTime& s) override;
	virtual void operator>>(QDate &s) override;
	virtual void operator<<(const QDate& s) override;
	virtual void operator>>(QTime &s) override;
	virtual void operator<<(const QTime& s) override;
	virtual void operator>>(QTimeZone &thing) override;
	virtual void operator<<(const QTimeZone& thing) override;
	virtual void operator<<(const QUuid& thing) override;
	virtual void operator>>(QUuid &thing) override;
	virtual void operator<<(const QPoint& thing) override;
	virtual void operator>>(QPoint &thing) override;
	virtual void operator<<(const QPointF& thing) override;
	virtual void operator>>(QPointF &thing) override;
	virtual void operator<<(const QImage& thing) override;
	virtual void operator>>(QImage &thing) override;
	virtual void operator<<(const QColor& thing) override;
	virtual void operator>>(QColor &thing) override;
	virtual void operator<<(const QTransform& thing) override;
	virtual void operator>>(QTransform &thing) override;
	virtual void operator<<(const QRect& thing) override;
	virtual void operator>>(QRect &thing) override;
	virtual void operator<<(const QRectF& thing) override;
	virtual void operator>>(QRectF &thing) override;

private:
	// Use these for internal R/W. They will handle 
	QString _ReadLine();
	QString Read();
	QString ReadNoTrim();
	void Write(const QString& s);
	bool IsMultiLine(const QString& s) const;

	/// Indent state tracking
	struct {
		int i = 0;
		QString s;
	} m_indent;
	QString m_sLabel;	///< The label to use on the next bit of data (read or write)

	QIODevice *m_pStream = nullptr;	///< Cached until Start()
	QTextStream* m_pTextStream = nullptr;	///< We are ultimately just forwarding to this

	int m_iLineNum = 0;	///< While reading/writing, we track this just for debugging.


	// There are a number of cases where we just want to
	// go to a QByteArray to make it lossless. These 
	// utility methods will help you do that for assorted types.
	// You can provide an optional display string (like for date/time types, etc)
	template<class T>
	void WriteAsByteArray(const T& obj, const QString& sDisplay = QString())
	{
		// Convert objects to bytes
		QByteArray ba;
		{
			QDataStream ds(&ba, QIODevice::WriteOnly);
			ds << obj;
		}

		QString sBa64 = ba.toBase64();
		if (sDisplay.isEmpty())
			Write(sBa64);
		else
			Write(QString("%1 %2").arg(sBa64, sDisplay));
	}

	template<class T>
	void ReadAsByteArray(T& obj)
	{
		QString sBase64 = Read();
		int iPos = sBase64.indexOf(' ');
		if (-1 != iPos)
			sBase64 = sBase64.left(iPos); // Prune display text
		
		QByteArray baBase64 = sBase64.toUtf8();
		QByteArray ba = QByteArray::fromBase64(baBase64);

		QDataStream ds(&ba, QIODevice::ReadOnly);
		ds >> obj;
	}
};

