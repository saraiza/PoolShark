#pragma once
#include "Logging.h"

namespace Logging {


/**
@brief Debug output logger.

This is the default ILog implementation. If you don't setup
anything different, you will get this logger. All it
does is output to the debugger's output window (OutputDebugString).
*/
class TraceLog : public ILog
{
public:
	virtual void Log(const QString& line) override;
	virtual void Flush() override {};
	virtual void Start() override {};

    virtual void RedirectIOToConsole();
	virtual QString GetLogFileName() override { return QString(); };
	virtual void Hint(Hints hint) override {};

};



} // namespace
