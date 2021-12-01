#pragma once

#include <QObject>
#include <functional>

/** @brief This class jush pushes lambda's on to a thread queue

	This class can go away in Qt 5.12, which supports pushing lambda's
	directly on to an objects thread...

*/
class LambdaDispatch : public QObject
{
	Q_OBJECT
public:
	LambdaDispatch() = default; // This shouldn't have a parent, it needs to be pushed to a thread

	typedef std::function<void()> runnable;

	void RunLambda(runnable func) const;
	void RunlambdaAsync(runnable func) const;

private slots:
	void RunLambdaSlot(runnable func);
};
Q_DECLARE_METATYPE(LambdaDispatch::runnable)
