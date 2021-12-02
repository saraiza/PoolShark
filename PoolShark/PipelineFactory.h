#pragma once
#include "Pipeline.h"
#include <functional>

/**
@brief Generate a step object by name

*/
class PipelineFactory
{
public:
	void Init();
	static PipelineStep CreateStep(const QString& sName);
	static QStringList StepNames();

private:
	PipelineFactory();
	static void Define(const QString& sName, QList<PipelineStepParam> listParams, PipelineStep::FuncOp funcOp);
	
	QMap<QString, PipelineStep> m_mapTemplates;
	static PipelineFactory ms_instance;
};

