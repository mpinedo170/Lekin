#pragma once

#include "AFLibDefine.h"
#include "AFLibTestMaps.h"

class CCriticalSection;

/////////////////////////////////////////////////////////////////////////////
// CThread
// Breakable thread class

namespace AFLibThread {
class AFLIB CThread
{
private:
    DWORD m_idThread;  // thread ID
    HANDLE m_hThread;  // thread handle

    // class for automatic cleanup
    struct TInit
    {
        ~TInit();
    };

    // static object whose destructor clears m_mapBreak
    static TInit m_init;

protected:
    // map from thread ID to pointer to its "break" flag
    static AFLib::CPointerMap<int, int, int> m_mapBreak;

    // map of break messages
    static AFLib::CSmartMap<DWORD, DWORD, CString, LPCTSTR> m_mapBrkMsg;

    // synchronisation object
    static CCriticalSection m_sync;

    // private constructor
    CThread(DWORD idThread, HANDLE hThread);

    // break a thread
    void PrivateStop(LPCTSTR message, bool bWait) const;

    // duplicate a handle
    static HANDLE Duplicate(HANDLE hThread);

public:
    // a var of this class prevents current thread from breaking
    class AFLIB CBreakLock
    {
    private:
        DEFINE_COPY_AND_ASSIGN(CBreakLock);

        bool m_bLocked;  // lock is active

    public:
        CBreakLock();
        ~CBreakLock();

        // re-lock, if was unlocked
        void Lock();

        // unlock early (before the destructor)
        void Unlock();
    };

    CThread();
    CThread(const CThread& thread);
    explicit CThread(DWORD idThread, DWORD dwDesiredAccess = THREAD_ALL_ACCESS);
    ~CThread();

    CThread& operator=(const CThread& thread);

    // clear the class, close the handle
    void Clear();

    // for API calls, automatically converts CThread to HANDLE
    operator HANDLE() const
    {
        return m_hThread;
    }

    // get thread ID
    DWORD GetID() const
    {
        return m_idThread;
    }

    // is valid thread?
    bool IsValid() const
    {
        return m_hThread != NULL;
    }

    // starts thread through AfxBeginThread
    static CThread Start(AFX_THREADPROC pfnThreadProc,
        LPVOID pParam,
        int nPriority = THREAD_PRIORITY_NORMAL,
        UINT nStackSize = 0,
        DWORD dwCreateFlags = 0,
        LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

    // is anyone trying to stop me?
    static bool IsBroken();

    // can I break now? (no break locks?)
    static bool CanBreak();

    // if IsBroken(), throw exception!
    static void Break();

    // sleep, checking for the "broken" condition every "inc" milliseconds
    static void SmartSleep(int msec, int inc = 10);

    // wait for single object, checking for the "broken" condition every "inc" milliseconds
    static void SmartWait(HANDLE h, int inc = 10);

    // stop the thread, cancel its SQL statement, then wait till it stops;
    // thread will stop when it calls Break()
    void Stop(LPCTSTR message = NULL) const;

    // try to stop the thread; do not wait till it stops
    void StopNoWait(LPCTSTR message = NULL) const;

    // terminate the thread (usually a really bad idea!)
    void Terminate(DWORD dwExitCode = -1);

    // is this thread "current"?
    bool IsCurrent() const;

    // check if the thread has finished
    bool IsFinished() const;

    // wait till the thread finishes; if bAllowBreak, allows breaking the CURRENT thread
    void WaitFor(bool bAllowBreak = false) const;

    // suspend thread
    void Suspend() const;

    // resume thread
    void Resume() const;

    // open heap for the current thread
    static bool OpenHeap(int startSize, int incSize = 0);

    // close heap for the current thread
    static void CloseHeap();

    // allocate memory in the local heap; bLocalHeap is false, just do malloc
    static LPVOID AllocMem(int nBytes, bool bLocalHeap = true);

    // free memory allocated in the local heap
    static void FreeMem(LPVOID p);
};
}  // namespace AFLibThread
