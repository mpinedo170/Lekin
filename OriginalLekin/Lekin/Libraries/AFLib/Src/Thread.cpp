#include "StdAfx.h"

#include "AFLibThread.h"

#include "AFLibExcBreak.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringParser.h"
#include "AFLibStrings.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibThread;
using namespace AFLibPrivate;

CPointerMap<int, int, int> CThread::m_mapBreak;
CSmartMap<DWORD, DWORD, CString, LPCTSTR> CThread::m_mapBrkMsg;
CCriticalSection CThread::m_sync;
CThread::TInit CThread::m_init;

/////////////////////////////////////////////////////////////////////////////
// CThread::TInit

CThread::TInit::~TInit()
{
    m_mapBreak.DestroyAll();
}

/////////////////////////////////////////////////////////////////////////////
// CThread::CBreakLock

namespace {  // how many times the current thread was locked from breaking?
__declspec(thread) int nBreakLocks = 0;

// pointer to the "broken" flag
__declspec(thread) LPINT pBrokenFlag = NULL;
}  // namespace

CThread::CBreakLock::CBreakLock()
{
    m_bLocked = false;
    Lock();
}

CThread::CBreakLock::~CBreakLock()
{
    Unlock();
}

void CThread::CBreakLock::Lock()
{
    if (m_bLocked) return;
    m_bLocked = true;
    ++nBreakLocks;
}

void CThread::CBreakLock::Unlock()
{
    if (!m_bLocked) return;
    m_bLocked = false;
    --nBreakLocks;
    ASSERT(nBreakLocks >= 0);
}

/////////////////////////////////////////////////////////////////////////////
// CThread

CThread::CThread() : m_idThread(0), m_hThread(NULL)
{}

CThread::CThread(DWORD idThread, HANDLE hThread) : m_idThread(idThread), m_hThread(hThread)
{}

CThread::CThread(DWORD idThread, DWORD dwDesiredAccess) : m_idThread(idThread), m_hThread(NULL)
{
    if (m_idThread != NULL) m_hThread = OpenThread(dwDesiredAccess, false, m_idThread);
}

CThread::CThread(const CThread& thread) : m_idThread(thread.m_idThread), m_hThread(NULL)
{
    m_hThread = Duplicate(thread.m_hThread);
}

CThread::~CThread()
{
    Clear();
}

CThread& CThread::operator=(const CThread& thread)
{
    if (this == &thread) return *this;

    Clear();
    m_idThread = thread.m_idThread;
    m_hThread = Duplicate(thread.m_hThread);
    return *this;
}

void CThread::Clear()
{
    if (!IsValid()) return;
    CloseHandle(m_hThread);
    m_hThread = NULL;
    m_idThread = 0;
}

void CThread::PrivateStop(LPCTSTR message, bool bWait) const
{
    DWORD idThread = m_idThread;
    if (!IsValid()) return;

    CSingleLock lock(&m_sync, true);
    if (IsFinished()) return;

    LPINT pBreak = m_mapBreak.GetAt(idThread);
    if (pBreak == NULL)
    {
        pBreak = new int(0);
        m_mapBreak.SetAt(idThread, pBreak);
    }

    if (!IsEmptyStr(message)) m_mapBrkMsg.SetAt(idThread, message);
    lock.Unlock();
    *pBreak = 1;

    if (!bWait) return;
    WaitFor();

    lock.Lock();
    m_mapBreak.DestroyAt(idThread);
    m_mapBrkMsg.RemoveKey(idThread);
}

HANDLE CThread::Duplicate(HANDLE hThread)
{
    if (hThread == NULL) return NULL;

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hResult = NULL;

    DuplicateHandle(hProcess, hThread, hProcess, &hResult, 0, false, DUPLICATE_SAME_ACCESS);
    return hResult;
}

CThread CThread::Start(AFX_THREADPROC pfnThreadProc,
    LPVOID pParam,
    int nPriority,
    UINT nStackSize,
    DWORD dwCreateFlags,
    LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
    CWinThread* pThread = AfxBeginThread(
        pfnThreadProc, pParam, nPriority, nStackSize, dwCreateFlags | CREATE_SUSPENDED, lpSecurityAttrs);

    DWORD idThread = pThread->m_nThreadID;
    HANDLE hThread = pThread->m_hThread;

    // clear the Break map -- just in case
    CSingleLock lock(&m_sync, true);
    m_mapBreak.DestroyAt(idThread);
    m_mapBrkMsg.RemoveKey(idThread);
    lock.Unlock();

    if (!(dwCreateFlags & CREATE_SUSPENDED)) pThread->ResumeThread();

    return CThread(idThread, Duplicate(hThread));
}

bool CThread::IsBroken()
{
    if (pBrokenFlag != NULL) return *pBrokenFlag != 0;

    DWORD idThread = GetCurrentThreadId();
    CSingleLock lock(&m_sync, true);

    pBrokenFlag = m_mapBreak.GetAt(idThread);
    if (pBrokenFlag != NULL) return *pBrokenFlag != 0;

    pBrokenFlag = new int(0);
    m_mapBreak.SetAt(idThread, pBrokenFlag);
    return false;
}

bool CThread::CanBreak()
{
    return nBreakLocks <= 0;
}

