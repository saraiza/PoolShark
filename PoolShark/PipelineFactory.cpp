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
		listParams += PipelineStepParam("Kernel", 5, 1, 500);
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
		listParams += PipelineStepParam("ksize", 1, 1, 11);
		listParams += PipelineStepParam("scale", 1.0, 0.0, 5.0);
		listParams += PipelineStepParam("delta", 0.0, 0.0, 255.0);
		listParams += PipelineStepParam("borderType", QStringList() << "BORDER_CONSTANT=0" << "BORDER_REPLICATE=1" << "BORDER_REFLECT=2" << "BORDER_WRAP=3" << "BORDER_REFLECT_101 (Default) = 4" /*"BORDER_TRANSPARENT = 5"*/ <<  "BORDER_ISOLATED=16");
		Define("Laplacian", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			// The input image must be an 8 bit grayscale image
			//if (input.img.elemSize() != 1)
			//	EXERR("RRT1", "Laplacian requires a grayscale input. Try using Canny() edge detection first.");

			int ksize = listParams.at(0).Value().toInt();
			double scale = listParams.at(1).Value().toDouble();
			double delta = listParams.at(2).Value().toDouble();
			int borderType = listParams.at(3).Value().toInt();

			// The kernel must be positive and odd
			if (ksize % 2 == 0)
				--ksize;

			PipelineData out;
			cv::Laplacian(input.img, out.img, CV_16S, ksize, scale, delta, borderType);
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
				EXERR("RRT2", "findContours requires a grayscale input. Try using Canny() edge detection first.");

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


	{
		QList<PipelineStepParam> listParams;

		listParams += PipelineStepParam("rho", 1.0, 1.0, 95.0);
		listParams += PipelineStepParam("theta", CV_PI / 180, 0.001, 2 * CV_PI);
		listParams += PipelineStepParam("threshold", 150, 0, 255);
		listParams += PipelineStepParam("srn", 0.0, 0.0, 500.0);
		listParams += PipelineStepParam("stn", 0.0, 0.0, 500.0);
		//listParams += PipelineStepParam("min_theta", 0.0, 0.0, 500.0);
		//listParams += PipelineStepParam("max_theta", CV_PI, 0.01, CV_PI);
		Define("HoughLines", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			// The input image must be an 8 bit grayscale image
			if (input.img.elemSize() != 1)
				EXERR("RRT3", "HoughLines requires a grayscale input. Try using Canny() edge detection first.");

			int i = 0;	// Param index
			double 	rho = listParams.at(i++).Value().toDouble();
			double 	theta = listParams.at(i++).Value().toDouble();
			int 	threshold = listParams.at(i++).Value().toInt();
			double 	srn = listParams.at(i++).Value().toDouble();
			double 	stn = listParams.at(i++).Value().toDouble();
			//double 	min_theta = listParams.at(i++).Value().toDouble();
			//double 	max_theta = listParams.at(i++).Value().toDouble();			
			//if (min_theta > max_theta)
			//	min_theta = max_theta - 0.01;

			PipelineData out;
			vector<cv::Vec2f> vectLines;
			cv::HoughLines(input.img, vectLines, rho, theta, threshold, srn, stn /*, min_theta, max_theta*/);

			// Draw the lines onto a blank image of the same size
			out.img = cv::UMat::zeros(input.img.rows, input.img.cols, CV_8UC3);
			cv::Scalar color(0, 0, 255);	// red
			int iExtent = qMax(input.img.rows, input.img.cols) * qSqrt(2.0f);
			for (size_t i = 0; i < vectLines.size(); i++)
			{
				float rho = vectLines[i][0];
				float theta = vectLines[i][1];
				cv::Point pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a * rho, y0 = b * rho;
				pt1.x = cvRound(x0 + iExtent * (-b));
				pt1.y = cvRound(y0 + iExtent * (a));
				pt2.x = cvRound(x0 - iExtent * (-b));
				pt2.y = cvRound(y0 - iExtent * (a));
				line(out.img, pt1, pt2, color, 3, cv::LINE_AA);
			}

			return out;
			});
	}


	{
		QList<PipelineStepParam> listParams;

		listParams += PipelineStepParam("rho", 1.0, 1.0, 95.0);
		listParams += PipelineStepParam("theta", CV_PI / 180, 0.001, 2 * CV_PI);
		listParams += PipelineStepParam("threshold", 80, 0, 255);
		listParams += PipelineStepParam("srn", 30.0, 0.0, 500.0);
		listParams += PipelineStepParam("stn", 10.0, 0.0, 500.0);
		//listParams += PipelineStepParam("min_theta", 0.0, 0.0, 500.0);
		//listParams += PipelineStepParam("max_theta", CV_PI, 0.01, CV_PI);
		Define("HoughLinesP", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			// The input image must be an 8 bit grayscale image
			if (input.img.elemSize() != 1)
				EXERR("RRT3", "HoughLinesP requires a grayscale input. Try using Canny() edge detection first.");

			int i = 0;	// Param index
			double 	rho = listParams.at(i++).Value().toDouble();
			double 	theta = listParams.at(i++).Value().toDouble();
			int 	threshold = listParams.at(i++).Value().toInt();
			double 	srn = listParams.at(i++).Value().toDouble();
			double 	stn = listParams.at(i++).Value().toDouble();
			//double 	min_theta = listParams.at(i++).Value().toDouble();
			//double 	max_theta = listParams.at(i++).Value().toDouble();			
			//if (min_theta > max_theta)
			//	min_theta = max_theta - 0.01;

			PipelineData out;
			vector<cv::Vec4i> vectLines;
			cv::HoughLinesP(input.img, vectLines, rho, theta, threshold, srn, stn /*, min_theta, max_theta*/);

			// Draw the lines onto a blank image of the same size
			out.img = cv::UMat::zeros(input.img.rows, input.img.cols, CV_8UC3);
			cv::Scalar color(0, 0, 255);	// red
			for (size_t i = 0; i < vectLines.size(); i++)
			{
				float rho = vectLines[i][0];
				float theta = vectLines[i][1];
				cv::Point pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a * rho, y0 = b * rho;
				pt1.x = cvRound(x0 + 1000 * (-b));
				pt1.y = cvRound(y0 + 1000 * (a));
				pt2.x = cvRound(x0 - 1000 * (-b));
				pt2.y = cvRound(y0 - 1000 * (a));
				line(out.img, pt1, pt2, color, 3, cv::LINE_AA);
			}

			return out;
			});
	}


	 {
		QList<PipelineStepParam> listParams;

		listParams += PipelineStepParam("seed_x", 0.5, 0.0, 1.0);
		listParams += PipelineStepParam("seed_y", 0.5, 0.0, 1.0);
		listParams += PipelineStepParam("tolerance", 30, 0, 255);
		listParams += PipelineStepParam("connectivity", QStringList() << "4=4" << "8=8");
		listParams += PipelineStepParam("flags", QStringList() << QString("Fixed Range=%1").arg(cv::FLOODFILL_FIXED_RANGE) << QString("Mask Only=%1").arg(cv::FLOODFILL_MASK_ONLY));
		listParams += PipelineStepParam("mask", 1, 0, 255);
		Define("floodFill", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {

			int i = 0;	// Param index
			double 	seed_x = listParams.at(i++).Value().toDouble();
			double 	seed_y = listParams.at(i++).Value().toDouble();
			int iTol = listParams.at(i++).Value().toInt();
			int iConnectivity = listParams.at(i++).Value().toInt();
			int iFloodFillFlags = listParams.at(i++).Value().toInt();
			int iMask = listParams.at(i++).Value().toInt();

			PipelineData out = input;
			out.img = input.img.clone();
			
			cv::Rect rcBounds;
			cv::Scalar diff(iTol, iTol, iTol);
			int iFlags = iConnectivity | iFloodFillFlags | iMask << 8;
			int iRet = cv::floodFill(out.img,
				cv::Point(out.img.cols * seed_x, out.img.rows * seed_y),
				cv::Scalar(0, 0, 255),
				&rcBounds,
				diff, diff,
				iFlags);

			return out;
			});
	}

	/*
	{
		QList<PipelineStepParam> listParams;
		Define("LineSegmentDetector", listParams, [](const PipelineData& input, const QList<PipelineStepParam>& listParams) {
			// The input image must be an 8 bit grayscale image
			if (input.img.elemSize() != 1)
				EXERR("RRT3", "LineSegmentDetector requires a grayscale input. Try using Canny() edge detection first.");

			cv::Ptr<cv::LineSegmentDetector> det = cv::createLineSegmentDetector();



			cv::Mat lines;
			det->detect(input.img, lines);

			// Draw the lines onto a blank image of the same size
			PipelineData out;
			out.img = cv::UMat::zeros(input.img.rows, input.img.cols, CV_8UC3);

			det->drawSegments(out.img, lines);

			return out;
			});
	}*/
}
