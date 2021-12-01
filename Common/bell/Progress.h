#pragma once

#include <QObject>
#include <functional>

/**
@brief Interface for generic C++ progress notification.

When a component reports progress, you provide it an implementation of
the IProgressSink interface and the component will call into it.



See the documentation for the Progress class below.
*/
class IProgressSink 
{
public:
	virtual ~IProgressSink() = default;

	/// Report a status message. The caller will provide the translation.
    virtual void Prog_OnStatus(const QString& sStatus) { Q_UNUSED(sStatus); }

	/// Report progress percentage from 0 to 100. It should always increase, never drop in value.
	virtual void Prog_OnProgress(int iProgress) = 0;

	/// Called when the job has been completed
    virtual void Prog_OnComplete() {};

	/// Called when the job has been aborted
    virtual void Prog_OnAborted() {};

	/// Called when the job has an error to report (it is done trying if it calls this)
    virtual void Prog_OnError(const QString& sErrMsg) { Q_UNUSED(sErrMsg) ;}

// Optional behavior, not all consumers will use these.
	virtual bool Prog_CheckCancel(void) { return false; }
};


/**
@brief Default implementation for IProgressSink

Sometimes you need to use a class that requires an IProgressSink
to be passed in to it, but you know that you don't care about
progress and don't need to waste time implementing a sink.

Just pass in a NullProgressSink instead.
*/
class NullProgressSink : public IProgressSink 
{
public:
	virtual ~NullProgressSink() override;
	virtual void Prog_OnStatus(const QString& /*status*/) override {};
	virtual void Prog_OnProgress(int /*iProgress*/) override {};
	virtual void Prog_OnComplete(void) override {};
	virtual void Prog_OnAborted(void) override {};
	virtual void Prog_OnError(const QString& /*sErrMsg*/) override {};
};



/**
@brief Gate, stops the flow of calls

If you need to support multiple progress chains that route to the same
sink, you can use this class as a kind of valve to turn off the calls
from one of the chains and turn on the other chain.
*/
class ProgressGate : public IProgressSink
{
public:
	ProgressGate(IProgressSink *pInner);
	virtual ~ProgressGate() override;
	void SetEnabled(bool bEnable);
	void SetStatusEnabled(bool bEnable);
	void SetInner(IProgressSink *pInner);
	virtual void Prog_OnStatus(const QString& sStatus) override;
	virtual void Prog_OnProgress(int iProgress) override;
	virtual void Prog_OnComplete(void) override;
	virtual void Prog_OnAborted(void) override;
	virtual void Prog_OnError(const QString& sErrMsg) override;
private:
	bool m_bEnabled = true;
	bool m_bStatusEnabled = true;
	IProgressSink *m_pInner = nullptr;
};



/**
@brief Interface for the Progress class.

IProgressSink and the Progress class helps you manage reporting progress percentages.
It is hierarchical. The Progress class wraps an IProgressSink. It always forwards all
notifications to the inner sink. 

here is how you would use it:
  
~~~~~~~~~~~~~~~~~~~~~{.c}
void BigJob(IProgressSink *pSink)
{
	//// This big job consists of two small jobs
	Progress prog(pSink);

	//// Job one 
	Progress prog1(&prog, 0, 50);	// Specify the range
	SmallJobA(&prog1);

	//// Job two 
	Progress prog2(&prog, 50, 100);	// Specify the range
	SmallJobB(&prog2);

	//// We are done!
	prog.SetComplete();
}


void MySmallJob(Progress& progress)
{
    //// Break this up into three jobs

	//// Job one 
	Progress prog1(&progress, 0, 33);	// Specify the range
	MySmallestJob(&prog1);

	//// Job two (we just reuse the same function for simplicity)
	Progress prog2(&progress, 33, 66);	// Specify the range
	MySmallestJob(&prog2);

	//// Job two (we just reuse the same function for simplicity)
	Progress prog3(&progress, 66, 100);	// Specify the range
	MySmallestJob(&prog3);
}



void MySmallestJob(Progress& progress)
{
	//// Perform our work
	progress.SetProgress(25);
	progress.SetProgress(50);
	progress.SetProgress(75);
	progress.SetProgress(100);
}
~~~~~~~~~~~~~~~~~~~~~

  Notice that each function only has to be concerned with
  tracking its local percentage. It does not know or care that
  its idea of 100% does not actually translate into a true 100%.
*/
class Progress : public QObject, public IProgressSink  
{
	Q_OBJECT;
public:
	Progress(IProgressSink *pSink = nullptr);
	Progress(IProgressSink *pSink, int iStart, int iStop);
	virtual ~Progress() override;
	
	void Init(IProgressSink *pSink, int iStart, int iStop);

	// Prettier methods (rather than the event callback methods)
	void SetStatus(const QString& status);
	void SetProgress(int iProgress);
	void SetCustomState(int state);
	void SetComplete(void);
	void SetAborted(void);
	void SetError(const QString& errorMsg);
	bool IsCancelled(void);
	void operator +=(int iProgressIncrement);

signals:
	void newStatus(QString status);
	void progress(int progress);
	void completed();
	void aborted();
	void error(QString sErrMsg);

protected:
	// IProgressSink implementation
    virtual void Prog_OnStatus(const QString& status) override;
    virtual void Prog_OnProgress(int iProgress) override;
    virtual void Prog_OnComplete() override;
    virtual void Prog_OnAborted() override;
    virtual void Prog_OnError(const QString& sErrMsg) override;

	// ICancelable implementation (IProgressSink inherits from ICancellable)
	virtual bool Prog_CheckCancel(void) override;

	// Only used by other Progress instances
	float GetProgress(void) { return m_fProgress; }

	/// Reuse the global NullProgressSink
	static NullProgressSink* Null();

private:

	IProgressSink *m_pInnerSink = nullptr;

	// These are 0.0f to 1.0f
	float m_fProgress;	///< The current progress position (for the job that we represent)
	float m_fStart;		///< Our 0% really means this value (from the inner sink)
	float m_fRange;		///< The range of progress. (100% = > m_fStart + m_fRange)

    int m_iPreviousProgress = 0;
};


