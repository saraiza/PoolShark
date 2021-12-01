#pragma once
#include "Archive.h"
#include <QDataStream>


/**
@brief Automatically switches between Read and Write downstream archives

There are always two downstream archives. 
*/
class ArchiveRouterAuto : public Archive
{
public:
	ArchiveRouterAuto();
	virtual ~ArchiveRouterAuto() override {}

	void SetInner(Archive* pArR, Archive* pArW);

	// These are forwarded to all downstream Archives
	virtual bool Start() override;
	virtual QDataStream::FloatingPointPrecision floatingPointPrecision() const override;
	virtual void setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec) override;

	// These are routed to the active archive

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
	Archive* m_pArR = nullptr;
	Archive* m_pArW = nullptr;
};

