#pragma once

class TMarginGantt;

/////////////////////////////////////////////////////////////////////////////
// CDlgPrintGantt dialog

class CDlgPrintGantt : public CDlgPrint
{
private:
    typedef CDlgPrint super;
    enum
    {
        IDD = IDD_PRINT_GANTT
    };

    TMarginGantt* GetMar();  // (TMarginGantt*)m_pMarginCur
    void EnableNoSplit();

    CSpinButtonCtrl m_spinPagesX;  // pages per row
    CSpinButtonCtrl m_spinPagesY;  // pages per column
    bool m_bCatch;                 // catch edit box changes?  E.g., during initialization -- no.

public:
    CDlgPrintGantt(bool bSetupOnly, CWnd* pParent = NULL);

protected:
    afx_msg void OnChange2();
    virtual BOOL OnInitDialog();

    virtual CString GetToolTip(UINT idc);
    virtual UINT SaveAdditionalData();
    virtual void LoadAdditionalData();

    DECLARE_MESSAGE_MAP()
};
