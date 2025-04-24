#pragma once

#include "AFLibIDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TRadDataUnit
// Radio button

namespace AFLibGui {
class AFLIB TRadDataUnit : public TIDataUnit
{
private:
    typedef TIDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TRadDataUnit);

public:
    TRadDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, int count, UINT idsHelp = 0);

    virtual bool PeerNeeded(int index) const;
    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
