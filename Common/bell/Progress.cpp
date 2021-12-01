
#include "Progress.h"




// initialize static member
static NullProgressSink gs_nullSink;


NullProgressSink* Progress::Null() 
{ 
	return &gs_nullSink; 
}


NullProgressSink::~NullProgressSink()
{
}

/***************************************************************/


ProgressGate::ProgressGate(IProgressSink *pInner)
{
	m_pInner = pInner;
}

ProgressGate::~ProgressGate()
{

}


void ProgressGate::SetInner(IProgressSink *pInner)
{
	m_pInner = pInner;
}

void ProgressGate::SetEnabled(bool bEnable)
{
	m_bEnabled = bEnable;
}

void ProgressGate::SetStatusEnabled(bool bEnable)
{
	m_bStatusEnabled = bEnable;
}

void ProgressGate::Prog_OnStatus(const QString& sStatus)
{
	if (m_bEnabled && m_bStatusEnabled)
		m_pInner->Prog_OnStatus(sStatus);
}


void ProgressGate::Prog_OnProgress(int iProgress)
{
	if (m_bEnabled)
		m_pInner->Prog_OnProgress(iProgress);
}

void ProgressGate::Prog_OnComplete(void)
{
	if (m_bEnabled)
		m_pInner->Prog_OnComplete();
}


void ProgressGate::Prog_OnAborted(void) 
{
	if (m_bEnabled)
		m_pInner->Prog_OnAborted();
}

void ProgressGate::Prog_OnError(const QString& sErrMsg) 
{
	if (m_bEnabled)
		m_pInner->Prog_OnError(sErrMsg);
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/***************************************************************
Progress - constructor

	This version wraps another Progress instance.
***************************************************************/
Progress::Progress(IProgressSink *pSink)
{
	if(NULL == pSink)
		m_pInnerSink = &gs_nullSink;
	else
		m_pInnerSink = pSink;
	
	m_fStart = 0.0f;
	m_fProgress = 0.0f;
	m_fRange = 1.0f;
}


/**
@brief constructor

This version wraps another Progress sink.
*/
Progress::Progress(IProgressSink *pInnerSink, int iStart, int iStop)
{
	Init(pInnerSink, iStart, iStop);
}

void Progress::Init(IProgressSink *pInnerSink, int iStart, int iStop)
{
	if(NULL == pInnerSink)
		m_pInnerSink = &gs_nullSink;
	else
		m_pInnerSink = pInnerSink;
	
	m_fProgress = 0.0f;

	m_fStart = (float)iStart/100.0f;
	m_fRange = (float)(iStop-iStart)/100.0f;
    m_iPreviousProgress = 0;

	// Reset to zero. I used to do this. But now some new usages of Progress
	// distance the construction from the usage
	//SetProgress(0);
}


Progress::~Progress()
{

}

void Progress::operator +=(int iProgressIncrement)
{
	SetProgress((int)(m_fProgress*100.0f + (float)iProgressIncrement + 0.5f));
}


void Progress::SetProgress(int iProgress)
{
	Prog_OnProgress(iProgress);
}




void Progress::SetStatus(const QString& status)
{
	Prog_OnStatus(status);
}



void Progress::SetComplete(void)
{
	Prog_OnComplete();
}


void Progress::SetAborted(void)
{
	Prog_OnAborted();
}


void Progress::SetError(const QString& errorMsg)
{
	Prog_OnError(errorMsg);
}


bool Progress::IsCancelled(void)
{
	return Prog_CheckCancel();
}





void Progress::Prog_OnStatus(const QString& status)
{
	m_pInnerSink->Prog_OnStatus(status);
	emit newStatus(status);
}

void Progress::Prog_OnProgress(int iProgress)
{
	// Massage the progress into the inner scale.
	m_fProgress = (float)iProgress/100.0f;
	float fInnerProgress = m_fStart + m_fRange * m_fProgress;
	int newProgress = (int)(100 * fInnerProgress + 0.5f);
	m_pInnerSink->Prog_OnProgress(newProgress);
    if (newProgress != m_iPreviousProgress)
    {
        // only emit progress signal when the value changes
        emit progress(newProgress);
        m_iPreviousProgress = newProgress;
    }
}

void Progress::Prog_OnComplete(void)
{
	m_pInnerSink->Prog_OnComplete();
	emit completed();
}

void Progress::Prog_OnAborted(void)
{
	m_pInnerSink->Prog_OnAborted();
	emit aborted();
}

void Progress::Prog_OnError(const QString& errMsg)
{
	m_pInnerSink->Prog_OnError(errMsg);
	emit error(errMsg);
}

bool Progress::Prog_CheckCancel(void)
{
	return m_pInnerSink->Prog_CheckCancel();
}

