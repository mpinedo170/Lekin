#include "StdAfx.h"

#include "AFLibDBGlobal.h"

#include "AFLibRecordsetParam.h"
#include "AFLibStringBuffer.h"
#include "LibBcp.h"
#include "Local.h"

#undef DBCHAR
#include <odbcss.h>  // SQL Server functions; includes WRONG definition of LPCBYTE!

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;

namespace {  // structure with information about each supported data type
struct TTypeDef
{
    UINT m_ids;         // IDS in resources for the name of the type
    int m_rawODBCType;  // default ODBC type
    int m_rawCType;     // corresponding C type
    int m_rawBCPType;   // SQL Server BCP type
};

TTypeDef arrTypeDef[] = {{IDS_BAD_TYPE, SQL_UNKNOWN_TYPE, SQL_C_DEFAULT, SQLVARIANT},
    {IDS_BIT, SQL_BIT, SQL_C_TINYINT, SQLINT1}, {IDS_TINYINT, SQL_TINYINT, SQL_C_TINYINT, SQLINT1},
    {IDS_SMALLINT, SQL_SMALLINT, SQL_C_SHORT, SQLINT2}, {IDS_INT, SQL_INTEGER, SQL_C_LONG, SQLINT4},
    {IDS_INT64, SQL_BIGINT, SQL_C_SBIGINT, SQLINT8}, {IDS_REAL, SQL_REAL, SQL_C_FLOAT, SQLFLT4},
    {IDS_FLOAT, SQL_DOUBLE, SQL_C_DOUBLE, SQLFLT8}, {IDS_DATE, SQL_DATE, SQL_C_TIMESTAMP, SQLDATETIME},
    {IDS_ASCII, SQL_VARCHAR, SQL_C_CHAR, SQLVARCHAR}, {IDS_UNICODE, SQL_WVARCHAR, SQL_C_WCHAR, SQLNVARCHAR},
    {IDS_BINARY, SQL_BINARY, SQL_C_BINARY, SQLVARBINARY}};

AFLibDB::TRecordsetParam paramDefault;
CCriticalSection syncParamDefault;

void InitParamDefault()
{
    if (paramDefault.m_bSmartRows == t3Undef)
    {
        CSingleLock lock(&syncParamDefault, true);

        // recheck: maybe was already initialized!
        if (paramDefault.m_bSmartRows == t3Undef) paramDefault.Default();
    }
}
}  // namespace

namespace AFLibDB {
const CString& GetSQLServerDriver()
{
    TLibBcp::m_a.InitLibrary();
    return TLibBcp::m_a.GetDriverTitle();
}

CString DBFieldTypeToStr(TDBFieldType dft)
{
    return LocalAfxString(arrTypeDef[dft].m_ids);
}

int DBFieldTypeToODBCType(TDBFieldType dft)
{
    return arrTypeDef[dft].m_rawODBCType;
}

int DBFieldTypeToCType(TDBFieldType dft)
{
    return arrTypeDef[dft].m_rawCType;
}

int DBFieldTypeToBCPType(TDBFieldType dft)
{
    return arrTypeDef[dft].m_rawBCPType;
}

CString CreateQuery(LPCTSTR table, const CString& fields, const CString& filter, const CString& order)
{
    TStringBuffer query;
    query += strSelect;

    if (fields.IsEmpty())
        query += strStar;
    else
        query += fields;

    query += strFrom;
    query += table;

    if (!filter.IsEmpty())
    {
        query += strWhere;
        query += filter;
    }

    if (!order.IsEmpty())
    {
        query += strOrderBy;
        query += order;
    }

    return LPCTSTR(query);
}

const TRecordsetParam& GetDefaultRecordsetParam()
{
    InitParamDefault();
    return paramDefault;
}

void SetDefaultRecordsetParam(const TRecordsetParam& param)
{
    InitParamDefault();
    CSingleLock lock(&syncParamDefault, true);
    paramDefault.Change(param);
}

void EnableDefaultSmartRows(bool bEnable)
{
    TRecordsetParam param;
    param.m_bSmartRows = bEnable;
    SetDefaultRecordsetParam(param);
}

void SetDefaultBulkRows(int nBulk)
{
    TRecordsetParam param;
    param.m_nBulk = nBulk;
    SetDefaultRecordsetParam(param);
}
}  // namespace AFLibDB
