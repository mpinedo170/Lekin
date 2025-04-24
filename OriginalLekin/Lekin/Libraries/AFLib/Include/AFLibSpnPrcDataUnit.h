#pragma once

#include "AFLibPrcDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TSpnPrcDataUnit
// Percentage edit box with a spin button

namespace AFLibGui {
class AFLIB TSpnPrcDataUnit : public TPrcDataUnit
{
private:
    typedef TPrcDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TSpnPrcDataUnit);

public:
    const UINT m_idcSpin;  // Control ID of the spin button

    TSpnPrcDataUnit(LPCTSTR key,
        UINT idc,
        UINT idcSpin,
        double& data,
        double init,
        double zero,
        double low,
        double high,
        UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
