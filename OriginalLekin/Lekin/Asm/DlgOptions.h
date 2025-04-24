#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgOptions dialog

class CDlgOptions : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_OPTIONS
    };

    CCmbFont m_cmbFaceFont;

public:
    CDlgOptions();

    int m_sizeFont;
    CString m_faceFont;
    int m_confirmLevel;

protected:
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
