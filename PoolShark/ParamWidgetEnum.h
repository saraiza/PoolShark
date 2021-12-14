#pragma once

#include <QWidget>
#include "ui_ParamWidgetEnum.h"
#include "Pipeline.h"

class ParamWidgetEnum : public QWidget
{
	Q_OBJECT

public:
	ParamWidgetEnum(QWidget* parent = Q_NULLPTR);
	ParamWidgetEnum(const QString& sName, const PipelineStepParam& psp, const QVariant& vCookie, QWidget *parent = Q_NULLPTR);

signals:
	void ParamChanged(QVariant vCookie, QVariant vNewValue);

private slots:
	void on_cbEnum_currentIndexChanged(int iIndex);

private:
	Ui::ParamWidgetEnum ui;
	bool m_bInitializing = true;
	QVariant m_vCookie;
	QList<int> m_listEnumValues;	///< Convert the indexes into absolute enum integer values
};
