#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPrintMan dialog

class CDlgPrintMan : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_PRINT_MAN
    };

public:
    CDlgPrintMan();

    int m_Win;
    int m_All;

    void DoPrint();

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeSel(UINT nIDC);
    afx_msg void OnPageSetup(UINT nIDC);

    DECLARE_MESSAGE_MAP()
};
