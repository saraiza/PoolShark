#include "stdafx.h"
#include "ParamWidgetInt.h"
#include <Util.h>


ParamWidgetInt::ParamWidgetInt(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.label->setText("");
}

ParamWidgetInt::ParamWidgetInt(
		const QString& sName, 
		const PipelineStepParam& psp, 
		const QVariant& vCookie, 
		QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_vCookie = vCookie;
	ui.label->setText(sName);
	ui.spinBox->setRange(psp.MinValue().toInt(), psp.MaxValue().toInt());
	ui.spinBox->setValue(psp.Value().toInt());
	ui.slider->setRange(psp.MinValue().toInt(), psp.MaxValue().toInt());
	ui.slider->setValue(psp.Value().toInt());
}




void ParamWidgetInt::on_slider_valueChanged(int value)
{
	if (m_bChanging)
		return;
	BoolSet bs(&m_bChanging, true);

	ui.spinBox->setValue(value);
	emit ParamChanged(m_vCookie, value);
}

void ParamWidgetInt::on_spinBox_valueChanged(int value)
{
	if (m_bChanging)
		return;
	BoolSet bs(&m_bChanging, true);

	ui.slider->setValue(value);
	emit ParamChanged(m_vCookie, value);
}
