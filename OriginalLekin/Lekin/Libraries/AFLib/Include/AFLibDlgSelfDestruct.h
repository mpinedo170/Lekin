#pragma once

#include "AFLibDlgTT.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgSelfDestruct dialog
// A modeless, focus-only window.

namespace AFLibGui {
class TBlockMBCounter;

class AFLIB CDlgSelfDestruct : public CDlgTT
{
private:
    typedef CDlgTT super;
    DEFINE_COPY_AND_ASSIGN(CDlgSelfDestruct);

    TBlockMBCounter* m_pBlock;  // blocks message boxes while dialog box is opened
    bool m_bParentDisabled;     // I had to disable this window's parent when a modal dlg box popped up

public:
    CDlgSelfDestruct();
    ~CDlgSelfDestruct();

    // create the dialog -- overwrite in derived classes
    virtual void SDCreate() = 0;

protected:
    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibGui
