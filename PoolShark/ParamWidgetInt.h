#pragma once

#include <QWidget>
#include "ui_ParamWidgetInt.h"
#include "Pipeline.h"

class ParamWidgetInt : public QWidget
{
	Q_OBJECT

public:
	ParamWidgetInt(QWidget *parent = Q_NULLPTR);
	void Init(const QString& sName, const PipelineStepParam& psp, QVariant vCookie);

signals:
	void ParamChanged(QVariant vCookie, QVariant vNewValue);

private:
	Ui::ParamWidgetInt ui;
	QVariant m_vCookie;
};
