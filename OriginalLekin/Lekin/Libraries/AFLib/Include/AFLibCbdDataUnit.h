#pragma once

#include "AFLibIDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TCbiDataUnit
// Combo-box "item data" data unit

namespace AFLibGui {
class AFLIB TCbdDataUnit : public TIDataUnit
{
private:
    typedef TIDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TCbdDataUnit);

public:
    TCbdDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
