#include "StdAfx.h"

#include "AFLibRecordsetAF.h"

#include "AFLibDBField.h"
#include "AFLibDatabaseAF.h"
#include "AFLibDatabasePool.h"
#include "AFLibDlgRun.h"
#include "AFLibExcBreak.h"
#include "AFLibExcMessage.h"
#include "AFLibRecordsetDef.h"
#include "AFLibRow.h"
#include "AFLibSemaLock.h"
#include "AFLibStringSpacer.h"
#include "AFLibStringTokenizer.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "LibBcp.h"
#include "Local.h"
#include "SQLStatement.h"

using namespace AFLibDB;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using namespace AFLibThread;

namespace {
CString InsertTop(LPCTSTR query, int maxRows)
{
    SAFE_CONST_STR(strSelectNS, _T("select"));
    SAFE_CONST_STR(strTopNS, _T("top"));

    TStringSpacer sp(query);
    sp.SetIgnoreCase(true);

    if (!sp.TestSymbol(strSelectNS)) return query;

    if (sp.TestSymbol(strTopNS))
    {
        int maxRows2 = sp.ReadInt();
        if (maxRows >= maxRows2) return query;
    }

    int maxRows3 = max(maxRows, 1);
    return strSelect + strTop + IntToStr(maxRows3) + strSpace + sp.ReadFinal();
}
}  // namespace

CRecordsetAF::CRecordsetAF()
{
    m_capacity = 0;
    m_nRows = 0;
    m_pDlgRun = NULL;
    m_param = GetDefaultRecordsetParam();
    m_arrField.SetSize(0, 1 << 10);
}

CRecordsetAF::~CRecordsetAF()
{
    Close();
}

void CRecordsetAF::PrivateClose()
{
    m_columns.DestroyAll();
    m_capacity = 0;
    m_nRows = 0;
}

void CRecordsetAF::Close()
{
    ProcessAttachedRows(0, m_nRows);
    PrivateClose();
}

