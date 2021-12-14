#include "stdafx.h"
#include "Application.h"
#include <opencv2/core/core.hpp> 


DECLARE_LOG_SRC("Application", LOGCAT_Common);


Application::Application(int& argc, char** argv)
	: QApplication(argc, argv)
{

}

bool Application::notify(QObject* rec, QEvent* ev)
{
	// uncomment this to debug hung event loops
//	qDebug() << rec << ev;
	try
	{
		return QApplication::notify(rec, ev);
	}
	catch (const Exception&)
	{
		// we emit the signal here so the UI thread can be the one to create the dialog.
		emit UnhandledException(ExceptionContainer::CurrentException());
	}
	catch (const cv::Exception& e)
	{
		const char* err_msg = e.what();
		QString sMsg = QString("Open CV Exception:\n\n%1").arg(QString(err_msg));
		ExceptionContainer exc = ExceptionContainer::CreateExceptionContainer<Exception>(sMsg);
		emit UnhandledException(exc);
	}
	catch (...)
	{
		LOGERR("Unknown Exception: Terminating!");

		std::terminate();
	}
	return false;
}