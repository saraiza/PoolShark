#include "stdafx.h"
#include "PipelineFactory.h"
#include <opencv2/core/core.hpp>           // cv::Mat
#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>     // cv::Canny()
#include <opencv2/highgui.hpp>


using namespace cv;

PipelineFactory PipelineFactory::ms_instance;



DECLARE_LOG_SRC("PipelineFactory", LOGCAT_Common);


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


QStringList PipelineFactory::StepNames()
{
	return ms_instance.m_mapTemplates.keys();
}

PipelineFactory::PipelineFactory()
{
	Init();
}

void PipelineFactory::Init()
{
	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("age", 1, 0, 10);
		listParams += PipelineStepParam("dogs", 2,0,20);
		listParams += PipelineStepParam("dog spice", 3.1f, 0.0f, 15.0f);
		Define("Doggy Spice", listParams, [](const cv::Mat& img, const QList<PipelineStepParam>& listParams) {
			return img;
			});
	}

	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("Kernel", 5, 1, 100);
		Define("GaussianBlur", listParams, [](const cv::Mat& img, const QList<PipelineStepParam>& listParams) {
			cv::Mat imgOut;
			int iKernel = listParams.at(0).Value().toInt();

			// The kernel must be odd or zero
			if (iKernel > 0 && iKernel % 2 == 0)
				--iKernel;
			else if (iKernel < 0)
				iKernel = 0;

			cv::GaussianBlur(img, imgOut, Size(iKernel, iKernel), 0.0); 
			return imgOut;
			});
	}

	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("feet", 10, 0, 15);
		listParams += PipelineStepParam("cats", 20, 15,25);
		listParams += PipelineStepParam("cat spice", 30.1f, 5.0f, 20.0f);
		Define("Cat Feet", listParams, [](const cv::Mat& img, const QList<PipelineStepParam>& listParams) {
			return img;
			});
	}


}

