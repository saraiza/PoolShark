#include "PipelineFactory.h"


PipelineFactory PipelineFactory::ms_instance;

void PipelineFactory::Define(const QString& sName, QList<PipelineStepParam> listParams, PipelineStep::FuncOp funcOp)
{
	PipelineStep step(sName, listParams, funcOp);
	ms_instance.m_mapTemplates[sName] = step;
}


PipelineStep PipelineFactory::CreateStep(const QString& sName)
{
	Q_ASSERT(ms_instance.m_mapTemplates.contains(sName));
	return ms_instance.m_mapTemplates.value(sName);
}

PipelineFactory::PipelineFactory()
{
	Init();
}

void PipelineFactory::Init()
{
	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("age", QVariant(1));
		listParams += PipelineStepParam("dogs", QVariant(2));
		listParams += PipelineStepParam("spice", QVariant(3.1f));
		Define("Doggy Spice", listParams, [](const cv::Mat& img, const QList<PipelineStepParam>& listParams) {
			return img;
			});
	}

	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("feet", QVariant(10));
		listParams += PipelineStepParam("cats", QVariant(20));
		listParams += PipelineStepParam("spice", QVariant(30.1f));
		Define("Doggy Spice", listParams, [](const cv::Mat& img, const QList<PipelineStepParam>& listParams) {
			return img;
			});
	}


}

