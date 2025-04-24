#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgObj1 dialog

class CDlgObj1 : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_OBJ1
    };

    int m_mask;

public:
    explicit CDlgObj1(int mask);

    TObjective m_Select;
    int m_Limit;
    bool m_bAskLimit;

protected:
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
