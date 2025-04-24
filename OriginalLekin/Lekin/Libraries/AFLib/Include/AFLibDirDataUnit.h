#pragma once

#include "AFLibSDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TDirDataUnit
// String edit box containing a directory name

namespace AFLibGui {
class AFLIB TDirDataUnit : public TSDataUnit
{
private:
    typedef TSDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TDirDataUnit);

public:
    bool m_bCreateOk;  // is it OK to create the directory?

    TDirDataUnit(
        LPCTSTR key, UINT idc, CString& data, LPCTSTR init, LPCTSTR zero, bool bCreateOk, UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
