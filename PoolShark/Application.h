#pragma once
#include <QApplication>
#include <Exception.h>

class Application : public QApplication
{
	Q_OBJECT
public:
	Application(int& argc, char** argv);

signals:
	void UnhandledException(ExceptionContainer exc);

protected:
	virtual bool notify(QObject* rec, QEvent* ev);
};

