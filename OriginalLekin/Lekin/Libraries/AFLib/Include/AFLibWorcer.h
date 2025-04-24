#pragma once

#include "AFLibThread.h"

/////////////////////////////////////////////////////////////////////////////
// TWorcer
// A thread with data; used together with TWorc to solve a set of tasks in stages

namespace AFLibThread {
class TWorc;

class AFLIB TWorcer
{
private:
    friend TWorc;
    DEFINE_COPY_AND_ASSIGN(TWorcer);

    TWorc* m_pWorc;      // container
    CThread m_thread;    // thread ID and handle
    CEvent m_evCompute;  // event to continue computing
    CEvent m_evFinish;   // event to finish computing
    CEvent m_evDone;     // raised when an operation is done
    CString m_strError;  // filled if thread ends with an error

    // static version of RunAndCatch()
    static UINT StaticRun(LPVOID _pWc);

    // helper for Run()
    bool RunHelper(bool bStart);

protected:
    // startup (runs before the loop)
    virtual void RunStart();

    // process the current stage; returns false if thread has to stop
    virtual bool RunStage() = 0;

    // cleanup (runs after the loop)
    virtual void RunFinish();

    // loop to run the stages
    void Run();

    // finish was called; stop gracefully
    bool IsFinishRaised();

public:
    explicit TWorcer(TWorc* pWorc);
    virtual ~TWorcer();

    // get the container worc
    TWorc* GetWorc() const
    {
        return m_pWorc;
    }
};
}  // namespace AFLibThread
