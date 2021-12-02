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

private slots:
	void on_slider_valueChanged(int value);
	void on_spinBox_valueChanged(int value);

private:
	Ui::ParamWidgetInt ui;
	bool m_bChanging = false;
	QVariant m_vCookie;
};
