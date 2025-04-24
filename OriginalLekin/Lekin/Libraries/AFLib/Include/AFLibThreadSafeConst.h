#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TThreadSafeConst
// Template class to establish thread-safe "static const" local variables.
// An auto variable would call the T constructor in every call to a local function;
//   simple "static const" is not thread-safe.
// Class T must allow a constructor with one argument.
// Should be used as follows.
//   static TThreadSafeConst<TYPE> tsc;
//   ...
//   void f()
//   { SAFE_CONST(tsc, T, var, init);
//     // var can be safely used within f()

namespace AFLibThread {
template <class T>
class TThreadSafeConst
{
private:
    DEFINE_COPY_AND_ASSIGN(TThreadSafeConst);

    CCriticalSection m_sync;         // synchronization object
    AFLib::CSmartArray<T> m_arrPtr;  // pointers to allocated constants

public:
    TThreadSafeConst()
    {
        m_arrPtr.SetSize(0, 1 << 10);
    }

    ~TThreadSafeConst()
    {
        m_arrPtr.DestroyAll();
    }

    // initialize the pointer if necessary
    template <class T2>
    void Process(T*& pT, const T2& initT)
    {
        CSingleLock lock(&m_sync, true);
        if (pT != NULL) return;

        pT = new T(initT);
        m_arrPtr.Add(pT);
        return;
    }
};

AFLIB extern TThreadSafeConst<CStringA> StaticStringStorageA;
AFLIB extern TThreadSafeConst<CStringW> StaticStringStorageW;
}  // namespace AFLibThread

#define SAFE_CONST(tsc, T, var, init)              \
    static T* p##var;                              \
    if (p##var == NULL) tsc.Process(p##var, init); \
    const T& var = *p##var;

#define SAFE_CONST_STR_A(var, init) SAFE_CONST(AFLibThread::StaticStringStorageA, CStringA, var, init)
#define SAFE_CONST_STR_W(var, init) SAFE_CONST(AFLibThread::StaticStringStorageW, CStringW, var, init)

#ifdef _UNICODE
#define StaticStringStorage StaticStringStorageW
#define SAFE_CONST_STR(var, init) SAFE_CONST_STR_W(var, init)
#else
#define StaticStringStorage StaticStringStorageA
#define SAFE_CONST_STR(var, init) SAFE_CONST_STR_A(var, init)
#endif
