#pragma once

#include "DlgLoadRS.h"
#include "LstWkc.h"
#include "Setup.h"

class TWorkcenter;

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadSetup dialog

class CDlgLoadSetup : public CDlgLoadRS
{
private:
    typedef CDlgLoadRS super;
    enum
    {
        IDD = IDD_LOAD_SETUP
    };

    CLstWkc m_lstWkc;

public:
    CDlgLoadSetup();

    TSetup m_setup;

protected:
    TWorkcenter* GetWkc(int i);

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnDblclkWkcId();
    afx_msg void OnSelchangeWkcId();

    virtual void FillListBox();
    virtual void LoadSelection();
    virtual void SaveSelection();

    DECLARE_MESSAGE_MAP()
};
