#pragma once

#include "AFLibIntDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TSpnDataUnit
// Integer edit box with a spin button

namespace AFLibGui {
class AFLIB TSpnDataUnit : public TIntDataUnit
{
private:
    typedef TIntDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TSpnDataUnit);

public:
    const UINT m_idcSpin;  // Control ID of the spin button

    TSpnDataUnit(LPCTSTR key,
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
