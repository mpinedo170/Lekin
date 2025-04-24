#pragma once

#include "AFLibTempTT.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgTT dialog
// A dialog box that supports tool tips and automatic resizes

namespace AFLibGui {
class AFLIB CDlgTT : public CTempTT<CDialog>
{
private:
    typedef CTempTT<CDialog> super;
    DEFINE_COPY_AND_ASSIGN(CDlgTT);

protected:
    // helper enum to init TResizeCoef
    enum TResizeStock
    {
        rzsNone,
        rzsMove,
        rzsPull
    };

    // auto-resizing coefficient set
    struct AFLIB TResize
    {
        double m_left;
        double m_top;
        double m_right;
        double m_bottom;

        TResize();
        TResize(TResizeStock rzsX, TResizeStock rzsY);
        TResize(double left, double top, double right, double bottom);

        // all coefficients are zero -- nothing to do
        bool IsNull() const;
    };

private:
    // auto-resizing control definition
    struct TPrivateResize
    {
        UINT m_idc;        // ctrl id
        TResize m_rz;      // how to move/resize
        bool m_bVisible;   // is this ctrl currently visible?
        CRect m_rectOrig;  // original rectangle of this control
    };

    CArray<TPrivateResize> m_arrResize;  // list of auto-resizing controls
    CSize m_szMin;                       // minimum size (initialized from current size)
    CSize* m_pSzSaved;                   // pointer to saved size from the lst time dlg was used

    // callback function for StartAutoResize()
    static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);

public:
    explicit CDlgTT();
    explicit CDlgTT(UINT nIDTemplate, CWnd* pWndParent = NULL);

protected:
    // resize the dialog to match its "contents" window
    void ResizeToContents(CWnd* pWndContents);

    // call this to start auto-resizing (second call fails)
    bool StartAutoResize(CSize* pSzSaved);

    // implement this to make auto-resizing work
    virtual TResize GetResizeMask(UINT idc);

    // if anything has to be resized non-automatically
    virtual void CustomResize(int cx, int cy);

    // create the icon for the dialog box
    //  (gets called on StartAutoResize(), but may be called manually)
    virtual void CreateDlgIcon();

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CDlgTT)
};
}  // namespace AFLibGui
