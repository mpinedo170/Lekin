#pragma once

#include "AFLibDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgDitemDetails dialog
// Dialog for editing a Ditem

namespace AFLibGui {
class TDitem;

class AFLIB CDlgDitemDetails : public CDlg
{
private:
    typedef CDlg super;
    DEFINE_COPY_AND_ASSIGN(CDlgDitemDetails);

public:
    CDlgDitemDetails();
    explicit CDlgDitemDetails(UINT nIDTemplate, CWnd* pWndParent = NULL);
    ~CDlgDitemDetails();

    CStringArray m_arrBadNames;  // prohibited (duplicate) names
    TDitem* m_pDitem;            // item being edited (i/o)

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibGui
