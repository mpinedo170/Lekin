#pragma once

#include "DlgA.h"
#include "LstWkc.h"

class TWorkcenter;

/////////////////////////////////////////////////////////////////////////////
// CDlgFlowRoute dialog

class CDlgFlowRoute : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_FLOW_ROUTE
    };

    CLstWkc m_lstRoute;

public:
    explicit CDlgFlowRoute(TWorkcenter** route);
    TWorkcenter** m_route;

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()
};
