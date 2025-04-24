#pragma once

#include "BtnStyle.h"
#include "DlgA.h"
#include "LstSeq.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgJob dialog

class CDlgJob : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_JOB
    };

    CBtnStyle m_btnStyle;
    CLstSeq m_lstSeq;

    void Modify();
    void Recompute();
    bool TestFocus();

public:
    CDlgJob();

    int m_due;
    int m_release;
    int m_start;
    int m_stop;
    int m_tardiness;
    int m_weight;
    int m_WT;

    TOperation* m_pOp;

    void FillData(TOperation* pOpNew = NULL);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnCancel();
    afx_msg void OnSelchangeRoute();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnKillfocusDue();
    afx_msg void OnKillfocusWeight();
    afx_msg void OnKillfocusRelease();

    DECLARE_MESSAGE_MAP()
};
