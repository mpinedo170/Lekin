#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgParameter dialog

class CDlgParameter : public CDlgA
{
protected:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_PARAMETER
    };

public:
    CDlgParameter();

    double m_k1;
    double m_k2;

    DECLARE_MESSAGE_MAP()
};
