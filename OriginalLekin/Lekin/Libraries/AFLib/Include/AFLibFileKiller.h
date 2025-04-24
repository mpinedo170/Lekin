#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TFileKiller
// Recursively deletes files

namespace AFLibIO {
class AFLIB TFileKiller
{
private:
    DEFINE_COPY_AND_ASSIGN(TFileKiller);

    CString m_path;    // path where to delete all files
    bool m_bKillThis;  // delete m_path itself?

    // recursive function to delete files
    void PrivateRun(const CString& path, double start, double step, bool bKillThis);

public:
    TFileKiller(LPCTSTR path, bool bKillThis);
    virtual ~TFileKiller();

    // does the work; returns false if interrupted
    bool Run();

    // get topmost deleted file
    const CString& GetPath()
    {
        return m_path;
    }

protected:
    // callback function -- called after deleting each file or dir
    virtual void Callback(LPCTSTR path, double progress);
};
}  // namespace AFLibIO
