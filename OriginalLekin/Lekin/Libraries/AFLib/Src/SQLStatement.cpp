#include "StdAfx.h"

#include "SQLStatement.h"

#include "AFLibDatabaseAF.h"
#include "AFLibDlgRun.h"
#include "AFLibRecordsetAF.h"
#include "AFLibStringParser.h"
#include "AFLibThread.h"
#include "AFLibThreadSafeStatic.h"

#undef DBCHAR
#include <odbcss.h>  // SQL Server functions; includes WRONG definition of LPCBYTE!

using namespace AFLibDB;
using namespace AFLib;
using namespace AFLibThread;
using AFLibIO::TStringBuffer;
using AFLibPrivate::TSQLStatement;

/////////////////////////////////////////////////////////////////////////////
// TSQLStatement::TStringParserEx

TSQLStatement::TStringParserEx::TStringParserEx(LPCTSTR sPtr) : super(sPtr)
{}

bool TSQLStatement::TStringParserEx::ReadPrefix(TStringBuffer& sDest)
{
    if (!ProtectedTestSymbol(_T('['))) return false;
    ProtectedFindFirstCh(true, _T(']'), sDest);
    return true;
}

void TSQLStatement::TStringParserEx::ReadMsg(TStringBuffer& sDest)
{
    ProtectedReadSegment(NULL, false, sDest);
}

/////////////////////////////////////////////////////////////////////////////
// TSQLStatement

TSQLStatement::TSQLStatement(CDatabaseAF* pDB, bool bReadMsg)
{
    m_pRs = NULL;
    m_pDB = pDB;
    m_pDlgRun = NULL;
    m_bReadMsg = bReadMsg;
    m_bAsyncStarted = false;
    m_nBulk = 1;
    m_hstmt = SQL_NULL_HSTMT;
    m_pRowStatus = NULL;
    m_nRowsFetched = 0;
}

TSQLStatement::~TSQLStatement()
{
    Close();
    m_arrColSrc.RemoveAll();
}

bool TSQLStatement::PrivateEndSQO(bool bReadMsg)
{
    switch (m_retCode)
    {
        case SQL_NO_DATA:
            return false;
        case SQL_SUCCESS:
            return true;
        case SQL_SUCCESS_WITH_INFO:
            if (bReadMsg) ReadMsg();
            return true;
    }

    PrivateThrow();
    return false;
}

bool TSQLStatement::IsAsyncNeeded()
{
    return m_pDlgRun != NULL || CThread::CanBreak();
}

void TSQLStatement::SetAsyncAttr(bool bOn)
{
    SQLSetStmtAttr(m_hstmt, SQL_ATTR_ASYNC_ENABLE,
        reinterpret_cast<SQLPOINTER>(bOn ? SQL_ASYNC_ENABLE_ON : SQL_ASYNC_ENABLE_OFF), SQL_IS_UINTEGER);
    EndSQO();
    m_bAsyncStarted = bOn;
}

void TSQLStatement::StartAsync()
{
    if (!m_bAsyncStarted && IsAsyncNeeded()) SetAsyncAttr(true);
}

bool TSQLStatement::AsyncSQO(SQLRETURN (TSQLStatement::*F)())
{
    bool bBroken = false;
    bool bBreakable = CThread::CanBreak();

    while (true)
    {
        m_retCode = (this->*F)();
        ReadMsg();
        if (m_retCode != SQL_STILL_EXECUTING) break;

        if (!bBroken && bBreakable && CThread::IsBroken())
        {
            SQLRETURN retCode = SQLCancel(m_hstmt);
            bBroken = retCode == SQL_SUCCESS;
        }

        Sleep(3);
    }

    CThread::Break();
    return PrivateEndSQO(false);
}

void TSQLStatement::EndSQO()
{
    if (!PrivateEndSQO(true)) PrivateThrow();
    CThread::Break();
}

SQLRETURN TSQLStatement::SQOExecDirect()
{
    return SQLExecDirect(m_hstmt, (SQLTCHAR*)(LPCTSTR(m_query)), SQL_NTS);
}

SQLRETURN TSQLStatement::SQOFetch()
{
    return SQLFetchScroll(m_hstmt, SQL_FETCH_NEXT, 0);
}

SQLRETURN TSQLStatement::SQOBulkAdd()
{
    return SQLBulkOperations(m_hstmt, SQL_ADD);
}

void TSQLStatement::PrivateOpen(LPCTSTR query, bool bExec, bool bForBulkODBC)
{
    ASSERT(m_hstmt == SQL_NULL_HSTMT);
    CThread::Break();

    m_retCode = SQLAllocHandle(SQL_HANDLE_STMT, m_pDB->m_hdbc, &m_hstmt);
    EndSQO();

    if (!bExec)
    {
        m_retCode = SQLSetStmtAttr(m_hstmt, SQL_ATTR_CONCURRENCY,
            reinterpret_cast<SQLPOINTER>(bForBulkODBC ? SQL_CONCUR_LOCK : SQL_CONCUR_READ_ONLY),
            SQL_IS_UINTEGER);
        EndSQO();

        m_retCode = SQLSetStmtAttr(m_hstmt, SQL_CURSOR_TYPE,
            reinterpret_cast<SQLPOINTER>(bForBulkODBC ? SQL_CURSOR_KEYSET_DRIVEN : SQL_CURSOR_FORWARD_ONLY),
            SQL_IS_UINTEGER);
        EndSQO();
    }

    if (IsAsyncNeeded())
        StartAsync();
    else
        SetAsyncAttr(false);

    m_query = query;

    if (!AsyncSQO(&TSQLStatement::SQOExecDirect)) PrivateThrow();

    // set the asynchronous flag to false for SQLNumResultCols() and SQLDescribeCol()
    if (m_bAsyncStarted) SetAsyncAttr(false);
}

