#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlg3Button dialog: dialog with 3 buttons

class CDlg3Button : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_CLEAR_SEQ
    };

public:
    CDlg3Button(int idd);

    bool m_bNormal;

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnNo();

    DECLARE_MESSAGE_MAP()
};
