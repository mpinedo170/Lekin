#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWndAFSplitter window
// Splitter window in CViewSplitted

namespace AFLibGui {
class CViewSplitted;

class AFLIB CWndAFSplitter : public CWnd
{
private:
    typedef CWnd super;

    HCURSOR m_hCursor;  // "size right-left" cursor handle
    bool m_bDrag;       // dragging now?
    int m_xStart;       // the x-offset of the mouse pointer from the left border of the strip when dragging
                        // started
    CRect m_rectSaved;  // saved position of the strip

    // leave dragging mode
    void StopDragging();

    // parent CViewSplitted window
    CViewSplitted* GetParent();

public:
    CWndAFSplitter();

    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CWndAFSplitter)
};
}  // namespace AFLibGui
