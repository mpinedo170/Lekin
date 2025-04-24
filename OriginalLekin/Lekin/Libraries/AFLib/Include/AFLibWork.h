#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TWork
// A bunch of threads with data, solving tasks from a set

namespace AFLibThread {
class TWorker;

class AFLIB TWork
{
private:
    DEFINE_COPY_AND_ASSIGN(TWork);

    // array of "workers"
    AFLib::CSmartArray<TWorker> m_arrWk;

protected:
    // synchronization object (not used in the base class)
    CCriticalSection m_sync;

    // create a worker
    virtual TWorker* CreateWorker();

    // called when multithreaded execution stops (normally or otherwise)
    virtual void CleanWork();

public:
    TWork();
    virtual ~TWork();

    // create the workers, start them, wait till they finish
    void ExecuteWork(int nWorkers);

    // take the next task; return false if tasks are over
    virtual bool NextTask(TWorker* pWk) = 0;

    // more than one thread?
    bool IsMultiThread()
    {
        return !m_arrWk.IsEmpty();
    }
};
}  // namespace AFLibThread
