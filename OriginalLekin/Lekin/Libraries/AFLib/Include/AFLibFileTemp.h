#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TFileTemp
// Temporary file: deleted on closing

namespace AFLibIO {
class AFLIB TFileTemp : public CFile
{
private:
    typedef CFile super;
    DEFINE_COPY_AND_ASSIGN(TFileTemp);

    CString m_name;  // cached name (saved after Close())

public:
    TFileTemp(LPCTSTR name, bool bTemporary, int shareFlags);
    ~TFileTemp();

    // done writing, but keep the file from being deleted
    void ReleaseWritingRights();

    // close and delete the file
    virtual void CloseAndDelete();
};
}  // namespace AFLibIO
