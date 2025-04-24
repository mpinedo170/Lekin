#pragma once

#include "AFLibDBGlobal.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CRow
// A Row for CRecordsetAF

namespace AFLibDB {
class CRecordsetAF;
class TDBField;

class AFLIB CRow
{
private:
    friend CRecordsetAF;
    DEFINE_COPY_AND_ASSIGN(CRow);

protected:
    CRecordsetAF* m_pRs;  // host recordset
    int m_nRow;           // index in the recordset
    int m_nColumns;       // cached number of columns

    // check if the row is still valid
    void TestValid() const;

public:
    CRow(const CRecordsetAF& rs, int nRow);
    ~CRow();

    // close the row
    void Close();

    // get query used to fetch the host recordset
    const CString& GetQuery() const;

    // get number of columns
    int GetColumnCount() const
    {
        return m_nColumns;
    }

    // get column name by index
    const CString& GetColumnName(int nCol) const;

    // does column exist in recordset?
    bool ColumnExists(const TDBField& dbf) const;
    bool ColumnExists(LPCTSTR name) const;

    // get column type
    TDBFieldType GetColumnType(const TDBField& dbf) const;
    TDBFieldType GetColumnType(LPCTSTR name) const;
    TDBFieldType GetColumnType(int nCol) const;

    // get boolean data
    bool GetBool(const TDBField& dbf) const;
    bool GetBool(LPCTSTR name) const;
    bool GetBool(int nCol) const;

    // get integer data (no conversion from string)
    int GetInt(const TDBField& dbf) const;
    int GetInt(LPCTSTR name) const;
    int GetInt(int nCol) const;

    // get 64-bit integer data (no conversion from string)
    INT64 GetInt64(const TDBField& dbf) const;
    INT64 GetInt64(LPCTSTR name) const;
    INT64 GetInt64(int nCol) const;

    // get floating point data (no conversion from string)
    double GetDouble(const TDBField& dbf) const;
    double GetDouble(LPCTSTR name) const;
    double GetDouble(int nCol) const;

    // get date data
    COleDateTime GetDateTime(const TDBField& dbf) const;
    COleDateTime GetDateTime(LPCTSTR name) const;
    COleDateTime GetDateTime(int nCol) const;

    // get binary data
    void GetBinary(const TDBField& dbf, CByteArray& data) const;
    void GetBinary(LPCTSTR name, CByteArray& data) const;
    void GetBinary(int nCol, CByteArray& data) const;

    // get any data
    CString GetString(const TDBField& dbf) const;
    CString GetString(LPCTSTR name) const;
    CString GetString(int nCol) const;

    // get ascii string (only for char fields)
    CStringA GetStringA(const TDBField& dbf) const;
    CStringA GetStringA(LPCTSTR name) const;
    CStringA GetStringA(int nCol) const;

    // get unicode string (only for char fields)
    CStringW GetStringW(const TDBField& dbf) const;
    CStringW GetStringW(LPCTSTR name) const;
    CStringW GetStringW(int nCol) const;

    // get formatted field, or null
    CString GetFmtString(const TDBField& dbf) const;
    CString GetFmtString(LPCTSTR name) const;
    CString GetFmtString(int nCol) const;

    // get ascii string directly into buffer, without allocating a CString
    int GetRawString(const TDBField& dbf, LPSTR buffer, int bufLen) const;
    int GetRawString(LPCTSTR name, LPSTR buffer, int bufLen) const;
    int GetRawString(int nCol, LPSTR buffer, int bufLen) const;

    // get unicode string directly into buffer, without allocating a CString
    int GetRawString(const TDBField& dbf, LPWSTR buffer, int bufLen) const;
    int GetRawString(LPCTSTR name, LPWSTR buffer, int bufLen) const;
    int GetRawString(int nCol, LPWSTR buffer, int bufLen) const;

    // is data null?
    bool IsNull(const TDBField& dbf) const;
    bool IsNull(LPCTSTR name) const;
    bool IsNull(int nCol) const;

    // row hasn't been closed yet
    bool IsValid() const
    {
        return m_pRs != NULL;
    }

    // index of the row in the recordset
    int GetRowIndex() const
    {
        return m_nRow;
    }
};
}  // namespace AFLibDB
