#pragma once

#include "AFLibArrays.h"
#include "AFLibThread.h"

/////////////////////////////////////////////////////////////////////////////
// TThreadSafeStatic
// Template class to establish thread-safe local storage, if the user
//   does not want to call the T constructor in every call to a local function.
// Class T must allow a default constructor or a constructor with one argument.
// Should be used as follows.
//   static TThreadSafeStatic<TYPE> tss;
//   ...
//   void f()
//   { SAFE_STATIC_INIT(tss, T, var, init);
// ** or **
//     SAFE_STATIC(tss, T, var);
//     // var can be safely used within f()
//
// Another way is:
//   static TThreadSafeStatic<TYPE> tss;
//   __declspec(thread) T* pT;
//   ...
//   void f()
//   { SAFE_STATIC_PTR(tss, T, pT);
//     // pT can be safely used within f()
//     // But do not call any other functions that use pT!
//
// Also, tss.Cleanup() should be called once in a while.

namespace AFLibThread {
template <class T>
class TThreadSafeStatic
{
private:
    DEFINE_COPY_AND_ASSIGN(TThreadSafeStatic);

    // data for every thread
    struct TThreadData
    {
        CThread m_thread;                // thread ID and handle
        AFLib::CSmartArray<T> m_arrPtr;  // all pointers for this thread

        explicit TThreadData(DWORD idThread) : m_thread(idThread)
        {
            m_arrPtr.SetSize(0, 64);
        }
        ~TThreadData()
        {
            m_arrPtr.DestroyAll();
        }

        UINT GetGLKey() const
        {
            return m_thread.GetID();
        }
    };

    // data for all threads
    AFLib::CSortedUintArray<TThreadData> m_data;

    // synchronization object
    CCriticalSection m_sync;

public:
    TThreadSafeStatic()
    {
        m_data.SetSize(0, 16);
    }

    ~TThreadSafeStatic()
    {
        m_data.DestroyAll();
    }

    // Get the thread-safe pointer; pT must be declspec(thread)!
    void Process(T*& pT)
    {
        CSingleLock lock(&m_sync, true);
        DWORD idThread = GetCurrentThreadId();
        int index = m_data.BSearch(idThread);
        TThreadData* pTd = NULL;

        if (index < 0)
        {
            pTd = new TThreadData(idThread);
            m_data.Insert(pTd);
        }
        else
            pTd = m_data[index];

        pTd->m_arrPtr.Add(pT);
    }

    // Remove data for finished threads
    void Cleanup()
    {
        CSingleLock lock(&m_sync, true);

        for (int i = m_data.GetSize(); --i >= 0;)
        {
            TThreadData* pTd = m_data[i];
            if (pTd->m_thread.IsFinished()) m_data.DestroyAt(i);
        }
    }
};

AFLIB extern TThreadSafeStatic<AFLibIO::TStringBufferA> StaticStringBufferStorageA;
AFLIB extern TThreadSafeStatic<AFLibIO::TStringBufferW> StaticStringBufferStorageW;
}  // namespace AFLibThread

#define SAFE_STATIC(tss, T, var)         \
    static __declspec(thread) T* p##var; \
    if (p##var == NULL)                  \
    {                                    \
        p##var = new T;                  \
        tss.Process(p##var);             \
    }                                    \
    T& var = *p##var;

#define SAFE_STATIC_INIT(tss, T, var, init) \
    static __declspec(thread) T* p##var;    \
    if (p##var == NULL)                     \
    {                                       \
        p##var = new T(init);               \
        tss.Process(p##var);                \
    }                                       \
    T& var = *p##var;

#define SAFE_STATIC_PTR(tss, T, pVar) \
    if (pVar == NULL)                 \
    {                                 \
        pVar = new T;                 \
        tss.Process(pVar);            \
    }

// SAFE_STATIC_SB is an easy way to create a thread-safe local String Buffer

#define SAFE_STATIC_SB_A(var)                                                           \
    SAFE_STATIC(AFLibThread::StaticStringBufferStorageA, AFLibIO::TStringBufferA, var); \
    var.Empty();

#define SAFE_STATIC_SB_W(var)                                                          \
    SAFE_STATIC(AFLibThread::StaticStringBufferStorageW, AFLibIO::TStringBufferW, var) \
    var.Empty();

#ifdef _UNICODE
#define SAFE_STATIC_SB(var) SAFE_STATIC_SB_W(var)
#define StaticStringBufferStorage StaticStringBufferStorageW
#else
#define SAFE_STATIC_SB(var) SAFE_STATIC_SB_A(var)
#define StaticStringBufferStorage StaticStringBufferStorageA
#endif