int CRecordsetAF::GetAFLibCount(TSQLStatement& stm)
{
    SAFE_CONST_STR(strCount, _T("AFLIB count ="));
    int pos = 0;

    while (true)
    {
        int pos2 = stm.m_logMsg.Find(strCount, pos);
        if (pos2 < 0) break;
        pos = pos2 + strCount.GetLength();
    }

    if (pos <= 0) return -1;

    TStringSpacer sp(LPCTSTR(stm.m_logMsg) + pos);
    try
    {
        int count = sp.ReadInt();
        if (count >= 0) return count;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return -1;
}

void CRecordsetAF::Open(CDatabaseAF* pDB, LPCTSTR query, int maxRows, TStringBuffer& logMsg)
{
    ASSERT(maxRows >= 0);

    if (IsValid()) ThrowMessage(LocalAfxString(IDS_ALREADY_OPEN));

    if (maxRows < INT_MAX && pDB->IsTopSupported())
        m_query = InsertTop(query, maxRows);
    else
        m_query = query;

    try
    {
        TSQLStatement stm(pDB, true);
        CDatabaseAF::CLock lock(pDB, false);

        stm.Open(this, false);
        PrepareColumns(stm);

        if (maxRows > 0)
        {
            stm.m_nBulk = min(maxRows, stm.m_nBulk);

            int nColumnsBound = BindColumns(stm);
            int count = GetAFLibCount(stm);

            if (count > 0)
            {
                if (m_pDlgRun != NULL) m_pDlgRun->SetTotalClicks(count);

                m_capacity = count;
                for (int i = 0; i < GetColumnCount(); ++i) m_columns[i]->Realloc(0);
            }

            stm.StartAsync();

            while (m_nRows < maxRows)
            {
                CThread::Break();

                for (int i = 0; i < GetColumnCount(); ++i) m_columns[i]->CleanTempBuffer();

                stm.ReduceBulk(maxRows - m_nRows);
                if (!stm.Fetch()) break;

                m_nRows += stm.m_nRowsFetched;
                if (m_pDlgRun != NULL) m_pDlgRun->AddClick(stm.m_nRowsFetched);

                int oldCapacity = m_capacity;

                if (m_nRows > m_capacity)
                {
                    int newCapacity = m_nRows;
                    if (m_nRows < maxRows && stm.m_nRowsFetched == stm.m_nBulk)
                    {  // double capacity, but no more than maxRows, but no less than m_nRows
                        newCapacity = max(m_nRows, min(maxRows, m_capacity * 2));
                    }
                    m_capacity = newCapacity;
                }

                for (int i = 0; i < GetColumnCount(); ++i) m_columns[i]->Realloc(oldCapacity);

                CThread::Break();
                for (int i = 0; i < nColumnsBound; ++i) m_columns[i]->UnloadTempBuffer(stm.m_nRowsFetched);
            }

            lock.Unlock();

            for (int i = 0; i < nColumnsBound; ++i) m_columns[i]->DestroyTempBuffer();

            logMsg += stm.m_logMsg;
        }
    }
    catch (CDBException* pExc)
    {
        UINT idsHelp = 0;
        CString msg = GetExceptionMsg(pExc, &idsHelp);
        pExc->Delete();
        PrivateClose();
        throw new CExcMessage(idsHelp, LocalAfxString(IDS_SQL_ERROR, msg));
    }
    catch (...)
    {
        PrivateClose();
        throw;
    }
}

void CRecordsetAF::Open(CDatabaseAF* pDB, LPCTSTR query, int maxRows)
{
    SAFE_STATIC_SB(logMsg);
    Open(pDB, query, maxRows, logMsg);
}

void CRecordsetAF::Open(CDatabasePool* pDP, LPCTSTR query, int maxRows, TStringBuffer& logMsg)
{
    Open(pDP->GetDB(), query, maxRows, logMsg);
}

void CRecordsetAF::Open(CDatabasePool* pDP, LPCTSTR query, int maxRows)
{
    Open(pDP->GetDB(), query, maxRows);
}

void CRecordsetAF::LoadTableDef(CDatabaseAF* pDB, LPCTSTR table, LPCTSTR fields)
{
    Open(pDB, CreateQuery(table, fields, strFalse, strEmpty), 0);
}

void CRecordsetAF::LoadTableDef(CDatabasePool* pDP, LPCTSTR table, LPCTSTR fields)
{
    LoadTableDef(pDP->GetDB(), table, fields);
}

void CRecordsetAF::LoadTableData(CDatabaseAF* pDB, LPCTSTR table, int maxRows)
{
    ASSERT(maxRows >= 0);

    if (maxRows == 0)
        LoadTableDef(pDB, table);
    else
        Open(pDB, CreateQuery(table, strEmpty, strEmpty, strEmpty), maxRows);
}

void CRecordsetAF::LoadTableData(CDatabasePool* pDP, LPCTSTR table, int maxRows)
{
    LoadTableData(pDP->GetDB(), table, maxRows);
}

void CRecordsetAF::Copy(const CRecordsetAF& recordset, int maxRows, LPCTSTR fields)
{
    ASSERT(maxRows >= 0);

    if (IsValid()) ThrowMessage(LocalAfxString(IDS_ALREADY_OPEN));

    m_query = recordset.m_query;
    m_param = recordset.m_param;

    if (maxRows > 0) m_nRows = min(maxRows, recordset.m_nRows);

    // if too few rows, set capacity to 16 just in case
    if (m_nRows > 0) m_capacity = max(m_nRows, 16);

    if (strStar == fields || IsEmptyStr(fields))
    {
        for (int i = 0; i < recordset.GetColumnCount(); ++i)
        {
            CThread::Break();
            CColumn* pColumn = CreateColumn();
            pColumn->Copy(recordset.GetColumn(i));
            AddColumnHelper(pColumn);
        }
    }
    else
    {
        TStringTokenizer sp(fields, chrComma);
        ASSERT(!sp.IsEof());

        while (!sp.IsEof())
        {
            CThread::Break();
            CString fieldName = sp.ReadStr();
            fieldName.Trim();

            if (fieldName.IsEmpty() || !recordset.ColumnExists(fieldName) || ColumnExists(fieldName))
                continue;

            CColumn* pColumn = CreateColumn();
            pColumn->Copy(recordset.GetColumn(fieldName));
            AddColumnHelper(pColumn);
        }
    }
}

void CRecordsetAF::Build(const CRecordsetDef& rsd)
{
    if (IsValid()) ThrowMessage(LocalAfxString(IDS_ALREADY_OPEN));

    if (rsd.IsEmpty()) ThrowMessage(LocalAfxString(IDS_EMPTY_RS_DEF));

    m_query = strSelect;

    for (int i = 0; i < rsd.GetColumnCount(); ++i)
    {
        CString fieldName = rsd.GetColumnName(i);
        AddColumn(fieldName, DBFieldTypeToODBCType(rsd.GetColumnType(i)), true, SQL_NO_TOTAL);

        if (i > 0) m_query += strCommaSpace;
        m_query += fieldName;
    }
}

void CRecordsetAF::SetRecordsetParam(const TRecordsetParam& param)
{
    m_param.Change(param);
}

CRecordsetAF* CRecordsetAF::CreateBulkInsertClone() const
{
    return new CRecordsetAF;
}

void CRecordsetAF::ProcessRow(CRow* pRow, int nRow, int count)
{
    if (pRow == NULL) return;

    if (pRow->m_nRow >= nRow + count)
        pRow->m_nRow -= count;
    else if (pRow->m_nRow >= nRow)
        pRow->Close();
}

void CRecordsetAF::ProcessAttachedRows(int nRow, int count)
{
    if (m_rows.GetCount() == 0) return;

    CRow* pRow = NULL;

    for (POSITION pos = m_rows.GetStartPosition(); pos != NULL;)
    {
        CRow* pRowNext = m_rows.GetNextAssoc(pos);
        // pRow->Close() will remove the row from m_rows.
        // Just in case: find pointer to next row before closing.
        ProcessRow(pRow, nRow, count);
        pRow = pRowNext;
    }

    ProcessRow(pRow, nRow, count);
}

CColumn* CRecordsetAF::CreateColumn()
{
    return new CColumn(this);
}

namespace {
WORD BadFieldID = 0xFFFF;
}

bool CRecordsetAF::ColumnExists(const TDBField& dbf) const
{
    int ID = dbf.GetID();
    return ID < 0 ? ColumnExists(dbf.S()) : ID < m_arrField.GetSize() && m_arrField[ID] != BadFieldID;
}

CColumn& CRecordsetAF::GetColumnNC(const TDBField& dbf)
{
    int ID = dbf.GetID();
    return ID < 0 ? GetColumnNC(dbf.S()) : *m_columns[m_arrField[ID]];
}

void CRecordsetAF::AddColumnHelper(CColumn* pColumn)
{
    int n = m_columns.GetSize();
    m_columns.Add(pColumn);

    int ID = TDBField::GetFieldID(pColumn->m_fieldName);
    if (ID < 0) return;

    while (ID >= m_arrField.GetSize()) m_arrField.Add(BadFieldID);
    m_arrField[ID] = n;
}

void CRecordsetAF::AddColumn(LPCTSTR fieldName, int rawODBCType, bool bNullable, int colSize)
{
    ASSERT(m_columns.FindKey(fieldName) == NULL);

    CColumn* pColumn = CreateColumn();
    pColumn->m_fieldName = fieldName;
    pColumn->m_rawODBCType = rawODBCType;
    pColumn->m_bNullable = bNullable;

    if (colSize != SQL_NO_TOTAL) pColumn->m_origColSize = colSize;

    AddColumnHelper(pColumn);
    pColumn->ResetFormat();
}

void CRecordsetAF::PrepareColumns(TSQLStatement& stm)
{
    int nCols = stm.CountColumns();

    for (int i = 0; i < nCols; ++i)
    {
        static const int BUF_LEN = 1 << 8;
        SQLTCHAR nameBuf[BUF_LEN];
        SQLSMALLINT nameLength = 0;
        SQLSMALLINT rawODBCType = 0;
        SQLUINTEGER colSize = 0;
        SQLSMALLINT nullable = 0;

        stm.m_retCode = SQLDescribeCol(
            stm.m_hstmt, i + 1, nameBuf, BUF_LEN - 1, &nameLength, &rawODBCType, &colSize, NULL, &nullable);
        stm.EndSQO();

        CString fieldName(reinterpret_cast<TCHAR*>(nameBuf), nameLength);
        if (fieldName.IsEmpty())
            fieldName = _T("(") + IntToStr(i) + _T(")");
        else if (m_columns.FindKey(fieldName) != NULL)
            fieldName += strSpace + _T("(") + IntToStr(i) + _T(")");
        else
            CDatabaseAF::CheckFieldName(fieldName);

        AddColumn(fieldName, rawODBCType, nullable != SQL_NO_NULLS, colSize);
    }
}

int CRecordsetAF::BindColumns(TSQLStatement& stm)
{
    stm.Bind();

    // can't bind more than 254 columns!
    int nColumnsBound = min(GetColumnCount(), 254);

    for (int i = 0; i < nColumnsBound; ++i)
    {
        CColumn* pColumn = m_columns[i];
        pColumn->CreateTempBuffer();

        stm.m_retCode = SQLBindCol(stm.m_hstmt, i + 1, DBFieldTypeToCType(pColumn->GetType()),
            pColumn->m_pTempData, pColumn->m_rawFieldSize, pColumn->m_pTempLen);
        stm.EndSQO();
    }

    return nColumnsBound;
}

void CRecordsetAF::BindColumnsForBCP(TSQLStatement& stm)
{
    stm.Close();

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        CColumn* pColumn = m_columns[i];
        pColumn->CreateBcpTempBuffer();

        stm.m_retCode = TLibBcp::m_a.Bind(
            stm.m_pDB->m_hdbc, pColumn->m_pTempLen, DBFieldTypeToBCPType(pColumn->GetType()), i + 1);
        stm.ThrowBCP();
    }
}