void TSQLStatement::Open(CRecordsetAF* pRs, bool bForBulkODBC)
{
    m_pRs = pRs;
    m_nBulk = m_pRs->m_param.m_nBulk;
    m_pDlgRun = pRs->m_pDlgRun;
    PrivateOpen(m_pRs->GetQuery(), false, bForBulkODBC);
}

void TSQLStatement::OpenExec(LPCTSTR query)
{
    PrivateOpen(query, true, false);
}

void TSQLStatement::Close()
{
    if (m_hstmt != SQL_NULL_HSTMT)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        m_hstmt = SQL_NULL_HSTMT;
    }

    if (m_pRowStatus != NULL)
    {
        delete[] m_pRowStatus;
        m_pRowStatus = NULL;
    }
}

void TSQLStatement::ReadMsg()
{
    SAFE_STATIC_SB(msgFinal);
    if (!m_bReadMsg) return;

    for (int curMsg = 1;; ++curMsg)
    {
        SQLTCHAR sqlState[6] = {0};
        SQLTCHAR msg[SQL_MAX_MESSAGE_LENGTH] = {0};
        SQLINTEGER nativeError = 0;
        SQLSMALLINT msgLen = 0;

        SQLRETURN rc = SQLGetDiagRec(
            SQL_HANDLE_STMT, m_hstmt, curMsg, sqlState, &nativeError, msg, SQL_MAX_MESSAGE_LENGTH, &msgLen);

        if (rc == SQL_ERROR || rc == SQL_NO_DATA) break;

        TStringParserEx sp(reinterpret_cast<LPCTSTR>(msg));
        for (int i = 0; i < mspN; ++i)
            if (!sp.ReadPrefix(m_arrMsgPrefix[i])) break;

        sp.ReadMsg(msgFinal);
        if (m_pDlgRun != NULL) m_pDlgRun->ProcessSQLMsg(msgFinal);

        m_logMsg += msgFinal;
        m_logMsg += strEol;
    }
}

void TSQLStatement::SetBulk(int nBulk)
{
    ASSERT(m_pRs != NULL);
    m_retCode = SQLSetStmtAttr(
        m_hstmt, SQL_ATTR_ROW_ARRAY_SIZE, reinterpret_cast<SQLPOINTER>(nBulk), SQL_IS_UINTEGER);
    EndSQO();
}

void TSQLStatement::Bind()
{
    ASSERT(m_pRs != NULL);
    ASSERT(m_pRowStatus == NULL);
    m_pRowStatus = new SQLUSMALLINT[m_nBulk];

    m_retCode = SQLSetStmtAttr(
        m_hstmt, SQL_ATTR_ROW_BIND_TYPE, reinterpret_cast<SQLPOINTER>(SQL_BIND_BY_COLUMN), SQL_IS_UINTEGER);
    EndSQO();

    SetBulk(m_nBulk);

    m_retCode = SQLSetStmtAttr(m_hstmt, SQL_ATTR_ROW_STATUS_PTR, m_pRowStatus, SQL_IS_POINTER);
    EndSQO();

    m_retCode = SQLSetStmtAttr(m_hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &m_nRowsFetched, SQL_IS_POINTER);
    EndSQO();
}

void TSQLStatement::ReduceBulk(int nBulk)
{
    if (nBulk < m_nBulk) SetBulk(nBulk);
}

bool TSQLStatement::Fetch()
{
    ASSERT(m_pRs != NULL);

    if (!AsyncSQO(&TSQLStatement::SQOFetch)) return false;

    for (SQLUINTEGER i = 0; i < m_nRowsFetched; ++i)
        if (m_pRowStatus[i] != SQL_ROW_SUCCESS && m_pRowStatus[i] != SQL_ROW_SUCCESS_WITH_INFO)
        {
            m_nRowsFetched = i;
            break;
        }

    CThread::Break();
    return m_nRowsFetched > 0;
}

void TSQLStatement::BulkAdd()
{
    ASSERT(m_pRs != NULL);
    if (!AsyncSQO(&TSQLStatement::SQOBulkAdd)) PrivateThrow();
}

int TSQLStatement::CountColumns()
{
    SQLSMALLINT nCols = 0;
    m_retCode = SQLNumResultCols(m_hstmt, &nCols);
    EndSQO();
    return nCols;
}

void TSQLStatement::PrivateThrow()
{
    AfxThrowDBException(m_retCode, m_pDB, m_hstmt);
}

void TSQLStatement::ThrowBCP()
{
    if (m_retCode != SUCCEED)
    {
        m_retCode = SQL_ERROR;
        PrivateThrow();
    }
}
