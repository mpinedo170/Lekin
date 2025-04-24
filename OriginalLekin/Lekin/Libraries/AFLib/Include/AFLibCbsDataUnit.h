#pragma once

#include "AFLibSDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TCbsDataUnit
// Combo-box string data unit

namespace AFLibGui {
class AFLIB TCbsDataUnit : public TSDataUnit
{
private:
    typedef TSDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TCbsDataUnit);

public:
    TCbsDataUnit(LPCTSTR key, UINT idc, CString& data, LPCTSTR init, LPCTSTR zero, UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
