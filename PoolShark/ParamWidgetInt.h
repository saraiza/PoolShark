#pragma once

#include <QWidget>
#include "ui_ParamWidgetInt.h"
#include "Pipeline.h"

class ParamWidgetInt : public QWidget
{
	Q_OBJECT

public:
	ParamWidgetInt(QWidget* parent = Q_NULLPTR);
	ParamWidgetInt(const QString& sName, const PipelineStepParam& psp, const QVariant& vCookie, QWidget* parent = Q_NULLPTR);

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
