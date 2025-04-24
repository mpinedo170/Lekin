#pragma once

#include "AFLibDefine.h"
#include "AFLibStringBuffer.h"

class CCriticalSection;

/////////////////////////////////////////////////////////////////////////////
// TLog
// Creates a log and posts data to it

namespace AFLibIO {
class AFLIB TLog
{
private:
    DEFINE_COPY_AND_ASSIGN(TLog);

    const CString m_path;       // log file path; if empty, log in memory, do not flush onto disk
    const bool m_bTimed;        // put time stamps in the log?
    int m_bufSize;              // max buffer size
    TStringBuffer m_buffer;     // buffer (to avoid opening and closing the file too often)
    CCriticalSection* m_pSync;  // syncronization object for the log (may be null)

    // class for automatic cleanup
    struct TInit
    {
        ~TInit();
    };

    static TLog* m_pLogObj;             // "standard" log
    static CCriticalSection m_syncObj;  // sync object for the m_pLogObj
    static TInit m_init;                // static object whose destructor kills m_pLogObj

    // helper for Write()
    bool PrivateWrite(LPCTSTR line);

    // helper for Flush()
    bool PrivateFlush();

    // helper for FlushInto()
    void FlushIntoHelper(TStringBuffer& sbDest);

    // helper for Clear()
    void PrivateClear();

public:
    TLog(LPCTSTR path, bool bTimed, bool bSync);
    ~TLog();

    // create a "standard" log
    static TLog& Get();

    // set buffer size (before data is flushed into a file)
    void SetBufferSize(int bufSize);

    // post data into the log
    bool Write(LPCTSTR line);

    // flush the buffer
    bool Flush();

    // flush into a different log
    void FlushInto(TLog& log2);

    // clear the log
    void Clear();
};
}  // namespace AFLibIO

// for debugging: log with the file and line number
#define LineLog(line) TLog::Get().Write(CString(__FILE__) + _T(", ") + IntToStr(__LINE__) + _T(": ") + (line))
