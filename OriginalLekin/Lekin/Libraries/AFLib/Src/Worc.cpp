#include "StdAfx.h"

#include "AFLibWorc.h"

#include "AFLibGlobal.h"
#include "AFLibWorcer.h"

using namespace AFLibThread;
using namespace AFLib;

TWorc::TWorc()
{}

TWorc::~TWorc()
{
    FinishAndDestroy(true);
}

void TWorc::StopAll(bool bForce)
{
    for (int i = 0; i < m_arrWc.GetSize(); ++i)
    {
        TWorcer* pWc = m_arrWc[i];
        pWc->m_evFinish.SetEvent();
        if (bForce) pWc->m_thread.StopNoWait();
    }
}

void TWorc::StartWorc(int nWorcers)
{
    ASSERT(m_arrWc.IsEmpty());
    nWorcers = max(nWorcers, 1);

    for (int i = 0; i < nWorcers; ++i) m_arrWc.Add(CreateWorcer(i, nWorcers));

    for (int i = 0; i < nWorcers; ++i)
    {
        TWorcer* pWc = m_arrWc[i];
        pWc->m_thread = CThread::Start(TWorcer::StaticRun, pWc);
    }

    WaitStage();
}

void TWorc::FinishWorc(bool bForce)
{
    if (m_arrWc.IsEmpty()) return;

    int nThreads = m_arrWc.GetSize();
    HANDLE* arrHThread = reinterpret_cast<HANDLE*>(_alloca(nThreads * sizeof(HANDLE)));

    for (int i = nThreads; --i >= 0;) arrHThread[i] = m_arrWc[i]->m_thread;

    DWORD e = WaitForMultipleObjects(nThreads, arrHThread, true, 0);
    if (e >= WAIT_OBJECT_0 && e < WAIT_OBJECT_0 + nThreads) return;

    StopAll(bForce);
    WaitForMultipleObjects(nThreads, arrHThread, true, INFINITE);
}

void TWorc::FinishAndDestroy(bool bForce)
{
    FinishWorc(bForce);
    m_arrWc.DestroyAll();
}

void TWorc::StartStage()
{
    for (int i = 0; i < m_arrWc.GetSize(); ++i)
    {
        TWorcer* pWc = m_arrWc[i];
        pWc->m_evDone.ResetEvent();
        pWc->m_evCompute.SetEvent();
    }
}

void TWorc::WaitStage()
{
    if (m_arrWc.IsEmpty()) return;

    int nThreads = m_arrWc.GetSize();
    TWorcer** arrWc = reinterpret_cast<TWorcer**>(_alloca(nThreads * sizeof(TWorcer*)));
    HANDLE* arrHEvDone = reinterpret_cast<HANDLE*>(_alloca(nThreads * sizeof(HANDLE)));

    for (int i = 0; i < m_arrWc.GetSize(); ++i)
    {
        arrWc[i] = m_arrWc[i];
        arrHEvDone[i] = m_arrWc[i]->m_evDone;
    }

    try
    {
        while (nThreads > 0)
        {
            CThread::Break();
            DWORD e = WaitForMultipleObjects(nThreads, arrHEvDone, false, 10);

            if (e >= WAIT_OBJECT_0 && e < WAIT_OBJECT_0 + nThreads)
            {
                e -= WAIT_OBJECT_0;
                TWorcer* pWc = arrWc[e];

                if (!pWc->m_strError.IsEmpty()) ThrowMessage(pWc->m_strError);

                --nThreads;
                memmove(arrWc + e, arrWc + e + 1, (nThreads - e) * sizeof(HANDLE));
                memmove(arrHEvDone + e, arrHEvDone + e + 1, (nThreads - e) * sizeof(TWorcer*));
            }
            else if (e != WAIT_TIMEOUT)
                ThrowMessage(strEmpty);
        }
    }
    catch (...)
    {
        StopAll(true);
        throw;
    }
}
