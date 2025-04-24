#include "StdAfx.h"

#include "AFLibRow.h"

#include "AFLibRecordsetAF.h"

using namespace AFLibDB;
using namespace AFLib;

CRow::CRow(const CRecordsetAF& rs, int nRow) : m_pRs(NULL), m_nRow(0), m_nColumns(0)
{
    ASSERT(nRow >= 0 && nRow < rs.GetRowCount());

    m_pRs = const_cast<CRecordsetAF*>(&rs);
    m_nRow = nRow;
    m_nColumns = m_pRs->GetColumnCount();

    if (m_pRs->GetRecordsetParam().m_bSmartRows) m_pRs->m_rows.Include(this);
}

CRow::~CRow()
{
    Close();
}

void CRow::TestValid() const
{
    ASSERT(m_pRs != NULL);
    ASSERT(m_nRow >= 0 && m_nRow < m_pRs->GetRowCount());
}

void CRow::Close()
{
    if (!IsValid()) return;
    if (m_pRs->GetRecordsetParam().m_bSmartRows) m_pRs->m_rows.Remove(this);
    m_pRs = NULL;
    m_nRow = 0;
    m_nColumns = 0;
}

const CString& CRow::GetQuery() const
{
    TestValid();
    return m_pRs->GetQuery();
}

const CString& CRow::GetColumnName(int nCol) const
{
    TestValid();
    return m_pRs->GetColumnName(nCol);
}

bool CRow::ColumnExists(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->ColumnExists(dbf);
}

bool CRow::ColumnExists(LPCTSTR name) const
{
    TestValid();
    return m_pRs->ColumnExists(name);
}

TDBFieldType CRow::GetColumnType(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetType();
}

TDBFieldType CRow::GetColumnType(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetType();
}

TDBFieldType CRow::GetColumnType(int nCol) const
{
    TestValid();
    return m_pRs->GetColumnType(nCol);
}

bool CRow::GetBool(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetBool(m_nRow);
}

bool CRow::GetBool(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetBool(m_nRow);
}

bool CRow::GetBool(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? false : m_pRs->GetColumn(nCol).GetBool(m_nRow);
}

int CRow::GetInt(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetInt(m_nRow);
}

int CRow::GetInt(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetInt(m_nRow);
}

int CRow::GetInt(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? 0 : m_pRs->GetColumn(nCol).GetInt(m_nRow);
}

INT64 CRow::GetInt64(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetInt64(m_nRow);
}

INT64 CRow::GetInt64(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetInt64(m_nRow);
}

INT64 CRow::GetInt64(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? 0 : m_pRs->GetColumn(nCol).GetInt64(m_nRow);
}

double CRow::GetDouble(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetDouble(m_nRow);
}

double CRow::GetDouble(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetDouble(m_nRow);
}

double CRow::GetDouble(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? 0. : m_pRs->GetColumn(nCol).GetDouble(m_nRow);
}

COleDateTime CRow::GetDateTime(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetDateTime(m_nRow);
}

COleDateTime CRow::GetDateTime(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetDateTime(m_nRow);
}

COleDateTime CRow::GetDateTime(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? odtZero : m_pRs->GetColumn(nCol).GetDateTime(m_nRow);
}

void CRow::GetBinary(const TDBField& dbf, CByteArray& data) const
{
    TestValid();
    m_pRs->GetColumn(dbf).GetBinary(m_nRow, data);
}

void CRow::GetBinary(LPCTSTR name, CByteArray& data) const
{
    TestValid();
    m_pRs->GetColumn(name).GetBinary(m_nRow, data);
}

void CRow::GetBinary(int nCol, CByteArray& data) const
{
    TestValid();
    if (nCol >= GetColumnCount())
        data.RemoveAll();
    else
        m_pRs->GetColumn(nCol).GetBinary(m_nRow, data);
}

CString CRow::GetString(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetString(m_nRow);
}

CString CRow::GetString(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetString(m_nRow);
}

CString CRow::GetString(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? strEmpty : m_pRs->GetColumn(nCol).GetString(m_nRow);
}

CStringA CRow::GetStringA(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetStringA(m_nRow);
}

CStringA CRow::GetStringA(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetStringA(m_nRow);
}

CStringA CRow::GetStringA(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? strEmptyA : m_pRs->GetColumn(nCol).GetStringA(m_nRow);
}

CStringW CRow::GetStringW(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetStringW(m_nRow);
}

CStringW CRow::GetStringW(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetStringW(m_nRow);
}

CStringW CRow::GetStringW(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? strEmptyW : m_pRs->GetColumn(nCol).GetStringW(m_nRow);
}

CString CRow::GetFmtString(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetFmtString(m_nRow);
}

CString CRow::GetFmtString(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetFmtString(m_nRow);
}

CString CRow::GetFmtString(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? strNull : m_pRs->GetColumn(nCol).GetFmtString(m_nRow);
}

int CRow::GetRawString(const TDBField& dbf, LPSTR buffer, int bufLen) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetRawString(m_nRow, buffer, bufLen);
}

int CRow::GetRawString(LPCTSTR name, LPSTR buffer, int bufLen) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetRawString(m_nRow, buffer, bufLen);
}

int CRow::GetRawString(int nCol, LPSTR buffer, int bufLen) const
{
    TestValid();
    return nCol >= GetColumnCount() ? -1 : m_pRs->GetColumn(nCol).GetRawString(m_nRow, buffer, bufLen);
}

int CRow::GetRawString(const TDBField& dbf, LPWSTR buffer, int bufLen) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).GetRawString(m_nRow, buffer, bufLen);
}

int CRow::GetRawString(LPCTSTR name, LPWSTR buffer, int bufLen) const
{
    TestValid();
    return m_pRs->GetColumn(name).GetRawString(m_nRow, buffer, bufLen);
}

int CRow::GetRawString(int nCol, LPWSTR buffer, int bufLen) const
{
    TestValid();
    return nCol >= GetColumnCount() ? -1 : m_pRs->GetColumn(nCol).GetRawString(m_nRow, buffer, bufLen);
}

bool CRow::IsNull(const TDBField& dbf) const
{
    TestValid();
    return m_pRs->GetColumn(dbf).IsNull(m_nRow);
}

bool CRow::IsNull(LPCTSTR name) const
{
    TestValid();
    return m_pRs->GetColumn(name).IsNull(m_nRow);
}

bool CRow::IsNull(int nCol) const
{
    TestValid();
    return nCol >= GetColumnCount() ? true : m_pRs->GetColumn(nCol).IsNull(m_nRow);
}
