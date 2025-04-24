#pragma once

#include "AFLibIDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TMskDataUnit
// Set of check boxes

namespace AFLibGui {
class AFLIB TMskDataUnit : public TIDataUnit
{
private:
    typedef TIDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TMskDataUnit);

public:
    // error to show if all checks are unselected; default=0, do not throw
    int m_errorID;

    TMskDataUnit(
        LPCTSTR key, UINT idc, int& data, int init, int zero, int count, UINT errorID = 0, UINT idsHelp = 0);

    virtual bool PeerNeeded(int index) const;
    void DDX(CDataExchange* pDX, bool bUndefOk) const;
    virtual bool IsUndefined() const;
};
}  // namespace AFLibGui
