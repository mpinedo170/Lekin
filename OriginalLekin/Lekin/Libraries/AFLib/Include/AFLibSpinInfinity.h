#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CSpinInfinity spin button
// Shows "infinity" if at upper limit

namespace AFLibGui {
class AFLIB CSpinInfinity : public CSpinButtonCtrl
{
private:
    typedef CSpinButtonCtrl super;
    DEFINE_COPY_AND_ASSIGN(CSpinInfinity);

public:
    CSpinInfinity();

    // infinity string (real infinity symbol if Unicode)
    static const TCHAR strInfinity[];

protected:
    afx_msg void OnUdnDeltaPos(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CSpinInfinity)
};
}  // namespace AFLibGui