void CThread::SmartSleep(int msec, int inc)
{
    inc = max(inc, 1);

    while (true)
    {
        Break();
        int nSleep = min(inc, msec);

        if (nSleep <= 0) break;
        Sleep(nSleep);
        msec -= nSleep;
    }
}

void CThread::SmartWait(HANDLE h, int inc)
{
    if (h == NULL) return;

    while (true)
    {
        Break();
        if (WaitForSingleObject(h, inc) != WAIT_TIMEOUT) break;
    }
}

void CThread::Break()
{
    SetThreadExecutionState(ES_SYSTEM_REQUIRED);
    if (nBreakLocks > 0 || !IsBroken()) return;

    CSingleLock lock(&m_sync, true);
    DWORD idThread = GetCurrentThreadId();
    CString message;

    m_mapBrkMsg.Lookup(idThread, message);
    pBrokenFlag = NULL;
    m_mapBreak.DestroyAt(idThread);
    m_mapBrkMsg.RemoveKey(idThread);
    throw new CExcBreak(message);
}

bool CThread::IsCurrent() const
{
    return m_idThread == GetCurrentThreadId();
}

bool CThread::IsFinished() const
{
    return IsCurrent() ? false : IsValid() ? WaitForSingleObject(m_hThread, 0) != WAIT_TIMEOUT : true;
}

void CThread::WaitFor(bool bAllowBreak) const
{
    if (!IsValid()) return;

    if (bAllowBreak)
        SmartWait(m_hThread);
    else
        WaitForSingleObject(m_hThread, INFINITE);
}

void CThread::Stop(LPCTSTR message) const
{
    PrivateStop(message, true);
}

void CThread::StopNoWait(LPCTSTR message) const
{
    PrivateStop(message, false);
}

void CThread::Terminate(DWORD dwExitCode)
{
    if (!IsFinished()) TerminateThread(m_hThread, dwExitCode);
}

void CThread::Suspend() const
{
    if (IsValid()) SuspendThread(m_hThread);
}

void CThread::Resume() const
{
    if (IsValid()) ResumeThread(m_hThread);
}

namespace {  // one block of a local heap
struct THeapElement
{
    HANDLE m_hHeap;
    LPVOID m_pStart;
    LPVOID m_pEnd;
    THeapElement* m_pHeNext;

    THeapElement(HANDLE hHeap, THeapElement* pHeNext);

    // try to alocate memory in this heap
    LPVOID Alloc(int nBytes);
};

THeapElement::THeapElement(HANDLE hHeap, THeapElement* pHeNext)
{
    m_hHeap = hHeap;
    m_pStart = m_pEnd = NULL;
    m_pHeNext = pHeNext;
}

LPVOID THeapElement::Alloc(int nBytes)
{
    LPVOID p = HeapAlloc(m_hHeap, 0, nBytes);

    if (p != NULL)
    {
        m_pStart = m_pStart == NULL ? p : min(m_pStart, p);
        m_pEnd = max(m_pEnd, p);
    }

    return p;
}

// handle of the thread's heap
__declspec(thread) THeapElement* pHeMain = NULL;

// memory block size in the current heap
__declspec(thread) int blockSize = 0;

// create a new element of the heap
bool CreateHeapElement(int size)
{
    HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, size, size);
    if (hHeap == NULL) return false;

    pHeMain = new THeapElement(hHeap, pHeMain);
    return true;
}
}  // namespace

bool CThread::OpenHeap(int startSize, int incSize)
{
    static const int MIN_BLOCK_SIZE = MegaByte;
    if (blockSize > 0) return false;

    blockSize = max(incSize > 0 ? incSize : startSize, MIN_BLOCK_SIZE);
    return CreateHeapElement(max(startSize, MIN_BLOCK_SIZE));
}

void CThread::CloseHeap()
{
    while (pHeMain != NULL)
    {
        THeapElement* pHe = pHeMain;
        pHeMain = pHe->m_pHeNext;
        HeapDestroy(pHe->m_hHeap);
        delete pHe;
    }

    blockSize = 0;
}

LPVOID CThread::AllocMem(int nBytes, bool bLocalHeap)
{
    if (nBytes <= 0) return NULL;

    LPVOID p = NULL;

    if (bLocalHeap && pHeMain != NULL && nBytes <= blockSize / 2)
    {
        for (THeapElement* pHe = pHeMain; pHe != NULL; pHe = pHe->m_pHeNext)
        {
            p = pHe->Alloc(nBytes);
            if (p != NULL) return p;

            HeapCompact(pHe->m_hHeap, 0);
            p = pHe->Alloc(nBytes);
            if (p != NULL) return p;
        }

        if (CreateHeapElement(blockSize))
        {
            p = pHeMain->Alloc(nBytes);
            if (p != NULL) return p;
        }
    }

    if (p == NULL)
    {
        p = malloc(nBytes);
        if (p == NULL) ThrowMessage(LocalAfxString(IDS_OUT_OF_MEM));
    }

    return p;
}

void CThread::FreeMem(LPVOID p)
{
    if (p == NULL) return;

    for (THeapElement* pHe = pHeMain; pHe != NULL; pHe = pHe->m_pHeNext)
        if (p >= pHe->m_pStart && p <= pHe->m_pEnd)
        {
            HeapFree(pHe->m_hHeap, 0, p);
            return;
        }

    free(p);
}
