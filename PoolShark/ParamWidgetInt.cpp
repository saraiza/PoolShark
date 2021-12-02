#include "ParamWidgetInt.h"

ParamWidgetInt::ParamWidgetInt(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}


void ParamWidgetInt::Init(const QString& sName, const PipelineStepParam& psp, QVariant vCookie)
{
	m_vCookie = vCookie;
	ui.label->setText(sName);
	ui.spinBox->setRange(psp.MinValue().toInt(), psp.MaxValue().toInt());
	ui.spinBox->setValue(psp.Value().toInt());
	ui.slider->setRange(psp.MinValue().toInt(), psp.MaxValue().toInt());
	ui.slider->setValue(psp.Value().toInt());
}
