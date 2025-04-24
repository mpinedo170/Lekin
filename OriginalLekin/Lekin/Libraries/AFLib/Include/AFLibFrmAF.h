#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CFrmAF frame window
// Child frame with OnGetMinMaxInfo() and other handy functions

namespace AFLibGui {
class TViewEx;
class CViewAF;

class AFLIB CFrmAF : public CMDIChildWnd
{
private:
    typedef CMDIChildWnd super;
    friend TViewEx;
    DEFINE_COPY_AND_ASSIGN(CFrmAF);

    CViewAF* m_pView;  // the enclosed view

    // helper for OnGetMinMaxInfo()
    bool PrivateProcessGetMinMaxInfo(MINMAXINFO& MMI);

public:
    CFrmAF();

    // update frame title based on document state
    void UpdateTitle();

    // get the enclosed view
    CViewAF* GetView();

    // update and activate frame
    void Activate();

    // if frame contains print preview, close it
    void CallCloseProc();

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual CDocument* GetActiveDocument();

protected:
    // called from OnGetMinMaxInfo(); return false if cannot determine yet
    virtual bool GetMinMaxInfo(MINMAXINFO& MMI);

    afx_msg void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);
    afx_msg void OnTimer(UINT nIDEvent);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CFrmAF)
};
}  // namespace AFLibGui
