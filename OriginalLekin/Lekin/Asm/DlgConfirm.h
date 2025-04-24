#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfirm dialog

class CDlgConfirm : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_CONFIRM
    };

    HICON m_hIcon;

public:
    CDlgConfirm();

    CString m_prompt;
    bool m_bConfirm;
    int m_level;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    afx_msg void OnNo();
    afx_msg void OnYes();
    afx_msg void OnClose();
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
