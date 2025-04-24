#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSBDlg dialog

class CSBDlg : public CDialog
{
    // Construction
public:
    CSBDlg(CWnd* pParent = NULL);  // standard constructor

    // Dialog Data
    //{{AFX_DATA(CSBDlg)
    enum
    {
        IDD = IDD_SB_DIALOG
    };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSBDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
                                                      //}}AFX_VIRTUAL

    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    //{{AFX_MSG(CSBDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
