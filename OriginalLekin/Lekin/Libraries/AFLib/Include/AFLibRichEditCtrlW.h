#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlW
// Rich Edit control supporting wide (2-byte) characters (Unicode).
// If program is compiled as Unicode, equivalent to CRichEditCtrl;
// If program is compiled as Ascii, uses special techniques.
// Also provides right-click menu.

namespace AFLibGui {
class AFLIB CRichEditCtrlW : public CRichEditCtrl
{
private:
    typedef CRichEditCtrl super;
    DEFINE_COPY_AND_ASSIGN(CRichEditCtrlW);

    // popup menu types
    enum TRightClickType
    {
        tNone,
        tRegular,
        tParagraph
    };

    TRightClickType m_type;  // what happens on right-click?

public:
    CRichEditCtrlW();

    // is entire text selected?
    bool IsAllSelected();

    // is nothing selected?
    bool IsAllUnselected();

    // set gray background (nice if the control is read-only)
    void SetGray();

    // disable the right-click popup menu
    void DisablePopupMenu();

    // enable the right-click popup menu (may enable "SelectParagraph" comamnd)
    void EnablePopupMenu(bool bEnablePar);

protected:
    // overridable: add or delete some menu items if necessary; return false to cancel
    virtual bool ModifyPopupMenu(CMenu* pMenu);

    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnEditTop();
    afx_msg void OnEditBottom();
    afx_msg void OnEditSelectAll();
    afx_msg void OnEditSelectPar();
    afx_msg void OnEditCopy();

    DECLARE_MESSAGE_MAP()

#ifndef _UNICODE
private:
    // tests if this is a Unicode window;
    // if not, makes it Unicode by calling SetWindowLongPtrW
    bool TestUnicode();

public:
    // overwrites the default window procedure to call TestUnicode()
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    // creates window (as Unicode!)
    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

    // subclass window is not a virtual function, so we are not sure
    // the following functions will be called.  But that's OK, since
    // the window will be converted to Unicode as soon as DefWindowProc()
    // is called

    BOOL SubclassWindow(HWND hWnd);
    BOOL SubclassDlgItem(UINT nID, CWnd* pWndParent);
#endif
};
}  // namespace AFLibGui
