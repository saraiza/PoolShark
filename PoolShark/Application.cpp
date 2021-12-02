#include "Application.h"



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
	catch (...)
	{
		qDebug() << "Unknown Exception: Terminating!";

		std::terminate();
	}
	return false;
}