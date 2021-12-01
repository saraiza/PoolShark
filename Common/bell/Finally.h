#ifndef FINALLY_H
#define FINALLY_H

#include <iostream>
#include <functional>
#include <stdexcept>
#include <random>

/*

An implementation of the finally keyword in C++. The idea behind a finally
keyword is to ensure a piece of code runs when a function exits. This is
important with languages that support exceptions. Since any function you
call may throw an exception, you must assume that your function could exit
at any time. If your function exits, you must ensure you've cleaned up any
resources (locks, files, DB transactions) you allocated at the start of the
function. This problem is solved in c++ with the RAII pattern. However, the
RAII pattern requires you to have a class written for the specific task you
are doing. In some cases it does not make sense to write an entire new class
just to clean up a one-off resource. In this case, a finally keyword would
be useful.

In the Finally example, I have implement a finally keyword using a combination
of a macro, and a lambda function. the FINALLY macro declares a class named
ScopedLambda. The ScopedLambda class stores the lambda function that runs when
the class goes out of scope. This means that any code contained in the FINALLY
macro will run when the function exits. Regardless of the reason or time of exit.

Here's an example of a function that prints a message when the function exits.
In this case, an exception is thrown.

void testFunc()
{
    FINALLY(LOGERR("This text will print when the function ends."););

    throw runtime_error("Some exception happened");
}

*/

/**
@brief ScopedLambda class

Takes a reference to a lambda and runs it when the class is destroyed
You can use this to always ensure a bit of code is run when a function or scope ends.
*/

template <class Lambda>
class ScopedLambda {
public:
    ScopedLambda(const Lambda &func) : m_func(func) {}
    ScopedLambda(const ScopedLambda &other) = delete; //No copy constructor, function would execute twice!

    ~ScopedLambda()
    {
        m_func();
    }

private:
    const Lambda& m_func;
};

//Some Macro magic here to make the Finally() macro work
#define FINALLY_CONCAT_PAIR_INNER(x,y) x ## y
#define FINALLY_CONCAT_PAIR(x,y) FINALLY_CONCAT_PAIR_INNER(x,y)

#define FINALLY_INNER2(lambda, object, ...) auto lambda = [&]() { __VA_ARGS__ ;}; ScopedLambda<decltype(lambda)> object(lambda);
#define FINALLY_INNER(unique, ...) FINALLY_INNER2(FINALLY_CONCAT_PAIR(finallyLambda, unique), \
    FINALLY_CONCAT_PAIR(finallyObject, unique), __VA_ARGS__)

//The Finally() macro does the real magic here. It constructs a lambda and ScopedLambda object from the arguments passed to it
#define FINALLY(...) FINALLY_INNER(__LINE__, __VA_ARGS__)





/// An easier to use non-template RAII class

using FinallyFunc = std::function<void()>;

class ScopedLambda2 {
public:
	ScopedLambda2(FinallyFunc &func)
	{
		m_func = func;
	}
	ScopedLambda2(const ScopedLambda2 &other) = delete; //No copy constructor, function would execute twice!

	~ScopedLambda2()
	{
		m_func();
	}

private:
	FinallyFunc m_func;
};

#define FINALLY2(lmbda)		

#endif // FINALLY_H
