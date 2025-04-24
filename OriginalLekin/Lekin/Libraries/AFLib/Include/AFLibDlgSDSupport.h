#pragma once

#include "AFLibDlgTT.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgSDSupport
// Supports a Self-Destruct child.

namespace AFLibGui {
class CDlgSelfDestruct;

class AFLIB CDlgSDSupport : public CDlgTT
{
private:
    typedef CDlgTT super;
    DEFINE_COPY_AND_ASSIGN(CDlgSDSupport);

    // destroy m_pDlgSD
    void DestroyDlgSD();

protected:
    // self-destruct dialog (destructs itself on lost focus)
    CDlgSelfDestruct* m_pDlgSD;

    // show self-destruct dialog
    void ShowDlgSD(CDlgSelfDestruct* pDlg);

public:
    CDlgSDSupport();
    explicit CDlgSDSupport(UINT nIDTemplate, CWnd* pWndParent = NULL);

protected:
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CDlgSDSupport)
};
}  // namespace AFLibGui
