#include "LambdaDispatch.h"

void LambdaDispatch::RunLambda(runnable func) const
{
	QMetaObject::invokeMethod(const_cast<LambdaDispatch*>(this), "RunLambdaSlot", Qt::BlockingQueuedConnection, Q_ARG(runnable, func));
}

void LambdaDispatch::RunlambdaAsync(runnable func) const
{
	QMetaObject::invokeMethod(const_cast<LambdaDispatch*>(this), "RunLambdaSlot", Qt::QueuedConnection, Q_ARG(runnable, func));
}

void LambdaDispatch::RunLambdaSlot(runnable func)
{
	func();
}