void CRecordsetAF::NullifyUnassignedColumns(TSQLStatement& stm)
{
    for (int i = 0; i < stm.m_pRs->GetColumnCount(); ++i)
    {
        CColumn* pColDst = &stm.m_pRs->GetColumnNC(i);
        if (stm.m_arrColSrc[i] == NULL) pColDst->NullifyTempBuffer();
    }
}

void CRecordsetAF::BulkInsertBySQL(TSQLStatement& stm, LPCTSTR table) const
{
    stm.Close();

    SAFE_STATIC_SB(queryStart);
    SAFE_STATIC_SB(query);

    queryStart += strInsertInto;
    queryStart += table;
    queryStart += _T(" (");

    for (int i = 0; i < stm.m_pRs->GetColumnCount(); ++i)
    {
        if (i > 0) queryStart += strCommaSpace;
        queryStart += stm.m_pRs->GetColumnName(i);
    }

    queryStart += _T(')');
    queryStart += strValues;
    queryStart += _T('(');

    for (int r = 0; r < GetRowCount(); ++r)
    {
        CThread::Break();
        query.Empty();
        query.Append(queryStart);

        for (int i = 0; i < stm.m_pRs->GetColumnCount(); ++i)
        {
            CColumn* pColDst = &stm.m_pRs->GetColumnNC(i);
            const CColumn* pColSrc = stm.m_arrColSrc[i];
            bool bNull = false;

            if (i > 0) query += strCommaSpace;

            if (pColSrc == NULL)
                bNull = true;
            else
                bNull = pColSrc->IsNull(r);

            if (bNull)
            {
                if (pColDst->IsNullable())
                    query += strNull;
                else if (pColDst->IsChar())
                    query += _T("''");
                else if (pColDst->IsDate())
                    query += _T("'1/1/1900'");
                else if (pColDst->IsBin())
                    query += _T("0x00");
                else
                    query += _T('0');
            }
            else
                query += pColSrc->GetFmtString(r, pColSrc);
        }

        query += _T(')');
        stm.m_pDB->ExecSQL(query, m_pDlgRun);
        if (m_pDlgRun != NULL) m_pDlgRun->AddClick();
    }
}

