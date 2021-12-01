#include "LambdaTask.h"

LambdaTask::LambdaTask(QString sTaskName, AutoRethrowMethod autoRethrowMethod, QObject *parent)
    : Task(sTaskName, autoRethrowMethod, parent)
{

}

void LambdaTask::SetFunction(std::function<void ()> func)
{
    m_funcTask = func;
}

void LambdaTask::Start()
{
    //Ensure that someone has set the function object to a valid function
    Q_ASSERT(m_funcTask);

	Task::Start();
}

void LambdaTask::Start(std::function<void ()> func)
{
	SetFunction(func);
	Start();
}

void LambdaTask::CheckAbort()
{
    Task::CheckAbort();
}

void LambdaTask::RunTask()
{
    //Just run the stored function and return
    m_funcTask();
}