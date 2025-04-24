#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgObj2 dialog

class CDlgObj2 : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_OBJ2
    };

public:
    CDlgObj2();

    int m_Select;

    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