void CRecordsetAF::BulkInsertByODBC(TSQLStatement& stm) const
{
    if (stm.m_pRs->BindColumns(stm) < stm.m_pRs->GetColumnCount())
        ThrowMessage(LocalAfxString(IDS_TOO_MANY_COLUMNS));

    NullifyUnassignedColumns(stm);
    stm.StartAsync();

    for (int rStart = 0; rStart < GetRowCount(); rStart += stm.m_nBulk)
    {
        CThread::Break();
        int count = min(GetRowCount() - rStart, stm.m_nBulk);
        stm.ReduceBulk(count);

        // fill data into columns
        for (int i = 0; i < stm.m_pRs->GetColumnCount(); ++i)
        {
            CColumn* pColDst = &stm.m_pRs->GetColumnNC(i);
            const CColumn* pColSrc = stm.m_arrColSrc[i];
            if (pColSrc != NULL) pColDst->LoadTempBuffer(*pColSrc, rStart, count);
        }

        // call ODBC Bulk Insert
        stm.BulkAdd();

        if (m_pDlgRun != NULL) m_pDlgRun->AddClick(count);
    }
}

void CRecordsetAF::BulkInsertByBCP(TSQLStatement& stm, LPCTSTR table) const
{
    int lastFlush = 0;

    stm.Close();
    stm.m_retCode = TLibBcp::m_a.Init(stm.m_pDB->m_hdbc, table);
    stm.ThrowBCP();

    try
    {
        stm.m_pRs->BindColumnsForBCP(stm);
        NullifyUnassignedColumns(stm);

        for (int r = 0; r < GetRowCount(); ++r)
        {
            CThread::Break();

            // flush if necessary
            if (r > 0 && r % m_param.m_nBulk == 0)
            {
                int count = TLibBcp::m_a.Batch(stm.m_pDB->m_hdbc);
                if (count < r - lastFlush) ThrowMessage(LocalAfxString(IDS_BCP_ERROR, table));
                lastFlush = r;
            }

            // fill data into columns
            for (int i = 0; i < stm.m_pRs->GetColumnCount(); ++i)
            {
                CColumn* pColDst = &stm.m_pRs->GetColumnNC(i);
                const CColumn* pColSrc = stm.m_arrColSrc[i];

                if (pColSrc != NULL)
                    pColDst->LoadTempBufferElement(*pColSrc, r, pColDst->m_pTempData, pColDst->m_pTempLen);
            }

            stm.m_retCode = TLibBcp::m_a.SendRow(stm.m_pDB->m_hdbc);
            stm.ThrowBCP();

            if (m_pDlgRun != NULL) m_pDlgRun->AddClick();
        }
    }
    catch (...)
    {
        TLibBcp::m_a.Done(stm.m_pDB->m_hdbc);
        throw;
    }

    int count = TLibBcp::m_a.Done(stm.m_pDB->m_hdbc);
    if (count < GetRowCount() - lastFlush) ThrowMessage(LocalAfxString(IDS_BCP_ERROR, table));
}

