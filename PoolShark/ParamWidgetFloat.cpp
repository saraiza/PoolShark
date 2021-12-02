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


void ParamWidgetFloat::Init(const QString& sName, const PipelineStepParam& psp, QVariant vCookie)
{
	m_vCookie = vCookie;
	ui.label->setText(sName);
	ui.dsb->setRange(psp.MinValue().toFloat(), psp.MaxValue().toFloat());
	ui.dsb->setValue(psp.Value().toFloat());

	float fMin = psp.MinValue().toFloat();
	float fMax = psp.MaxValue().toFloat();
	float fRange = fMax - fMin;

	// Divide the range up for 32 bit
	m_tx.fMin = fMin;
	m_tx.fMax = fMax;
	ui.slider->setRange(0, SLIDER_DIVISIONS);
	ui.slider->setValue(ValueToSlider(psp.Value().toFloat()));
}

float ParamWidgetFloat::SliderToValue(int iSlider)
{
	double dFactor = (double)iSlider / SLIDER_DIVISIONS;
	return m_tx.fMin + dFactor * (m_tx.fMax - m_tx.fMin);
}

int ParamWidgetFloat::ValueToSlider(float fValue)
{
	double dFactor = (fValue - m_tx.fMin) / (m_tx.fMax - m_tx.fMin);
	int iSlider = dFactor * SLIDER_DIVISIONS;
	return iSlider;
}

void ParamWidgetFloat::on_slider_valueChanged(int value)
{
	if (m_bChanging)
		return;
	BoolSet bs(&m_bChanging, true);

	float fNewValue = SliderToValue(value);
	ui.dsb->setValue(fNewValue);
	emit ParamChanged(m_vCookie, QVariant(fNewValue));
}


void ParamWidgetFloat::on_dsb_valueChanged(double value)
{
	if (m_bChanging)
		BoolSet bs(&m_bChanging, true);

	int iSlider = ValueToSlider(value);
	float fNewValue = (float)value;
	emit ParamChanged(m_vCookie, QVariant(fNewValue));
	ui.slider->setValue(iSlider);
}
