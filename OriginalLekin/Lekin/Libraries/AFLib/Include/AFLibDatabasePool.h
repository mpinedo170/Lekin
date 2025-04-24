#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"
#include "AFLibThread.h"

/////////////////////////////////////////////////////////////////////////////
// CDatabasePool
// A pool of connections to the same database.
// Used to speed up multi-threaded execution.
// Opens a new connection for every calling thread.

namespace AFLibDB {
class CDatabaseAF;

class AFLIB CDatabasePool
{
private:
    DEFINE_COPY_AND_ASSIGN(CDatabasePool);

protected:
    bool m_bBcpEnabled;      // BCP was enabled
    DWORD m_dwQueryTimeout;  // query timeout
    CString m_dbName;        // default DB name

    // data for every thread
    struct TThreadData
    {
        AFLibThread::CThread m_thread;  // thread ID and handle
        CDatabaseAF* m_pDB;             // database connection
    };

    CString m_lpszConnect;       // connection parameters
    CArray<TThreadData> m_data;  // data for all threads (sorted)
    CCriticalSection* m_pSync;   // synchronization object

    // opens a database connection
    virtual bool ProtectedOpen(CDatabaseAF* pDB, LPCTSTR lpszConnect);

    // helper for ProtectedGetDBThrow(): tries to open the connection, returns NULL if can't
    virtual CDatabaseAF* ProtectedGetDB(bool bForceReopen);

    // helper for GetDB() and RegetDB(): throws if can't open connection
    CDatabaseAF* ProtectedGetDBThrow(bool bForceReopen);

public:
    CDatabasePool();
    virtual ~CDatabasePool();

    // try to enable BCP (call before opening the database)
    bool EnableBcp();

    // set query timeout in seconds (call before opening the database)
    void SetQueryTimeout(DWORD dwSeconds);

    // set default DB name (call before opening the database)
    void SetDefaultDB(LPCTSTR dbName);

    // open the pool
    virtual bool Open(LPCTSTR lpszConnect);

    // is the pool open?
    virtual bool IsOpen();

    // get the number of connections
    virtual int GetPoolSize();

    // close all connections
    virtual void Close();

    // closes connections for finished threads
    virtual void Cleanup();

    // get free DB connection
    CDatabaseAF* GetDB()
    {
        return ProtectedGetDBThrow(false);
    }

    // get free DB connection
    CDatabaseAF* RegetDB()
    {
        return ProtectedGetDBThrow(true);
    }
};
}  // namespace AFLibDB
