#pragma once

#include "AFLibDBGlobal.h"

/////////////////////////////////////////////////////////////////////////////
// CRecordsetDef
// Recordset Definition: a bunch of column names and types

namespace AFLibDB {
class CRecordsetAF;
class CDatabaseAF;
class CDatabasePool;

class AFLIB CRecordsetDef
{
private:
    // definition of a single column
    struct TColDef
    {
        TDBFieldType m_dft;   // field type
        CString m_fieldName;  // filed title
    };

    // array of column definitions
    CArray<TColDef> m_columns;

    // helper for constructors
    void ReadFromRs(const CRecordsetAF& rs);

public:
    CRecordsetDef();
    CRecordsetDef(const CRecordsetDef& rsd);
    CRecordsetDef(CDatabaseAF* pDB, LPCTSTR table, LPCTSTR fields = NULL);
    CRecordsetDef(CDatabasePool* pDP, LPCTSTR table, LPCTSTR fields = NULL);
    explicit CRecordsetDef(const CRecordsetAF& rs);

    CRecordsetDef& operator=(const CRecordsetDef& rsd);
    bool operator==(const CRecordsetDef& rsd) const;

    bool operator!=(const CRecordsetDef& rsd) const
    {
        return !(*this == rsd);
    }

    // insert column definition at given index
    void InsertColumn(int index, TDBFieldType dft, const CString& fieldName);

    // add column definition at the end of the list
    void AddColumn(TDBFieldType dft, const CString& fieldName);

    // delete column at given index
    void DeleteColumn(int index);

    // find column index by name
    int FindColumn(LPCTSTR fieldName);

    // get number of columns
    int GetColumnCount() const
    {
        return m_columns.GetSize();
    }

    // no columns?
    bool IsEmpty() const
    {
        return GetColumnCount() <= 0;
    }

    // get column type
    TDBFieldType GetColumnType(int index) const
    {
        return m_columns[index].m_dft;
    }

    // get column type
    const CString& GetColumnName(int index) const
    {
        return m_columns[index].m_fieldName;
    }
};
}  // namespace AFLibDB
