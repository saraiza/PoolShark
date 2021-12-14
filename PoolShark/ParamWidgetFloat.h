#pragma once

#include <QWidget>
#include "ui_ParamWidgetFloat.h"
#include "Pipeline.h"

class ParamWidgetFloat : public QWidget
{
	Q_OBJECT

public:
	ParamWidgetFloat(QWidget* parent = Q_NULLPTR);
	ParamWidgetFloat(const QString& sName, const PipelineStepParam& psp, const QVariant& vCookie, QWidget* parent = Q_NULLPTR);

signals:
	void ParamChanged(QVariant vCookie, QVariant vNewValue);

private slots:
	void on_slider_valueChanged(int value);
	void on_dsb_valueChanged(double value);

private:
	Ui::ParamWidgetFloat ui;
	QVariant m_vCookie; 
	bool m_bChanging = false;
	
	struct {
		double dMin = 0.0f;
		double dMax = 0.0f;
	} m_tx;
	double SliderToValue(int iSlider);
	int ValueToSlider(double dValue);
};
