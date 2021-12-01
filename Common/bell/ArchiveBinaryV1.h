#pragma once
#include "Archive.h"
#include <QDataStream>


/**
@brief ArchiveBinaryV1 writes as raw binary data

Part of the SerMig system. The class is versioned
in case we want or need to change the format.
*/
class ArchiveBinaryV1 : public Archive
{
public:
	ArchiveBinaryV1(QIODevice *pStream, SerMig::Options options = SerMig::OPT_None);
	virtual ~ArchiveBinaryV1() override {}


	virtual bool Start() override;

	virtual QDataStream::FloatingPointPrecision floatingPointPrecision() const override;
	virtual void setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec) override;

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
	QDataStream m_dataStream;	///< We are ultimately just forwarding to this
};

