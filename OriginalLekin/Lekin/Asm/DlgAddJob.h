#pragma once

#include "BtnStyle.h"
#include "DlgA.h"

#include "Route.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgAddJob dialog: Add/Edit job

class CDlgAddJob : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_ADD_JOB
    };

    TMode m_mode;
    CEdit m_editProc;
    CBtnTxt m_btnRoute;
    CBtnTxt m_btnLoad;
    CString m_idOld;
    CRect m_rectRoute;
    bool m_bSingle;

public:
    CDlgAddJob();

    int m_jobCount;
    int m_due;
    int m_weight;
    int m_release;
    int m_proc;
    CString m_status;

    CBtnStyle m_btnStyle;
    TRoute m_route;

    void AddJob(TJob& job);
    void ReadJob(TJob& job);

    CString& JobID()
    {
        return m_btnStyle.m_style.m_id;
    }
    CString& JobName()
    {
        return m_btnStyle.m_style.m_comment;
    }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeJobCount();
    afx_msg void OnLoad();
    virtual void OnOK();
    afx_msg void OnRoute();
    afx_msg void OnPaint();

    virtual bool IsDUNeeded(const TDataUnit* pDu);
    virtual bool IsDUMultiSel(const TDataUnit* pDu);
    virtual void SaveDU(const TDataUnit* pDu);

    bool Route1();
    void UpdateProcessing();

    DECLARE_MESSAGE_MAP()
};
