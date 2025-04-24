#pragma once

#include "DlgA.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgMenu dialog

class CDlgMenu : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_MENU
    };

    CBtnImg m_arrButton[8];

public:
    CDlgMenu();

    TWorkspace m_wkt;

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnClick(UINT id);

    DECLARE_MESSAGE_MAP()
};
