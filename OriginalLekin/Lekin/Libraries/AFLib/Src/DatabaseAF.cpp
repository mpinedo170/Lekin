#include "StdAfx.h"

#include "AFLibDatabaseAF.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibRecordsetAF.h"
#include "AFLibSemaphoreAF.h"
#include "AFLibStringTokenizerAlt.h"
#include "AFLibThreadSafeConst.h"
#include "LibBcp.h"
#include "Local.h"
#include "SQLStatement.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibDB;
using namespace AFLibThread;
using namespace AFLibPrivate;

/////////////////////////////////////////////////////////////////////////////
// CDatabaseAF::CLock

CDatabaseAF::CLock::CLock(CDatabaseAF* pDB, bool bExclusive) :
    super(pDB->m_pSem, bExclusive ? CSemaLock::exclusive : 1)
{}

/////////////////////////////////////////////////////////////////////////////
// CDatabaseAF

CDatabaseAF::CDatabaseAF()
{
    m_bBcpEnabled = false;
    m_pSem = NULL;
    ClearFlags();
}

CDatabaseAF::~CDatabaseAF()
{
    delete m_pSem;
}

void CDatabaseAF::ClearFlags()
{
    m_bBcpSupported = m_bBulkAddSupported = m_bTruncateSupported = m_bTopSupported = false;
    m_bSysIndSupported = true;
    delete m_pSem;
    m_pSem = NULL;
}

bool CDatabaseAF::EnableBcp()
{
    ASSERT(!IsOpen());
    m_bBcpEnabled = TLibBcp::m_a.InitLibrary();
    return m_bBcpEnabled;
}

BOOL CDatabaseAF::OpenEx(LPCTSTR lpszConnectString, DWORD dwOptions)
{
    ASSERT_VALID(this);
    ASSERT(lpszConnectString == NULL || AfxIsValidString(lpszConnectString));
    ASSERT(!(dwOptions & noOdbcDialog && dwOptions & forceOdbcDialog));

    ClearFlags();

    // Exclusive access not supported.
    ASSERT(!(dwOptions & openExclusive));

    m_bUpdatable = !(dwOptions & openReadOnly);

    try
    {
        RETCODE retCode = 0;
        SQLINTEGER attrInt = 0;

        m_strConnect = lpszConnectString;

        // Allocate the HDBC
        AllocConnect(dwOptions);

        // Allow BCP operations on the HDBC
        if (m_bBcpEnabled) SQLSetConnectAttr(m_hdbc, SQL_COPT_SS_BCP, LPVOID(SQL_BCP_ON), SQL_IS_INTEGER);

        // Make connection
        if (!Connect(dwOptions)) return false;

        // Verify support for required functionality and cache info
        VerifyConnect();
        GetConnectInfo();

        // If BCP was previously enabled, check if it is supported
        if (m_bBcpEnabled)
        {
            retCode = SQLGetConnectAttr(m_hdbc, SQL_COPT_SS_BCP, &attrInt, sizeof(SQLINTEGER), NULL);
            if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
                if ((attrInt & 0xFFFF) == SQL_BCP_ON) m_bBcpSupported = true;
        }

        retCode = SQLGetInfo(m_hdbc, SQL_MAX_CONCURRENT_ACTIVITIES, &attrInt, sizeof(SQLINTEGER), NULL);
        attrInt &= 0xFFFF;

        if ((retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) || attrInt < 1) attrInt = 1;
        m_pSem = new CSemaphoreAF(attrInt);

        retCode = SQLGetInfo(m_hdbc, SQL_KEYSET_CURSOR_ATTRIBUTES1, &attrInt, sizeof(SQLINTEGER), NULL);
        if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
            if ((attrInt & SQL_CA1_BULK_ADD) != 0) m_bBulkAddSupported = true;

        static const int BUF_SIZE = 1 << 10;
        CString buffer;
        retCode =
            SQLGetInfo(m_hdbc, SQL_KEYWORDS, buffer.GetBuffer(BUF_SIZE), BUF_SIZE * sizeof(TCHAR), NULL);
        buffer.ReleaseBuffer();

        if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
        {
            buffer.MakeLower();
            TStringTokenizerAlt sp(buffer, strCommaSpace);

            while (!sp.IsEof())
            {
                CString keyWord = sp.ReadStr();
                if (keyWord == _T("truncate"))
                    m_bTruncateSupported = true;
                else if (keyWord == _T("top"))
                    m_bTopSupported = true;
            }
        }
    }
    catch (...)
    {
        Free();
        throw;
    }

    return true;
}

