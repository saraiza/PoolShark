#pragma once

#include <QWidget>
#include "ui_ParamWidgetFloat.h"
#include "Pipeline.h"

class ParamWidgetFloat : public QWidget
{
	Q_OBJECT

public:
	ParamWidgetFloat(QWidget *parent = Q_NULLPTR);

	void Init(const QString& sName, const PipelineStepParam& psp, QVariant vCookie);

signals:
	void ParamChanged(QVariant vCookie, QVariant vNewValue);

private slots:
	void on_slider_valueChanged(int value);

private:
	Ui::ParamWidgetFloat ui;
	QVariant m_vCookie;
};