void CRecordsetAF::BulkInsert(CDatabaseAF* pDB, LPCTSTR table, TBulkInsertMethod bim) const
{
    if (GetRowCount() < 1) return;

    switch (bim)
    {
        case bimSmart:
            bim = pDB->IsBcpSupported() ? bimBCP : pDB->IsBulkAddSupported() ? bimODBC : bimSQL;
            break;

        case bimBCP:
            if (!pDB->IsBcpSupported()) ThrowMessage(LocalAfxString(IDS_NO_BCP));
            break;

        case bimODBC:
            if (!pDB->IsBulkAddSupported()) ThrowMessage(LocalAfxString(IDS_NO_BULK_ADD));
            break;
    }

    CAutoPtr<CRecordsetAF> pRs2(CreateBulkInsertClone());
    pRs2->m_query = CreateQuery(table, strEmpty, strFalse, strEmpty);
    pRs2->SetRecordsetParam(GetRecordsetParam());
    pRs2->m_param.m_maxCharBuffer = INT_MAX;
    pRs2->m_param.m_maxLongCharBuffer = INT_MAX;
    pRs2->m_param.m_bThrowOnBadData = m_param.m_bThrowOnBadData;

    pRs2->m_param.m_nBulk = bim == bimODBC ? min(GetRowCount(), m_param.m_nBulk) : 1;

    try
    {
        TSQLStatement stm(pDB, false);
        CDatabaseAF::CLock lock(pDB, bim == bimBCP);

        stm.Open(pRs2, bim == bimODBC);
        pRs2->PrepareColumns(stm);
        stm.m_arrColSrc.SetSize(pRs2->GetColumnCount());

        for (int i = 0; i < pRs2->GetColumnCount(); ++i)
        {
            CColumn* pColDest = &pRs2->GetColumnNC(i);
            const CColumn* pColSrc = m_columns.FindKey(pColDest->GetName());
            stm.m_arrColSrc[i] = pColSrc;

            if (pColSrc != NULL) pColDest->ResetForBulkInsert(*pColSrc);
        }

        // add one row to be used as a buffer
        pRs2->AddRow(1);
        CThread::Break();

        switch (bim)
        {
            case bimSQL:
                BulkInsertBySQL(stm, table);
                break;

            case bimODBC:
                BulkInsertByODBC(stm);
                break;

            case bimBCP:
                BulkInsertByBCP(stm, table);
                break;
        }
    }
    catch (CDBException* pExc)
    {
        UINT idsHelp = 0;
        CString msg = GetExceptionMsg(pExc, &idsHelp);
        pExc->Delete();
        throw new CExcMessage(idsHelp, LocalAfxString(IDS_SQL_ERROR, msg));
    }
}

