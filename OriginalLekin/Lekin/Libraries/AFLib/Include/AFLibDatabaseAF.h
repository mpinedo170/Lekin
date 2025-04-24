#pragma once

#include "AFLibSemaLock.h"

namespace AFLibGui {
class CDlgRun;
}

/////////////////////////////////////////////////////////////////////////////
// CDatabaseAF
// ODBC Database that supports "light" recordsets & BCP.
// Once opened, can be used in multiple threads.

namespace AFLibDB {
class AFLIB CDatabaseAF : public CDatabase
{
private:
    typedef CDatabase super;
    DEFINE_COPY_AND_ASSIGN(CDatabaseAF);

    bool m_bBcpEnabled;         // BCP was enabled, but it does not mean it's supported
    bool m_bBcpSupported;       // BCP is supported for this database
    bool m_bBulkAddSupported;   // ODBC Bulk Add is supported for this database
    bool m_bTruncateSupported;  // Truncate table supported in this flavor of SQL?
    bool m_bTopSupported;       // Top n supported in this flavor of SQL?
    bool m_bSysIndSupported;    // sysindexes supported

    // semaphore to limit multithreaded access and preserve exclusivity of BCP
    AFLibThread::CSemaphoreAF* m_pSem;

    // clear all the "supported" flags
    void ClearFlags();

    // helper for CountRecords()
    int PrivateCountRecords(LPCTSTR table, LPCTSTR filter, bool bBySys);

public:
    // Lock class for preserving exclusivity
    class AFLIB CLock : public AFLibThread::CSemaLock
    {
    private:
        typedef AFLibThread::CSemaLock super;

    public:
        CLock(CDatabaseAF* pDB, bool bExclusive);
    };

    friend CLock;

    CDatabaseAF();
    ~CDatabaseAF();

    // try to enable BCP (call before opening the database)
    bool EnableBcp();

    // is BCP supported?
    bool IsBcpSupported() const
    {
        return m_bBcpSupported;
    }

    // is Add supported in SQLBulkOperations()?
    bool IsBulkAddSupported() const
    {
        return m_bBulkAddSupported;
    }

    // is "top n" supported in this flavor of SQL?
    bool IsTopSupported() const
    {
        return m_bTopSupported;
    }

    // can new query run now?
    bool IsAvailable(bool bExclusive) const;

    // Build count(*) query and run it; optimize through sysindexes if possible
    int CountRecords(LPCTSTR table, LPCTSTR filter = NULL);

    // execute SQL query (may be breakable from other thread)
    void ExecSQL(LPCTSTR query);

    // execute SQL query, read messages into arrMsg
    void ExecSQL(LPCTSTR query, AFLibIO::TStringBuffer& logMsg);

    // execute SQL query, interactively send messages to pDlgRun
    void ExecSQL(LPCTSTR query, AFLibGui::CDlgRun* pDlgRun);

    // set default database
    void Use(LPCTSTR dbName);

    // truncate table
    void Truncate(LPCTSTR table);

    virtual BOOL OpenEx(LPCTSTR lpszConnectString, DWORD dwOptions = 0);
    virtual void Close();

    // check field name for a table, throw exception if invalid
    static void CheckFieldName(LPCTSTR fieldName);
};
}  // namespace AFLibDB
