#pragma once

#include "AFLibIDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TIntDataUnit
// Integer edit box

namespace AFLibGui {
class AFLIB TIntDataUnit : public TIDataUnit
{
private:
    typedef TIDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TIntDataUnit);

public:
    int m_low;   // lower bound
    int m_high;  // upper bound

    TIntDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, int low, int high, UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
