#pragma once

#include "AFLibDlgTT.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgListHelper dialog
// A modeless, focus-only window.
// Opens to edit a list box item.

namespace AFLibPrivate {
class CDlgListHelper : public AFLibGui::CDlgTT
{
private:
    typedef CDlgTT super;

    CEdit m_wndMain;     // main edit box
    CWnd* m_pWndParent;  // parent window
    CRect m_rect;        // rectangle, where to show

public:
    int m_index;     // item index in the list
    CString m_data;  // edited item

    explicit CDlgListHelper(CWnd* pWndParent);

    // create and show the dialog box
    void Create(const CRect& rect);

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibPrivate
