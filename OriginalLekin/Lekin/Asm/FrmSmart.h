#pragma once

#include "ViewSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CFrmSmart frame: used for all document windows

class CFrmSmart : public CMDIChildWnd
{
private:
    typedef CMDIChildWnd super;

protected:
    CFrmSmart();

public:
    CViewSmart* m_pView;
    CToolBar m_wndToolBar;
    CRect m_rectFrame;
    CRect m_rectView;

    void Redraw(bool bHor, bool bVer);
    void UpdateTitle();

public:
    virtual void RecalcLayout(BOOL bNotify = true);

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

    afx_msg void OnClose();
    afx_msg void OnPaint();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CFrmSmart)
};
