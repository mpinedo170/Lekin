#pragma once

#include "AFLibDDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TDblDataUnit
// Double edit box

namespace AFLibGui {
class AFLIB TDblDataUnit : public TDDataUnit
{
private:
    typedef TDDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TDblDataUnit);

public:
    double m_low;   // lower bound
    double m_high;  // upper bound

    TDblDataUnit(LPCTSTR key,
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
