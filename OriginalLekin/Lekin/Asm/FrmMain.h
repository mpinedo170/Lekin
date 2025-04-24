#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFrmMain frame: Main Frame

class CFrmMain : public CMDIFrameWnd
{
private:
    typedef CMDIFrameWnd super;

    TMenuBitmapper m_menuBitmapper;

public:
    CFrmMain();

    void InitBitmaps(CToolBar& wndToolBar);
    void InitPrintBitmap(CToolBar& wndToolBar);

    CRect m_winRect;
    CRect m_cliRect;
    CRect m_oldRect;

    virtual HMENU GetWindowMenuPopup(HMENU hMenuBar);

protected:
    CToolBar m_wndToolBar;
    CStatusBar m_wndStatusBar;

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void GetMessageString(UINT nID, CString& rMessage) const;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnClose();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

    DECLARE_DYNAMIC(CFrmMain)
    DECLARE_MESSAGE_MAP()
};
