#pragma once

#include "AFLibCollections.h"
#include "AFLibColumn.h"
#include "AFLibRecordsetParam.h"
#include "AFLibTestMaps.h"

namespace AFLibIO {
class TStringWriter;
}

namespace AFLibGui {
class CDlgRun;
}

namespace AFLibPrivate {
class TSQLStatement;
}

/////////////////////////////////////////////////////////////////////////////
// CRecordsetAF
// Light ODBC Recordset with BCP

namespace AFLibDB {
class CDatabaseAF;
class CDatabasePool;
class CColumn;
class CRow;
class CRowNC;
class CRecordsetDef;
class TDBField;

class AFLIB CRecordsetAF
{
private:
    friend AFLibPrivate::TSQLStatement;
    DEFINE_COPY_AND_ASSIGN(CRecordsetAF);

    // columns
    AFLib::CStrCollection<CColumn> m_columns;

    // "attached" rows (used only if smart rows are on)
    AFLib::CPtrTestMap<CRow> m_rows;

    // fast access to columns through TDBField
    CWordArray m_arrField;

    // recordset parameters
    TRecordsetParam m_param;

    // current capacity
    int m_capacity;

    // number of rows
    int m_nRows;

    // adjust "attached" rows if smart rows are used
    void ProcessAttachedRows(int nRow, int count);

    // helper for ProcessAttachedRows()
    static inline void ProcessRow(CRow* pRow, int nRow, int count);

    // mini-close
    void PrivateClose();

private:
    // check if row count was messaged by a query (helper for Open())
    int GetAFLibCount(AFLibPrivate::TSQLStatement& stm);

    // adds column to the list, processes its name
    void AddColumnHelper(CColumn* pColumn);

    // add column
    void AddColumn(LPCTSTR fieldName, int rawODBCType, bool bNullable, int colSize);

    // build columns with no data (helper for Open())
    void PrepareColumns(AFLibPrivate::TSQLStatement& stm);

    // bind columns (helper for Open())
    int BindColumns(AFLibPrivate::TSQLStatement& stm);

    // bind columns (helper for BulkInsert())
    void BindColumnsForBCP(AFLibPrivate::TSQLStatement& stm);

    // bulk insert by SQL (helper for BulkInsert())
    void BulkInsertBySQL(AFLibPrivate::TSQLStatement& stm, LPCTSTR table) const;

    // bulk insert by ODBC (helper for BulkInsert())
    void BulkInsertByODBC(AFLibPrivate::TSQLStatement& stm) const;

    // bulk insert by BCP (helper for BulkInsert())
    void BulkInsertByBCP(AFLibPrivate::TSQLStatement& stm, LPCTSTR table) const;

    // set buffers in unassigned columns to NULL or zero (helper for BulkInsert())
    static void NullifyUnassignedColumns(AFLibPrivate::TSQLStatement& stm);

    friend CRow;
    friend CRowNC;

protected:
    // gets filled in Open()
    CString m_query;

    // must create recordset of the same type
    virtual CRecordsetAF* CreateBulkInsertClone() const;

    // can be overridden to create new column types
    virtual CColumn* CreateColumn();

    // get column (non-const) by field ID
    CColumn& GetColumnNC(const TDBField& dbf);

    // get column (non-const) by name
    CColumn& GetColumnNC(LPCTSTR name)
    {
        ASSERT(ColumnExists(name));
        return *m_columns.FindKey(name);
    }

    // get column (non-const) by index
    CColumn& GetColumnNC(int nCol)
    {
        return *m_columns[nCol];
    }

public:
    // progress bar dialog
    AFLibGui::CDlgRun* m_pDlgRun;

    CRecordsetAF();
    virtual ~CRecordsetAF();

    // open the recordset; call Close() before trying to call Open() again!
    virtual void Open(CDatabaseAF* pDB, LPCTSTR query, int maxRows, AFLibIO::TStringBuffer& logMsg);
    void Open(CDatabaseAF* pDB, LPCTSTR query, int maxRows = INT_MAX);
    void Open(CDatabasePool* pDP, LPCTSTR query, int maxRows, AFLibIO::TStringBuffer& logMsg);
    void Open(CDatabasePool* pDP, LPCTSTR query, int maxRows = INT_MAX);

    // load table definition
    void LoadTableDef(CDatabaseAF* pDB, LPCTSTR table, LPCTSTR fields = NULL);
    void LoadTableDef(CDatabasePool* pDP, LPCTSTR table, LPCTSTR fields = NULL);

    // load all data from table
    void LoadTableData(CDatabaseAF* pDB, LPCTSTR table, int maxRows = INT_MAX);
    void LoadTableData(CDatabasePool* pDP, LPCTSTR table, int maxRows = INT_MAX);

    // copy all data or definition only
    virtual void Copy(const CRecordsetAF& recordset, int maxRows = INT_MAX, LPCTSTR fields = NULL);

    // build definition manually, then create recordset
    virtual void Build(const CRecordsetDef& rsd);

    // clear all data
    virtual void Close();

    // set recordset parameters
    void SetRecordsetParam(const TRecordsetParam& param);

    // get recordset parameters
    const TRecordsetParam& GetRecordsetParam() const
    {
        return m_param;
    }

    // for BulkInsert: how to insert?
    enum TBulkInsertMethod
    {
        bimSQL,
        bimODBC,
        bimBCP,
        bimSmart
    };

    // insert data into the database through through SQL, BulkOperations, or BCP
    void BulkInsert(CDatabaseAF* pDB, LPCTSTR table, TBulkInsertMethod bim = bimSmart) const;
    void BulkInsert(CDatabasePool* pDP, LPCTSTR table, TBulkInsertMethod bim = bimSmart) const;

    // get query that was used to build recordset
    const CString& GetQuery() const
    {
        return m_query;
    }

    // get number of columns
    int GetColumnCount() const
    {
        return m_columns.GetSize();
    }

    // get column name
    const CString& GetColumnName(int nCol) const
    {
        return m_columns[nCol]->GetName();
    }

    // get column type
    TDBFieldType GetColumnType(int nCol) const
    {
        return m_columns[nCol]->GetType();
    }

    // check if column exists
    bool ColumnExists(LPCTSTR name) const
    {
        return m_columns.FindKey(name) != NULL;
    }

    // check if column exists
    bool ColumnExists(const TDBField& dbf) const;

    // get column by field ID
    const CColumn& GetColumn(const TDBField& dbf) const
    {
        return const_cast<CRecordsetAF*>(this)->GetColumnNC(dbf);
    }

    // get column by name
    const CColumn& GetColumn(LPCTSTR name) const
    {
        ASSERT(ColumnExists(name));
        return *m_columns.FindKey(name);
    }

    // get column by index
    const CColumn& GetColumn(int nCol) const
    {
        return *m_columns[nCol];
    }

    // get number of rows
    int GetRowCount() const
    {
        return m_nRows;
    }

    // get capacity
    int GetCapacity() const
    {
        return m_capacity;
    }

    // create new row, return index of new row
    int AddRow(int newCapacity = 0);

    // delete row (maybe more than one)
    void DeleteRow(int nRow, int count = 1);

    // delete all rows
    void DeleteAllRows();

    // dump into a string
    virtual CString ToString() const;

    // does recordset contain any columns?
    bool IsValid() const
    {
        return GetColumnCount() > 0;
    }

    // is recordset empty?
    bool IsEmpty() const
    {
        return GetRowCount() <= 0;
    }
};
}  // namespace AFLibDB
