#include "StdAfx.h"

#include "AFLibLog.h"

#include "AFLibExcBreak.h"
#include "AFLibGlobal.h"
#include "AFLibThreadSafeStatic.h"

using namespace AFLib;
using namespace AFLibIO;

TLog* TLog::m_pLogObj = NULL;
CCriticalSection TLog::m_syncObj;
TLog::TInit TLog::m_init;

/////////////////////////////////////////////////////////////////////////////
// TLog::TInit

TLog::TInit::~TInit()
{
    delete m_pLogObj;
    m_pLogObj = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// TLog

TLog::TLog(LPCTSTR path, bool bTimed, bool bSync) :
    m_bTimed(bTimed),
    m_path(IsEmptyStr(path) ? strEmpty : GetFullPath(path))
{
    m_pSync = bSync ? new CCriticalSection : NULL;
    m_bufSize = 0;
}

TLog::~TLog()
{
    Flush();
    delete m_pSync;
}

TLog& TLog::Get()
{
    if (m_pLogObj != NULL) return *m_pLogObj;
    CSingleLock lock(&m_syncObj, true);

    if (m_pLogObj == NULL)
    {
        CString path = GetExeFullPath();
        if (StringEndsWithIC(path, extExe)) path.Delete(path.GetLength() - 4, 4);
        path += extLog;
        m_pLogObj = new TLog(path, true, true);
    }
    return *m_pLogObj;
}

bool TLog::PrivateWrite(LPCTSTR line)
{
    if (m_bTimed)
    {
        m_buffer += TimeToStr(COleDateTime::GetCurrentTime());
        m_buffer += strPrefixer;
    }

    m_buffer += line;
    m_buffer += strEol;

    return m_buffer.GetLength() > m_bufSize ? Flush() : true;
}

bool TLog::Write(LPCTSTR line)
{
    if (m_pSync == NULL) return PrivateWrite(line);

    CSingleLock lock(m_pSync, true);
    return PrivateWrite(line);
}

bool TLog::PrivateFlush()
{
    bool bOk = false;

    try
    {
        CFile Fout(
            m_path, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyNone);
        Fout.SeekToEnd();
        m_buffer.Write(Fout);
        bOk = true;
    }
    catch (AFLibThread::CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    m_buffer.Empty();
    return bOk;
}

bool TLog::Flush()
{
    if (m_path.IsEmpty()) return true;

    if (m_pSync == NULL) return PrivateFlush();

    CSingleLock lock(m_pSync, true);
    return PrivateFlush();
}

void TLog::FlushIntoHelper(TStringBuffer& sbDest)
{
    sbDest += m_buffer;
    m_buffer.Empty();
}

void TLog::FlushInto(TLog& log2)
{
    if (m_pSync != NULL && log2.m_pSync != NULL)
    {  // both logs are sync-ed: need a temp buffer to avoid deadlock
        SAFE_STATIC_SB(sb);

        CSingleLock lock1(m_pSync, true);
        FlushIntoHelper(sb);
        lock1.Unlock();

        CSingleLock lock2(log2.m_pSync, true);
        log2.m_buffer += sb;
        return;
    }

    CCriticalSection* pSync = m_pSync != NULL ? m_pSync : log2.m_pSync;
    if (pSync != NULL)
    {
        CSingleLock lock(pSync, true);
        FlushIntoHelper(log2.m_buffer);
    }
    else
        FlushIntoHelper(log2.m_buffer);
}

void TLog::SetBufferSize(int bufSize)
{
    bufSize = max(bufSize, 0);

    if (m_pSync == NULL)
        m_bufSize = bufSize;
    else
    {
        CSingleLock lock(m_pSync, true);
        m_bufSize = bufSize;
    }
}

void TLog::PrivateClear()
{
    if (!m_path.IsEmpty()) DeleteFile(m_path);
    m_buffer.Empty();
}

void TLog::Clear()
{
    if (m_pSync == NULL)
        PrivateClear();
    else
    {
        CSingleLock lock(m_pSync, true);
        PrivateClear();
    }
}
