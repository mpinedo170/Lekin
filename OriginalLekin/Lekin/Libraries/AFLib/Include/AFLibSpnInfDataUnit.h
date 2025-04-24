#pragma once

#include "AFLibIntDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TSpnInfDataUnit
// Integer edit box with an "infinity" spin button

namespace AFLibGui {
class AFLIB TSpnInfDataUnit : public TIntDataUnit
{
private:
    typedef TIntDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TSpnInfDataUnit);

public:
    const UINT m_idcSpin;  // Control ID of the spin button

    TSpnInfDataUnit(LPCTSTR key,
        UINT idc,
        UINT idcSpin,
        int& data,
        int init,
        int zero,
        int low,
        int high,
        UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
