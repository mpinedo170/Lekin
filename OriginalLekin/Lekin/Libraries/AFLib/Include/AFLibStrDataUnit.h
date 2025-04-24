#pragma once

#include "AFLibSDataUnit.h"

/////////////////////////////////////////////////////////////////////////////
// TStrDataUnit
// String edit box

namespace AFLibGui {
class AFLIB TStrDataUnit : public TSDataUnit
{
private:
    typedef TSDataUnit super;
    DEFINE_COPY_AND_ASSIGN(TStrDataUnit);

public:
    int m_low;     // min number of characters
    int m_high;    // max number of characters
    bool m_bTrim;  // trim leading and trailing spaces

    TStrDataUnit(LPCTSTR key,
        UINT idc,
        CString& data,
        LPCTSTR init,
        LPCTSTR zero,
        int low,
        int high,
        bool bTrim = false,
        UINT idsHelp = 0);

    virtual void DDX(CDataExchange* pDX, bool bUndefOk) const;
};
}  // namespace AFLibGui
