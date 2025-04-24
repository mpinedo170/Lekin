#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TWorc
// A bunch of threads with data, solving tasks in stages

namespace AFLibThread {
class TWorcer;

class AFLIB TWorc
{
private:
    DEFINE_COPY_AND_ASSIGN(TWorc);

    AFLib::CSmartArray<TWorcer> m_arrWc;  // array of Worcers

    // stop all threads
    void StopAll(bool bForce);

protected:
    // create a worker
    virtual TWorcer* CreateWorcer(int index, int nWorcers) = 0;

public:
    TWorc();
    virtual ~TWorc();

    // create all Workcers
    void StartWorc(int nWorcers);

    // set evFinish for all Worcers, wait; if bForce, Break each thread
    void FinishWorc(bool bForce);

    // finish all Worcers, then clear m_arrWc
    // IMPORTANT: FinishAndDestroy(true) has to be explicitly called
    //    from the destructors of every derived class!
    void FinishAndDestroy(bool bForce);

    // allow the threads compute a stage
    void StartStage();

    // wait for all threads to finish the stage
    void WaitStage();

    // get the number of worcers
    int GetWcCount() const
    {
        return m_arrWc.GetSize();
    }

    // get the indexed worcer (const)
    const TWorcer* GetWc(int index) const
    {
        return m_arrWc[index];
    }

    // get the indexed worcer (non-const)
    TWorcer* GetWc(int index)
    {
        return m_arrWc[index];
    }
};
}  // namespace AFLibThread
