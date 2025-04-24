#pragma once

#include "AFLibThread.h"

/////////////////////////////////////////////////////////////////////////////
// TWorker
// A thread with data; used together with TWork to solve a set of similar tasks

namespace AFLibThread {
class TWork;

class AFLIB TWorker
{
private:
    friend TWork;
    DEFINE_COPY_AND_ASSIGN(TWorker);

    TWork* m_pWork;      // container
    CThread m_thread;    // thread ID and handle
    CString m_strError;  // filled if thread ends with an error

protected:
    // process one task from the set
    virtual void ProcessTask();

    // do the actual work
    virtual void Run();

    // run this thread, catch exceptions
    virtual void RunAndCatch();

    // static version of RunAndCatch()
    static UINT StaticRun(LPVOID _pWk);

public:
    explicit TWorker(TWork* pWork);
    virtual ~TWorker();

    // get the container work
    TWork* GetWork() const
    {
        return m_pWork;
    }
};
}  // namespace AFLibThread
