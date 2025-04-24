#include "StdAfx.h"

#include "AFLibSemaphoreAF.h"

#include "AFLibThread.h"

using namespace AFLibThread;

CSemaphoreAF::CSemaphoreAF(int initialCount, int maxCount)
{
    ASSERT(initialCount >= 0);
    m_count = initialCount;
    m_maxCount = max(initialCount, maxCount);
    ASSERT(m_maxCount > 0);

    m_pSync = new CCriticalSection;
    m_pEvInc = new CEvent(false, true);
}

CSemaphoreAF::~CSemaphoreAF()
{
    delete m_pEvInc;
    delete m_pSync;
}

bool CSemaphoreAF::Lock(int count, DWORD dwTimeout)
{
    ASSERT(count > 0 && count <= m_maxCount);
    INT64 msStart = GetTickCount();

    while (true)
    {
        CThread::Break();
        CSingleLock lock(m_pSync, true);

        if (m_count >= count)
        {
            m_count -= count;
            return true;
        }

        DWORD msWait = 100;

        if (dwTimeout != INFINITE)
        {
            INT64 msSpent = INT64(GetTickCount()) - msStart;
            // take care of GetTickCount() wraparound at 49.7 days
            if (msSpent < 0) msSpent += INT64(1) << 32;
            if (msSpent > dwTimeout) return false;
            msWait = min(dwTimeout - DWORD(msSpent), msWait);
        }

        // Not enough resources.
        // Need to wait till some thread frees a few.
        // When that happens, an event is signalled.

        HANDLE hEvInc = *m_pEvInc;
        CThread::Break();
        lock.Unlock();

        // Can't wait indefinitely.
        // If other thread decrements the count and pulses the event RIGHT HERE,
        //   WaitForSingleObject() below will not notice that.
        // It will wait till yet another thread grabs and releases the resource.
        WaitForSingleObject(hEvInc, msWait);
    }
}

bool CSemaphoreAF::LockExclusive(DWORD dwTimeout)
{
    return Lock(m_maxCount, dwTimeout);
}

void CSemaphoreAF::Unlock(int count)
{
    if (count <= 0) return;

    CSingleLock lock(m_pSync, true);
    ASSERT(count <= m_maxCount - m_count);

    m_count += count;
    m_pEvInc->PulseEvent();
}
