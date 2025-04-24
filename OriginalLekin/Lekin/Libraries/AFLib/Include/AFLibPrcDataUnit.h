#pragma once

#include "AFLibDDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TPrcDataUnit
// Double edit box (percentage value)

namespace AFLibGui {
class AFLIB TPrcDataUnit : public TDDataUnit
{
private:
    typedef TDDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TPrcDataUnit);

public:
    double m_low;   // lower bound
    double m_high;  // upper bound

    TPrcDataUnit(LPCTSTR key,
        UINT idc,
        double& data,
        double init,
        double zero,
        double low,
        double high,
        UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
