#include "ParamWidgetEnum.h"

ParamWidgetEnum::ParamWidgetEnum(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}



ParamWidgetEnum::ParamWidgetEnum(
		const QString& sName, 
		const PipelineStepParam& psp, 
		const QVariant& vCookie,
		QWidget* parent)
	: QWidget(parent)
{
	Q_ASSERT(QVariant::StringList == psp.Type());

	ui.setupUi(this);

	m_vCookie = vCookie;
	ui.label->setText(sName);
	ui.cbEnum->addItems(psp.EnumNames());
	m_listEnumValues = psp.EnumValues();

	int iCurrentValue = psp.Value().toInt();
	int iCurrentIndex = m_listEnumValues.indexOf(iCurrentValue);
	ui.cbEnum->setCurrentIndex(iCurrentIndex);

	m_bInitializing = false;
}


void ParamWidgetEnum::on_cbEnum_currentIndexChanged(int iIndex)
{
	if (m_bInitializing)
		return;

	int iEnumValue = m_listEnumValues.at(iIndex);
	emit ParamChanged(m_vCookie, QVariant(iEnumValue));
}
