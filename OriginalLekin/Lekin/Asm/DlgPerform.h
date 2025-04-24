#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPerform dialog

class CDlgPerform : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_PERFORMANCE
    };

public:
    CDlgPerform();

    void FillData();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
};
