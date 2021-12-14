#include "stdafx.h"
#include "PipelineFactory.h"
#include <Exception.h>
#include <Logging.h>
#include <opencv2/core/core.hpp>           // cv::Mat
#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>     // cv::Canny()
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/gpu/gpu.hpp>

using namespace std;

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
		listParams += PipelineStepParam("Kernel", 5, 1, 100);
		Define("GaussianBlur", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			PipelineData out;
			int iKernel = listParams.at(0).Value().toInt();

			// The kernel must be odd or zero
			if (iKernel > 0 && iKernel % 2 == 0)
				--iKernel;
			else if (iKernel < 0)
				iKernel = 0;

			cv::GaussianBlur(input.img, out.img, cv::Size(iKernel, iKernel), 0.0);
			return out;
			});
	}

	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("Thresh1", 100.0, 0.0, 255.0);
		listParams += PipelineStepParam("Thresh2", 175.0, 0.0, 255.0);
		listParams += PipelineStepParam("Aperture", 3, 3, 11);
		Define("Canny", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			double dThresh1 = listParams.at(0).Value().toDouble();
			double dThresh2 = listParams.at(1).Value().toDouble();
			int iApertureSize = listParams.at(2).Value().toInt();
			if (0 == iApertureSize % 2)
			{
				LOGINFO("Even aperture size for Canny filter must be odd, bumping up one");
				++iApertureSize;
			}
			if (dThresh1 > dThresh2)
			{
				LOGINFO("Canny dThresh1 > dThresh2, clamping");
				dThresh1 = dThresh2;
			}

			PipelineData out;
			cv::Canny(input.img, out.img, dThresh1, dThresh2, iApertureSize);
			return out;
			});
	}


	{
		QList<PipelineStepParam> listParams;
		listParams += PipelineStepParam("Mode", QStringList() << "RETR_EXTERNAL=1" << "RETR_LIST=1" << "RETR_CCOMP=2" << "RETR_TREE=3" /* << "RETR_FLOODFILL=4" */);
		listParams += PipelineStepParam("Method", QStringList() << "CHAIN_APPROX_NONE=1" << "CHAIN_APPROX_SIMPLE=2" << "CHAIN_APPROX_TC89_L1=3" << "CHAIN_APPROX_TC89_KCOS=4");
		Define("findContours", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			// The input image must be an 8 bit grayscale image
			if (input.img.elemSize() != 1)
				EXERR("RRTK", "findContours requires a grayscale input. Try using Canny() edge detection first.");

			int iMode = listParams.at(0).Value().toInt();
			int iMethod = listParams.at(1).Value().toInt();
			PipelineData out;
			cv::findContours(input.img, out.contours, iMode, iMethod);

			// Draw the contours onto a blank image of the same size
			out.img = cv::UMat::zeros(input.img.rows, input.img.cols, CV_8UC3);
			cv::Scalar color(0, 0, 255);	// red
			cv::drawContours(out.img, out.contours, -1, color);			
			return out;
			});
	}
}

