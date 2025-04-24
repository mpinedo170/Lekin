#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CEditCode edit box
// Edits source code

namespace AFLibGui {
class AFLIB CEditCode : public CEdit
{
private:
    typedef CEdit super;
    DEFINE_COPY_AND_ASSIGN(CEditCode);

    CFont m_fontCourier;    // fixed-width font for the editor
    CWnd* m_pWndLabel;      // the window to show cursor position
    UINT m_idcLabel;        // identifier for the window to show cursor position
    CString m_cursorLabel;  // label with the cursor position

    // set the text of the cursor position window
    void FlushCursorLabel();

    // change cursor label
    void SetCursorLabel(int xPos, int yPos);

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
    CEditCode();

    // set the window to show cursor position
    void SetCursorLabelWnd(CWnd* pWndLabel);
    void SetCursorLabelWnd(UINT idcLabel);

    // get window text, trim trailing space
    CString GetTrimmedText();

    // overridable initialization function
    virtual void OnInit();

    virtual void PreSubclassWindow();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg LRESULT OnPaste(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibGui
