#pragma once

#include "AFLibDlgSelfDestruct.h"

namespace AFLibGui {
class CDlgRun;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgConfirm dialog
// A modeless, focus-only window.
// Opens to edit a list box item.

namespace AFLibPrivate {
class CDlgConfirm : public AFLibGui::CDlgSelfDestruct
{
private:
    typedef CDlgSelfDestruct super;

    AFLibGui::CDlgRun* m_pDlgParent;  // parent dialog box

public:
    explicit CDlgConfirm(AFLibGui::CDlgRun* pDlgParent);

protected:
    virtual void SDCreate();
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibPrivate
