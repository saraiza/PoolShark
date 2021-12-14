#include "stdafx.h"
#include "ParamWidgetFloat.h"
#include <Util.h>


#define SLIDER_DIVISIONS			10000

ParamWidgetFloat::ParamWidgetFloat(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.label->setText("");
}

ParamWidgetFloat::ParamWidgetFloat(
		const QString& sName, 
		const PipelineStepParam& psp, 
		const QVariant& vCookie, 
		QWidget* parent)
	: QWidget(parent)
{
	Q_ASSERT(QVariant::Double == psp.Type());
	ui.setupUi(this);

	double dValue = psp.Value().toDouble();
	double dMin = psp.MinValue().toDouble();
	double dMax = psp.MaxValue().toDouble();

	m_vCookie = vCookie;
	ui.label->setText(sName);
	ui.dsb->setRange(dMin, dMax);
	ui.dsb->setValue(dValue);

	// Divide the range up for 32 bit
	m_tx.dMin = dMin;
	m_tx.dMax = dMax;

	ui.slider->setRange(0, SLIDER_DIVISIONS);
	ui.slider->setValue(ValueToSlider(dValue));
}


double ParamWidgetFloat::SliderToValue(int iSlider)
{
	double dFactor = (double)iSlider / SLIDER_DIVISIONS;
	return m_tx.dMin + dFactor * (m_tx.dMax - m_tx.dMin);
}

int ParamWidgetFloat::ValueToSlider(double dValue)
{
	double dFactor = (dValue - m_tx.dMin) / (m_tx.dMax - m_tx.dMin);
	int iSlider = dFactor * SLIDER_DIVISIONS;
	return iSlider;
}

void ParamWidgetFloat::on_slider_valueChanged(int value)
{
	if (m_bChanging)
		return;
	BoolSet bs(&m_bChanging, true);

	double dNewValue = SliderToValue(value);
	ui.dsb->setValue(dNewValue);
	emit ParamChanged(m_vCookie, QVariant(dNewValue));
}


void ParamWidgetFloat::on_dsb_valueChanged(double value)
{
	if (m_bChanging)
		BoolSet bs(&m_bChanging, true);

	// Enforce limits
	value = qMax(value, m_tx.dMin);
	value = qMin(value, m_tx.dMax);

	int iSlider = ValueToSlider(value);
	emit ParamChanged(m_vCookie, QVariant(value));
	ui.slider->setValue(iSlider);
}
