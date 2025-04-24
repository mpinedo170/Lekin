#pragma once

#include "AFLibLCBDraw.h"

class CToolTipCtrl;

/////////////////////////////////////////////////////////////////////////////
// CBtnOwner
// Owner-draw button.  Provides a standard button outline picture.
// The user has to draw the insides of the button.

namespace AFLibGui {
class AFLIB CBtnOwner : public CButton
{
private:
    typedef CButton super;
    DEFINE_COPY_AND_ASSIGN(CBtnOwner);

    CToolTipCtrl* m_pToolTip;  // tool tip control
    CString m_tip;             // tool tip text
    bool m_bCheckbox;          // check box behavior?

    void InitToolTip();  // initialize tool tip control

public:
    CBtnOwner();
    ~CBtnOwner();

    // start or end checkbox behavior
    void SetCheckbox(bool bCheckbox, bool bRedraw = true);

    // does the button behave as checkbox?
    bool IsCheckbox()
    {
        return m_bCheckbox;
    }

    // enable or disable tool tip for the button
    virtual void EnableButtonToolTip(bool bEnable);

    // is tool tip enabled?
    bool IsButtonToolTipEnabled();

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    TLCBDraw m_draw;  // Drawing information

    afx_msg LRESULT OnBmGetCheck(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnBmSetCheck(WPARAM wParam, LPARAM lParam);

    virtual void PreSubclassWindow();
    int OnCreate(LPCREATESTRUCT lpCreateStruct);

    virtual void OnInit();        // called in Subclass and Create
    virtual void GetItemText();   // get button text
    virtual void Draw(CDC* pDC);  // DrawItem wrapper

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CBtnOwner)
};
}  // namespace AFLibGui
