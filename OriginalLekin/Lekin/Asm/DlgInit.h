#pragma once

#include "DlgA.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgInit dialog

class CDlgInit : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_INIT
    };

    TWorkspace m_wkt;

public:
    CDlgInit();

    int m_jobCount;
    int m_mchCount;

protected:
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
