#pragma once

#include "AFLibIDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TCbiDataUnit
// Combo-box index data unit

namespace AFLibGui {
class AFLIB TCbiDataUnit : public TIDataUnit
{
private:
    typedef TIDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TCbiDataUnit);

public:
    TCbiDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
