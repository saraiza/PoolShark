#include "ParamWidgetFloat.h"


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
	ui.slider->setRange(psp.MinValue().toFloat(), psp.MaxValue().toFloat());
	ui.slider->setValue(psp.Value().toFloat());
}



void ParamWidgetFloat::on_slider_valueChanged(int value)
{
	float fNewValue = (float)value;
	emit ParamChanged(m_vCookie, QVariant(fNewValue));
}

