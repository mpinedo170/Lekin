#pragma once

#include "BtnStyle.h"
#include "DlgA.h"
#include "Setup.h"

class TWorkcenter;

/////////////////////////////////////////////////////////////////////////////
// CDlgAddWkc dialog: Add/Edit workcenter

class CDlgAddWkc : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_ADD_WKC
    };

    CString m_idOld;
    TMode m_mode;
    int m_wkcCount;
    bool m_bSetupChanged;
    bool m_bIDReadOnly;

public:
    CDlgAddWkc();

    CBtnStyle m_btnStyle;

    int m_mchCount;
    int m_release;
    CString m_status;

    TSetup m_setup;

    void AddWkc(TWorkcenter& wkc);
    void ReadWkc(TWorkcenter& wkc);

    CString& WkcID()
    {
        return m_btnStyle.m_style.m_id;
    }
    CString& WkcName()
    {
        return m_btnStyle.m_style.m_comment;
    }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnSetup();
    virtual void OnOK();

    virtual bool IsDUNeeded(const TDataUnit* pDu);
    virtual bool IsDUMultiSel(const TDataUnit* pDu);
    virtual void SaveDU(const TDataUnit* pDu);

    DECLARE_MESSAGE_MAP()
};
