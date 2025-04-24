#pragma once

#include "DlgA.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgTitle dialog

class CDlgTitle : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_TITLE
    };

    CImageAF m_imgStern;
    CStatic m_wndStern;
    CStatic m_wndBlink;
    bool m_bBlinking;
    int m_ticks;

public:
    CDlgTitle();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDEvent);
    virtual void OnCancel();
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};
