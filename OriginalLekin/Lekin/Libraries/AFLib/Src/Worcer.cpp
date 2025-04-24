#include "StdAfx.h"

#include "AFLibWorcer.h"

#include "AFLibGlobal.h"
#include "AFLibWorc.h"

using namespace AFLibThread;
using namespace AFLib;

TWorcer::TWorcer(TWorc* pWorc)
{
    m_pWorc = pWorc;
}

TWorcer::~TWorcer()
{
    m_thread.WaitFor();
}

bool TWorcer::RunHelper(bool bStart)
{
    bool bOk = true;
    try
    {
        if (bStart)
            RunStart();
        else
            bOk = RunStage();
    }
    catch (CException* pExc)
    {
        m_strError = GetExceptionMsg(pExc);
        pExc->Delete();
        if (m_strError.IsEmpty()) m_strError = strEllipsis;
        return false;
    }
    catch (...)
    {
        m_strError = strEllipsis;
        return false;
    }

    return bOk;
}

void TWorcer::Run()
{
    if (!RunHelper(true))
    {
        m_evDone.SetEvent();
        return;
    }

    HANDLE arrEvent[2] = {m_evCompute, m_evFinish};
    m_evDone.SetEvent();

    while (true)
    {
        DWORD e = WaitForMultipleObjects(2, arrEvent, false, INFINITE);
        if (e != WAIT_OBJECT_0) break;
        if (!RunHelper(false)) break;

        m_evCompute.ResetEvent();
        m_evDone.SetEvent();
    }

    m_evDone.SetEvent();
}

UINT TWorcer::StaticRun(LPVOID _pWc)
{
    TWorcer* pWc = reinterpret_cast<TWorcer*>(_pWc);
    pWc->Run();
    pWc->RunFinish();
    return 0;
}

void TWorcer::RunStart()
{}

void TWorcer::RunFinish()
{}

bool TWorcer::IsFinishRaised()
{
    return WaitForSingleObject(m_evFinish, 0) == WAIT_OBJECT_0;
}
