#pragma once

#include "DlgA.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgWorkspace dialog

class CDlgWorkspace : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_WORKSPACE
    };

    bool m_bRouteChanged;

public:
    CDlgWorkspace();

    TWorkspace m_Workspace;
    bool m_bOldFlex;
    TShop m_oldShop;
    TWorkcenterArray m_route;

protected:
    afx_msg void OnRoute();
    virtual void OnOK();
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
