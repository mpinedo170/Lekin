#pragma once

#include "DlgOppy.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRoute dialog

class CDlgRoute : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_ROUTE
    };

    enum
    {
        cmUp,
        cmDown,
        cmNew,
        cmDel,
        cmN
    };
    CBtnImg m_arrButton[cmN];
    CDlgOppy* m_pDlgInternal;
    void Disability();

public:
    CDlgRoute(TRoute& route);
    ~CDlgRoute();

    TRoute& m_route;

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnNew();
    afx_msg void OnDel();
    afx_msg void OnUp();
    afx_msg void OnDown();
    afx_msg void OnLoad();
    afx_msg void OnTimer(UINT nIDEvent);

    DECLARE_MESSAGE_MAP()
};
