#pragma once

#include "AFLibDefine.h"
#include "AFLibPointers.h"
#undef DBCHAR
#include <odbcss.h>  // SQL Server functions; includes WRONG definition of LPCBYTE!

/////////////////////////////////////////////////////////////////////////////
// TLibBcp
// Helper class for loading the SQL Server Native Client DLL.
// This makes AFLib.dll work on PCs without sqlncli.dll

namespace AFLibPrivate {
class TLibBcp
{
private:
    DEFINE_COPY_AND_ASSIGN(TLibBcp);

    HINSTANCE m_hInstLib;   // library instance handle
    CString m_driverTitle;  // title of the Native Client driver

    // pointers to library functions
    RETCODE(SQL_API* m_pfBcpInit)(HDBC, LPCTSTR, LPCTSTR, LPCTSTR, INT);
    RETCODE(SQL_API* m_pfBcpBind)(HDBC, LPCBYTE, INT, DBINT, LPCBYTE, INT, INT, INT);
    DBINT(SQL_API* m_pfBcpBatch)(HDBC);
    DBINT(SQL_API* m_pfBcpDone)(HDBC);
    RETCODE(SQL_API* m_pfBcpSendRow)(HDBC);

public:
    TLibBcp();
    ~TLibBcp();

    // open the library handle and fill function pointers
    bool InitLibrary();

    // call BcpInit()
    RETCODE Init(HDBC hdbc, LPCTSTR table);

    // call BcpBind()
    RETCODE Bind(HDBC hdbc, LPCVOID pData, INT eDataType, INT idxServerCol);

    // call BcpBatch()
    DBINT Batch(HDBC hdbc)
    {
        ASSERT(m_hInstLib != NULL);
        return m_pfBcpBatch(hdbc);
    }

    // call BcpDone()
    DBINT Done(HDBC hdbc)
    {
        ASSERT(m_hInstLib != NULL);
        return m_pfBcpDone(hdbc);
    }

    // call BcpSendRow()
    RETCODE SendRow(HDBC hdbc)
    {
        ASSERT(m_hInstLib != NULL);
        return m_pfBcpSendRow(hdbc);
    }

    // get the driver title
    const CString& GetDriverTitle()
    {
        return m_driverTitle;
    }

    // one and only object
    static TLibBcp m_a;
};
}  // namespace AFLibPrivate
