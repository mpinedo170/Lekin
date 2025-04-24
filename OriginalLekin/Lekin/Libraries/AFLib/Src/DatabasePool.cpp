#include "StdAfx.h"

#include "AFLibDatabasePool.h"

#include "AFLibDatabaseAF.h"
#include "AFLibGlobal.h"
#include "LibBcp.h"

using namespace AFLibDB;
using namespace AFLib;
using namespace AFLibThread;

namespace {
int compareThread(LPCVOID p1, LPCVOID p2)
{
    DWORD a1 = *reinterpret_cast<LPCDWORD>(p1);
    const CThread* pThread = reinterpret_cast<const CThread*>(p2);
    DWORD a2 = pThread->GetID();
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}
}  // namespace

CDatabasePool::CDatabasePool()
{
    m_bBcpEnabled = false;
    m_dwQueryTimeout = 0;
    m_data.SetSize(0, 64);
    m_pSync = new CCriticalSection;
}

CDatabasePool::~CDatabasePool()
{
    Close();
    delete m_pSync;
}

bool CDatabasePool::EnableBcp()
{
    ASSERT(!IsOpen());
    m_bBcpEnabled = AFLibPrivate::TLibBcp::m_a.InitLibrary();
    return m_bBcpEnabled;
}

void CDatabasePool::SetQueryTimeout(DWORD dwSeconds)
{
    ASSERT(!IsOpen());
    m_dwQueryTimeout = dwSeconds;
}

void CDatabasePool::SetDefaultDB(LPCTSTR dbName)
{
    ASSERT(!IsOpen());
    m_dbName = dbName;
}

bool CDatabasePool::Open(LPCTSTR lpszConnect)
{
    CSingleLock lock(m_pSync, true);
    ASSERT(!IsOpen());
    ASSERT(!IsEmptyStr(lpszConnect));

    m_lpszConnect = lpszConnect;
    return ProtectedGetDB(false) != NULL;
}

bool CDatabasePool::IsOpen()
{
    CSingleLock lock(m_pSync, true);
    return !m_lpszConnect.IsEmpty();
}

int CDatabasePool::GetPoolSize()
{
    CSingleLock lock(m_pSync, true);
    return m_data.GetSize();
}

bool CDatabasePool::ProtectedOpen(CDatabaseAF* pDB, LPCTSTR lpszConnect)
{
    if (m_bBcpEnabled) pDB->EnableBcp();
    if (m_dwQueryTimeout > 0) pDB->SetQueryTimeout(m_dwQueryTimeout);

    if (!pDB->Open(NULL, false, false, lpszConnect, false)) return false;

    if (!m_dbName.IsEmpty()) pDB->Use(m_dbName);
    return true;
}

CDatabaseAF* CDatabasePool::ProtectedGetDBThrow(bool bForceReopen)
{
    CDatabaseAF* pDB = ProtectedGetDB(bForceReopen);
    if (pDB == NULL) AFLib::ThrowMessageNoDB();
    return pDB;
}

CDatabaseAF* CDatabasePool::ProtectedGetDB(bool bForceReopen)
{
    CSingleLock lock(m_pSync, true);
    DWORD idThread = GetCurrentThreadId();
    bool bFound = false;
    int index = BinarySearch(
        &idThread, m_data.GetData(), m_data.GetSize(), sizeof(TThreadData), compareThread, bFound);

    if (bFound)
    {
        CDatabaseAF* pDB = m_data[index].m_pDB;
        if (!bForceReopen) return pDB;

        delete pDB;
        m_data.RemoveAt(index);
    }

    TRACE(_T("Opening DB for ") + IntToStr(idThread) + _T(" (0x") + HexToStr(idThread, 4) + _T(")\n"));
    lock.Unlock();

    CAutoPtr<CDatabaseAF> pDB(new CDatabaseAF);
    if (!ProtectedOpen(pDB, m_lpszConnect)) return NULL;

    lock.Lock();
    TThreadData td = {CThread(idThread), pDB.Detach()};

    m_data.InsertAt(index, td);
    return td.m_pDB;
}

void CDatabasePool::Close()
{
    CSingleLock lock(m_pSync, true);

    for (int i = m_data.GetSize(); --i >= 0;) delete m_data[i].m_pDB;

    m_data.RemoveAll();
    m_lpszConnect.Empty();
}

void CDatabasePool::Cleanup()
{
    CSingleLock lock(m_pSync, true);

    for (int i = m_data.GetSize(); --i >= 0;)
    {
        TThreadData& td = m_data[i];
        if (td.m_thread.IsFinished())
        {
            delete td.m_pDB;
            m_data.RemoveAt(i);
        }
    }
}
