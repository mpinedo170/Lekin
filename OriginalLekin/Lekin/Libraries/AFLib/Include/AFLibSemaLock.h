#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CSemaLock
// A single lock class for CSemaphoreAF

namespace AFLibThread {
class CSemaphoreAF;

class AFLIB CSemaLock
{
private:
    DEFINE_COPY_AND_ASSIGN(CSemaLock);

    CSemaphoreAF* m_pSemaphore;  // underlying semaphore
    int m_maxCount;              // cached m_pSemaphore->GetMaxCount()
    int m_count;                 // number of locked resources

public:
    // use exclusive in constructor or Lock() to lock all resources in pSemaphore
    enum
    {
        exclusive = -0xAF
    };

    explicit CSemaLock(CSemaphoreAF* pSemaphore, int initialLock = 1);
    ~CSemaLock();

    // attempt to lock given amount of the resource; count may be = exclusive
    bool Lock(int count = 1, DWORD dwTimeout = INFINITE);

    // unlock given amount of the resource
    void Unlock(int count = INT_MAX);

    // unlock all except a few units of the resource
    void UnlockExcept(int count = 1);

    // max number of resources
    int GetMaxCount() const
    {
        return m_maxCount;
    }

    // amount of locked resources
    int GetLockedCount() const
    {
        return m_count;
    }
};
}  // namespace AFLibThread
