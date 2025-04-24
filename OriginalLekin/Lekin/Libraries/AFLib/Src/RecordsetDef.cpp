#include "StdAfx.h"

#include "AFLibRecordsetDef.h"

#include "AFLibDatabaseAF.h"
#include "AFLibRecordsetAF.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibDB;
using AFLibPrivate::LocalAfxString;

CRecordsetDef::CRecordsetDef()
{}

CRecordsetDef::CRecordsetDef(const CRecordsetDef& rsd)
{
    m_columns.Copy(rsd.m_columns);
}

CRecordsetDef::CRecordsetDef(CDatabaseAF* pDB, LPCTSTR table, LPCTSTR fields)
{
    CRecordsetAF rs;
    rs.LoadTableDef(pDB, table, fields);
    ReadFromRs(rs);
}

CRecordsetDef::CRecordsetDef(CDatabasePool* pDP, LPCTSTR table, LPCTSTR fields)
{
    CRecordsetAF rs;
    rs.LoadTableDef(pDP, table, fields);
    ReadFromRs(rs);
}

CRecordsetDef::CRecordsetDef(const CRecordsetAF& rs)
{
    ReadFromRs(rs);
}

CRecordsetDef& CRecordsetDef::operator=(const CRecordsetDef& rsd)
{
    if (&rsd == this) return *this;
    m_columns.Copy(rsd.m_columns);
    return *this;
}

bool CRecordsetDef::operator==(const CRecordsetDef& rsd) const
{
    if (GetColumnCount() != rsd.GetColumnCount()) return false;

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        if (GetColumnType(i) != rsd.GetColumnType(i)) return false;
        if (GetColumnName(i) != rsd.GetColumnName(i)) return false;
    }

    return true;
}

void CRecordsetDef::ReadFromRs(const CRecordsetAF& rs)
{
    for (int i = 0; i < rs.GetColumnCount(); ++i)
    {
        TColDef cd = {rs.GetColumnType(i), rs.GetColumnName(i)};
        m_columns.Add(cd);
    }
}

void CRecordsetDef::InsertColumn(int index, TDBFieldType dbf, const CString& fieldName)
{
    ASSERT(index >= 0 && index <= GetColumnCount());
    CDatabaseAF::CheckFieldName(fieldName);

    if (FindColumn(fieldName) >= 0) ThrowMessage(LocalAfxString(IDS_DUP_FIELD_NAME, fieldName));

    TColDef cd = {dbf, fieldName};
    m_columns.InsertAt(index, cd);
}

void CRecordsetDef::AddColumn(TDBFieldType dbf, const CString& fieldName)
{
    InsertColumn(GetColumnCount(), dbf, fieldName);
}

void CRecordsetDef::DeleteColumn(int index)
{
    ASSERT(index >= 0 && index < GetColumnCount());
    m_columns.RemoveAt(index);
}

int CRecordsetDef::FindColumn(LPCTSTR fieldName)
{
    int i = GetColumnCount();
    while (--i >= 0)
        if (GetColumnName(i) == fieldName) break;
    return i;
}
