#include "StdAfx.h"

#include "AFLibWork.h"

#include "AFLibGlobal.h"
#include "AFLibWorker.h"

using namespace AFLibThread;
using namespace AFLib;

TWork::TWork()
{}

TWork::~TWork()
{}

TWorker* TWork::CreateWorker()
{
    return new TWorker(this);
}

void TWork::CleanWork()
{
    for (int i = 0; i < m_arrWk.GetSize(); ++i) m_arrWk[i]->m_thread.StopNoWait();

    for (int i = 0; i < m_arrWk.GetSize(); ++i) m_arrWk[i]->m_thread.WaitFor();

    m_arrWk.DestroyAll();
}

void TWork::ExecuteWork(int nWorkers)
{
    try
    {
        if (nWorkers <= 1)
        {
            CAutoPtr<TWorker> pWk(CreateWorker());
            pWk->RunAndCatch();
            return;
        }

        CArray<HANDLE, HANDLE> arrHThread;
        arrHThread.SetSize(nWorkers);
        m_arrWk.SetSize(nWorkers);

        for (int i = 0; i < nWorkers; ++i)
        {
            TWorker* pWk = CreateWorker();
            m_arrWk[i] = pWk;
            pWk->m_thread = CThread::Start(TWorker::StaticRun, pWk);
            arrHThread[i] = pWk->m_thread;
        }

        while (!arrHThread.IsEmpty())
        {
            CThread::Break();
            DWORD e = WaitForMultipleObjects(arrHThread.GetSize(), arrHThread.GetData(), false, 10);

            if (e >= WAIT_OBJECT_0 && e < WAIT_OBJECT_0 + arrHThread.GetSize())
            {
                e -= WAIT_OBJECT_0;
                if (!m_arrWk[e]->m_strError.IsEmpty()) ThrowMessage(m_arrWk[e]->m_strError);
                m_arrWk.DestroyAt(e);
                arrHThread.RemoveAt(e);
            }
            else if (e != WAIT_TIMEOUT)
                ThrowMessage(strEmpty);
        }
    }
    catch (...)
    {
        CleanWork();
        throw;
    }

    CleanWork();
}
