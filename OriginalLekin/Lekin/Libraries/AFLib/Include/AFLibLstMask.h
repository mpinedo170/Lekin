#pragma once

#include "AFLibLstOwner.h"

/////////////////////////////////////////////////////////////////////////////
// CLstMask list box
// Single-selection list box where some items may be disabled

namespace AFLibGui {
class AFLIB CLstMask : public CLstOwner
{
private:
    typedef CLstOwner super;
    DEFINE_COPY_AND_ASSIGN(CLstMask);

    // search for an enabled item in a given direction
    int Search(int iFrom, int iTo, int d);

    // set current selection
    void PrivateSetCurSel(int index);

public:
    CLstMask();

    // check if selected item is disabled; move selector if necessary
    void FixSelection();

protected:
    bool m_bAlwaysSelect;  // never leave selection empty

    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg LRESULT OnLbSetCurSel(WPARAM wParam, LPARAM lParam);
    virtual void OnInit();
    virtual void Draw(CDC* pDC);

    // is item enabled?
    virtual bool IsEnabled(int index);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CLstMask)
};
}  // namespace AFLibGui
