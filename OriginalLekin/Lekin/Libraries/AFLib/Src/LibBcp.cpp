#include "StdAfx.h"

#include "LibBcp.h"

using AFLibPrivate::TLibBcp;

namespace {
const LPCSTR arrSqlDriver[][2] = {
    {"SQL Server Native Client 10.0", "sqlncli10.dll"}, {"SQL Native Client", "sqlncli.dll"}, {NULL}};
}

TLibBcp TLibBcp::m_a;

TLibBcp::TLibBcp()
{
    m_hInstLib = NULL;
    m_pfBcpInit = NULL;
    m_pfBcpBind = NULL;
    m_pfBcpBatch = NULL;
    m_pfBcpDone = NULL;
    m_pfBcpSendRow = NULL;
}

TLibBcp::~TLibBcp()
{
    if (m_hInstLib != NULL) FreeLibrary(m_hInstLib);
}

bool TLibBcp::InitLibrary()
{
    if (m_hInstLib != NULL) return true;

    for (int i = 0; arrSqlDriver[i][0] != NULL; ++i)
    {
        m_hInstLib = LoadLibraryA(arrSqlDriver[i][1]);
        if (m_hInstLib == NULL) continue;

#ifdef _UNICODE
        static const LPCSTR procName = "bcp_initW";
#else
        static const LPCSTR procName = "bcp_initA";
#endif

        m_pfBcpInit = reinterpret_cast<RETCODE(SQL_API*)(HDBC, LPCTSTR, LPCTSTR, LPCTSTR, INT)>(
            GetProcAddress(m_hInstLib, procName));

        m_pfBcpBind = reinterpret_cast<RETCODE(SQL_API*)(HDBC, LPCBYTE, INT, DBINT, LPCBYTE, INT, INT, INT)>(
            GetProcAddress(m_hInstLib, "bcp_bind"));

        m_pfBcpBatch = reinterpret_cast<DBINT(SQL_API*)(HDBC)>(GetProcAddress(m_hInstLib, "bcp_batch"));

        m_pfBcpDone = reinterpret_cast<DBINT(SQL_API*)(HDBC)>(GetProcAddress(m_hInstLib, "bcp_done"));

        m_pfBcpSendRow = reinterpret_cast<RETCODE(SQL_API*)(HDBC)>(GetProcAddress(m_hInstLib, "bcp_sendrow"));

        if (m_pfBcpInit == NULL || m_pfBcpBind == NULL || m_pfBcpBatch == NULL || m_pfBcpDone == NULL ||
            m_pfBcpSendRow == NULL)
        {
            FreeLibrary(m_hInstLib);
            m_hInstLib = NULL;
            continue;
        }

        m_driverTitle = arrSqlDriver[i][0];
        return true;
    }

    return false;
}

RETCODE TLibBcp::Init(HDBC hdbc, LPCTSTR table)
{
    ASSERT(m_hInstLib != NULL);
    return m_pfBcpInit(hdbc, table, NULL, NULL, DB_IN);
}

RETCODE TLibBcp::Bind(HDBC hdbc, LPCVOID pData, INT eDataType, INT idxServerCol)
{
    ASSERT(m_hInstLib != NULL);
    return m_pfBcpBind(hdbc, reinterpret_cast<LPCBYTE>(pData), sizeof(SQLLEN), SQL_VARLEN_DATA, NULL, 0,
        eDataType, idxServerCol);
}
