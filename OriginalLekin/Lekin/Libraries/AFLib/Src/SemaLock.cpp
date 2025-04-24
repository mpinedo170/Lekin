#include "StdAfx.h"

#include "AFLibSemaLock.h"

#include "AFLibSemaphoreAF.h"

using namespace AFLibThread;

CSemaLock::CSemaLock(CSemaphoreAF* pSemaphore, int initialLock)
{
    m_pSemaphore = pSemaphore;
    m_maxCount = m_pSemaphore->GetMaxCount();
    m_count = 0;
    Lock(initialLock);
}

CSemaLock::~CSemaLock()
{
    Unlock();
}

bool CSemaLock::Lock(int count, DWORD dwTimeout)
{
    ASSERT(count >= 0 || count == exclusive);

    if (count == exclusive) count = m_maxCount - m_count;

    if (count == 0) return true;
    if (count > m_maxCount - m_count) return false;

    if (m_pSemaphore->Lock(count, dwTimeout)) m_count += count;
    return true;
}

void CSemaLock::Unlock(int count)
{
    count = min(count, m_count);
    m_pSemaphore->Unlock(count);
    m_count -= count;
}

void CSemaLock::UnlockExcept(int count)
{
    ASSERT(count <= m_count);
    Unlock(m_count - count);
}
