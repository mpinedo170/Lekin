#include "StdAfx.h"

#include "AFLibRowNC.h"

#include "AFLibRecordsetAF.h"
#include "AFLibStrings.h"

using namespace AFLibDB;
using namespace AFLib;

CRowNC::CRowNC(CRecordsetAF& rs, int index) : super(rs, index == newRow ? rs.AddRow() : index)
{}

void CRowNC::Kill()
{
    TestValid();
    m_pRs->DeleteRow(m_nRow);
}

void CRowNC::SetBool(const TDBField& dbf, bool data)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetBool(m_nRow, data);
}

void CRowNC::SetBool(LPCTSTR name, bool data)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetBool(m_nRow, data);
}

void CRowNC::SetBool(int nCol, bool data)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetBool(m_nRow, data);
}

void CRowNC::SetInt(const TDBField& dbf, INT64 data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetInt(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetInt(LPCTSTR name, INT64 data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetInt(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetInt(int nCol, INT64 data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetInt(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetDouble(const TDBField& dbf, double data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetDouble(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetDouble(LPCTSTR name, double data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetDouble(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetDouble(int nCol, double data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetDouble(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetDateTime(const TDBField& dbf, const COleDateTime& data)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetDateTime(m_nRow, data);
}

void CRowNC::SetDateTime(LPCTSTR name, const COleDateTime& data)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetDateTime(m_nRow, data);
}

void CRowNC::SetDateTime(int nCol, const COleDateTime& data)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetDateTime(m_nRow, data);
}

void CRowNC::SetBinary(const TDBField& dbf, const CByteArray& data)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetBinary(m_nRow, data);
}

void CRowNC::SetBinary(LPCTSTR name, const CByteArray& data)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetBinary(m_nRow, data);
}

void CRowNC::SetBinary(int nCol, const CByteArray& data)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetBinary(m_nRow, data);
}

void CRowNC::SetString(const TDBField& dbf, LPCTSTR data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetString(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetString(LPCTSTR name, LPCTSTR data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetString(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetString(int nCol, LPCTSTR data, bool bThrowOnBadData)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetString(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetStringNull(const TDBField& dbf, LPCTSTR data, bool bThrowOnBadData)
{
    TestValid();
    CColumn& column = m_pRs->GetColumnNC(dbf);

    if (IsEmptyStr(data))
        column.SetNull(m_nRow);
    else
        column.SetString(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetStringNull(LPCTSTR name, LPCTSTR data, bool bThrowOnBadData)
{
    TestValid();
    CColumn& column = m_pRs->GetColumnNC(name);

    if (IsEmptyStr(data))
        column.SetNull(m_nRow);
    else
        column.SetString(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetStringNull(int nCol, LPCTSTR data, bool bThrowOnBadData)
{
    TestValid();
    CColumn& column = m_pRs->GetColumnNC(nCol);

    if (IsEmptyStr(data))
        column.SetNull(m_nRow);
    else
        column.SetString(m_nRow, data, bThrowOnBadData);
}

void CRowNC::SetStringA(const TDBField& dbf, LPCSTR data)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetStringA(m_nRow, data);
}

void CRowNC::SetStringA(LPCTSTR name, LPCSTR data)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetStringA(m_nRow, data);
}

void CRowNC::SetStringA(int nCol, LPCSTR data)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetStringA(m_nRow, data);
}

void CRowNC::SetStringW(const TDBField& dbf, LPCWSTR data)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetStringW(m_nRow, data);
}

void CRowNC::SetStringW(LPCTSTR name, LPCWSTR data)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetStringW(m_nRow, data);
}

void CRowNC::SetStringW(int nCol, LPCWSTR data)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetStringW(m_nRow, data);
}

void CRowNC::SetNull(const TDBField& dbf)
{
    TestValid();
    m_pRs->GetColumnNC(dbf).SetNull(m_nRow);
}

void CRowNC::SetNull(LPCTSTR name)
{
    TestValid();
    m_pRs->GetColumnNC(name).SetNull(m_nRow);
}

void CRowNC::SetNull(int nCol)
{
    TestValid();
    m_pRs->GetColumnNC(nCol).SetNull(m_nRow);
}
