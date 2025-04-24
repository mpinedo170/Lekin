#include "StdAfx.h"

#include "AFLibStorage.h"

#include "AFLibGlobal.h"

using AFLibIO::TStorage;

TStorage::TStorage(LPCTSTR name) : m_file(name, true, CFile::shareDenyWrite)
{
    m_pSync = new CCriticalSection;
}

TStorage::~TStorage()
{
    delete m_pSync;
}

void TStorage::Clear(int pos)
{
    CSingleLock lock(m_pSync, true);
    if (m_file.GetLength() > pos) m_file.SetLength(pos);
}

INT64 TStorage::Write(LPCVOID pData, int size, INT64 pos)
{
    CSingleLock lock(m_pSync, true);
    INT64 posNew = m_file.GetLength();

    if (pos >= 0)
    {
        int sz = GetBlockSize(pos);
        if (sz <= size || pos + sz + sizeof(int) >= posNew) posNew = pos;
    }

    m_file.Seek(posNew, CFile::begin);
    m_file.Write(&size, sizeof(int));
    m_file.Write(pData, size);
    return posNew;
}

INT64 TStorage::Write(LPCTSTR s, INT64 pos)
{
    if (AFLib::IsEmptyStr(s)) return -1;
    return Write(s, (_tcslen(s) + 1) * sizeof(TCHAR), pos);
}

int TStorage::GetBlockSize(INT64 pos)
{
    CSingleLock lock(m_pSync, true);
    m_file.Seek(pos, CFile::begin);

    int sz = 0;
    m_file.Read(&sz, sizeof(int));
    return sz;
}

LPVOID TStorage::Read(INT64 pos)
{
    CSingleLock lock(m_pSync, true);
    int sz = GetBlockSize(pos);
    CAutoVectorPtr<BYTE> pData(new BYTE[sz]);

    m_file.Read(pData, sz);
    return pData.Detach();
}

void TStorage::Read(LPVOID pData, INT64 pos)
{
    CSingleLock lock(m_pSync, true);
    int sz = GetBlockSize(pos);
    m_file.Read(pData, sz);
}

void TStorage::Read(CString& s, INT64 pos)
{
    if (pos < 0)
    {
        s.Empty();
        return;
    }

    CSingleLock lock(m_pSync, true);
    int sz = GetBlockSize(pos);
    int szBuffer = (sz + sizeof(TCHAR) - 1) / sizeof(TCHAR);
    m_file.Read(s.GetBuffer(szBuffer), sz);
    s.ReleaseBuffer();
}
