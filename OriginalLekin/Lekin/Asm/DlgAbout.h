#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgAbout dialog

class CDlgAbout : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_ABOUT
    };

    CButton m_btnShowAgain;
    CButton m_btnOk;
    int m_marginX, m_marginY;
    bool m_bShowAgain;

    CSize Draw(CDC* pDC);

public:
    CDlgAbout();

    bool m_bShowAnyway;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();

    DECLARE_MESSAGE_MAP()
};
