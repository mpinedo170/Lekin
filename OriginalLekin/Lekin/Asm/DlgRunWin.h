#pragma once

#include "DlgRunAlg.h"
#include "Plugin.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunWin dialog

class CDlgRunWin : public CDlgRunAlg
{
private:
    typedef CDlgRunAlg super;
    enum
    {
        IDD = IDD_RUN_WIN
    };

public:
    CDlgRunWin();
};
