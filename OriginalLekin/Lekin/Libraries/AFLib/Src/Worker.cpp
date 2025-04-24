#include "StdAfx.h"

#include "AFLibWorker.h"

#include "AFLibGlobal.h"
#include "AFLibWork.h"

using namespace AFLibThread;

TWorker::TWorker(TWork* pWork)
{
    m_pWork = pWork;
}

TWorker::~TWorker()
{}

void TWorker::ProcessTask()
{}

void TWorker::RunAndCatch()
{
    try
    {
        Run();
    }
    catch (CException* pExc)
    {
        m_strError = AFLib::GetExceptionMsg(pExc);
        if (!m_pWork->IsMultiThread()) throw;
        pExc->Delete();
    }
    if (m_pWork->IsMultiThread()) CThread::CloseHeap();
}

void TWorker::Run()
{
    while (m_pWork->NextTask(this))
    {
        CThread::Break();
        ProcessTask();
    }
}

UINT TWorker::StaticRun(LPVOID _pWk)
{
    TWorker* pWk = reinterpret_cast<TWorker*>(_pWk);
    pWk->RunAndCatch();
    return 0;
}