void CRecordsetAF::BulkInsert(CDatabasePool* pDP, LPCTSTR table, TBulkInsertMethod bim) const
{
    BulkInsert(pDP->GetDB(), table, bim);
}

int CRecordsetAF::AddRow(int newCapacity)
{
    int oldCapacity = m_capacity;
    ++m_nRows;

    if (m_nRows > m_capacity)
    {
        if (newCapacity >= m_nRows)
            m_capacity = newCapacity;
        else
            m_capacity = oldCapacity == 0 ? 16 : oldCapacity * 2;
    }

    for (int i = 0; i < GetColumnCount(); ++i) m_columns[i]->Realloc(oldCapacity);

    return m_nRows - 1;
}

void CRecordsetAF::DeleteRow(int nRow, int count)
{
    int oldNRows = m_nRows;
    count = min(count, oldNRows - nRow);
    if (count <= 0) return;

    ProcessAttachedRows(nRow, count);

    for (int i = 0; i < GetColumnCount(); ++i) m_columns[i]->DeleteRow(nRow, count);

    m_nRows -= count;
}

void CRecordsetAF::DeleteAllRows()
{
    ProcessAttachedRows(0, m_nRows);

    for (int i = 0; i < GetColumnCount(); ++i) m_columns[i]->DeleteAllRows();

    m_nRows = m_capacity = 0;
}

CString CRecordsetAF::ToString() const
{
    CIntArray widths;
    widths.SetSize(GetColumnCount());

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        const CColumn& column = GetColumn(i);
        widths[i] = column.GetName().GetLength();

        for (int r = 0; r < GetRowCount(); ++r)
            widths[i] = max(column.GetFmtString(r).GetLength(), widths[i]);

        if (i > 0) ++widths[i];
    }

    TStringBuffer buffer;

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        const CString& s = GetColumnName(i);
        buffer.AppendCh(chrSpace, widths[i] - s.GetLength());
        buffer += s;
    }
    buffer += strEol;

    for (int r = 0; r < GetRowCount(); ++r)
    {
        CRow row(*this, r);
        for (int i = 0; i < GetColumnCount(); ++i)
        {
            CString s = row.GetFmtString(i);
            buffer.AppendCh(chrSpace, widths[i] - s.GetLength());
            buffer += s;
        }
        buffer += strEol;
    }

    return LPCTSTR(buffer);
}