void CDatabaseAF::Close()
{
    super::Close();
    ClearFlags();
}

bool CDatabaseAF::IsAvailable(bool bExclusive) const
{
    return m_pSem->GetFreeCount() >= (bExclusive ? m_pSem->GetMaxCount() : 1);
}

int CDatabaseAF::PrivateCountRecords(LPCTSTR table, LPCTSTR filter, bool bBySys)
{
    CRecordsetAF rs;
    CString query;

    if (bBySys)
    {
        CString sysTable = _T("sysindexes");

        LPCTSTR ss = _tcsstr(table, _T(".."));
        if (ss != NULL) sysTable = CString(table, ss + 2 - table) + sysTable;

        query = CreateQuery(sysTable, _T("max(\"rows\")"),
            CString(_T("id = object_id('")) + table +
                _T("')")
                _T(" and indid in (0,1)"),
            strEmpty);
    }
    else
        query = CreateQuery(table, _T("count(*)"), filter, strEmpty);

    rs.Open(this, query, 1);
    if (rs.GetRowCount() == 0) return 0;

    const CColumn& column = rs.GetColumn(0);
    switch (column.GetType())
    {
        case dftTinyInt:
        case dftSmallInt:
        case dftInt:
            return column.GetInt(0);

        case dftBigInt:
            return int(column.GetInt64(0));

        case dftReal:
        case dftFloat:
            return AFLibMath::Rint(column.GetDouble(0));
    }

    return 0;
}

int CDatabaseAF::CountRecords(LPCTSTR table, LPCTSTR filter)
{
    if (m_bSysIndSupported && filter == NULL && !StringStartsWith(table, strPound))
    {
        try
        {
            return PrivateCountRecords(table, filter, true);
        }
        catch (CException* pExc)
        {
            pExc->Delete();
            m_bSysIndSupported = false;
        }
    }

    return PrivateCountRecords(table, filter, false);
}

void CDatabaseAF::ExecSQL(LPCTSTR query)
{
    TSQLStatement stm(this, false);
    CLock lock(this, false);
    stm.OpenExec(query);
}

void CDatabaseAF::ExecSQL(LPCTSTR query, TStringBuffer& logMsg)
{
    TSQLStatement stm(this, true);
    CLock lock(this, false);
    stm.OpenExec(query);
    logMsg += stm.m_logMsg;
}

void CDatabaseAF::ExecSQL(LPCTSTR query, AFLibGui::CDlgRun* pDlgRun)
{
    TSQLStatement stm(this, pDlgRun != NULL);
    stm.m_pDlgRun = pDlgRun;
    CLock lock(this, false);
    stm.OpenExec(query);
}

void CDatabaseAF::Use(LPCTSTR dbName)
{
    ExecSQL(strUse + dbName);
}

void CDatabaseAF::Truncate(LPCTSTR table)
{
    SAFE_CONST_STR(strTrunc, _T("truncate table "));
    SAFE_CONST_STR(strDelete, _T("delete from "));

    ExecSQL((m_bTruncateSupported ? strTrunc : strDelete) + table);
}

void CDatabaseAF::CheckFieldName(LPCTSTR fieldName)
{
    if (IsEmptyStr(fieldName) || _tcspbrk(fieldName, _T("()[]`'\"")) != NULL)
        ThrowMessage(LocalAfxString(IDS_BAD_FIELD_NAME, fieldName));
}
