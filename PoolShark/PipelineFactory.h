#pragma once
#include "Pipeline.h"
#include <functional>

/**
@brief OpenCV sequence of processing steps

*/
class PipelineFactory
{
public:
	void Init();
	static PipelineStep CreateStep(const QString& sName);

private:
	PipelineFactory();
	static void Define(const QString& sName, QList<PipelineStepParam> listParams, PipelineStep::FuncOp funcOp);
	
	QMap<QString, PipelineStep> m_mapTemplates;
	static PipelineFactory ms_instance;
};

