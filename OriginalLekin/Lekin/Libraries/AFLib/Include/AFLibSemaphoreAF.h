#pragma once

#include "AFLibDefine.h"

class CCriticalSection;
class CEvent;

/////////////////////////////////////////////////////////////////////////////
// CSemaphoreAF
// Semaphore that allows locking multiple units of "resource", as well as
//   "exclusive" locking

namespace AFLibThread {
class AFLIB CSemaphoreAF
{
private:
    DEFINE_COPY_AND_ASSIGN(CSemaphoreAF);

    volatile int m_count;  // counter of free resources
    int m_maxCount;        // max number of resources

    CCriticalSection* m_pSync;  // for limiting access to the counter
    CEvent* m_pEvInc;           // signals that m_count was increased

public:
    explicit CSemaphoreAF(int initialCount = INT_MAX, int maxCount = 0);
    ~CSemaphoreAF();

    // get max number of resouces
    int GetMaxCount() const
    {
        return m_maxCount;
    }

    // get current number of free resources
    int GetFreeCount() const
    {
        return m_count;
    }

    // lock given number of resources
    bool Lock(int count = 1, DWORD dwTimeout = INFINITE);

    // lock all resources
    bool LockExclusive(DWORD dwTimeout = INFINITE);

    // unlock given number of resources
    void Unlock(int count);
};
}  // namespace AFLibThread
