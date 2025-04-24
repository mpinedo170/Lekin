#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgInvalidate dialog

class CDlgInvalidate : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_INVALIDATE
    };

public:
    CDlgInvalidate();

    CString m_Prompt;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    afx_msg void OnOkAll();

    DECLARE_MESSAGE_MAP()
};
