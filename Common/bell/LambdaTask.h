#pragma once

#include "Task.h"
#include <functional>

/**
@brief Create a task from a lambda instead of inheriting Task directly

In cases where you need a small, single use Task, it may not make sense
to inherit from the task class. Instead, you can use this class to
create a task in-line. This is similar in concept to the QtConcurent::run
function, but allows for the lifetime and exception behavior of a Task
object to be used.

To use, simply create an instance of this class, call SetFunction with a lambda
or other callable, and then run Start(). Some function from Task have been made
public. This allows them to be called from the external lambda.

**/
class LambdaTask : public Task
{
public:
    LambdaTask(QString sTaskName, AutoRethrowMethod autoRethrowMethod, QObject *parent = 0);

    void SetFunction(std::function<void()> func);
    void Start() override;
	void Start(std::function<void()> func);

    //Re-declare these functions as public, so they can be used from an external function
    void CheckAbort();

	void SleepMs(int iMs)
	{
		Task::SleepMs(iMs);
	}

    //Use some fancy "Perfect Forwarding" magic to handle all the possible
    //overloads of the Wait() function without havint to re-declare them here
    //http://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c
    template <typename... Args>
    int Wait(Args &&...args)
    {
        return Task::Wait(std::forward<Args>(args)...);
    }

protected:
    void RunTask() override;

private:
    std::function<void()> m_funcTask;
};


