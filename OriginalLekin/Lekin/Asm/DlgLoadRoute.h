#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadRoute dialog

#include "DlgLoadRS.h"
#include "LstLoadRoute.h"
#include "Route.h"

class CDlgLoadRoute : public CDlgLoadRS
{
private:
    typedef CDlgLoadRS super;
    enum
    {
        IDD = IDD_LOAD_ROUTE
    };

    CLstLoadRoute m_lstRoute;

public:
    CDlgLoadRoute();

    TRoute m_route;

protected:
    void GetRoute(int i, TRoute& route);

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnDblclkRoute();
    afx_msg void OnSelchangeRoute();

    virtual void FillListBox();
    virtual void SaveSelection();
    virtual void LoadSelection();

    DECLARE_MESSAGE_MAP()
};
