#pragma once

namespace AFLibGui {
class TDragField;
}

/////////////////////////////////////////////////////////////////////////////
// CWndDragHelper window
// A message handler for the drag field.
// TDragField is not a window and can't handle messages.

namespace AFLibPrivate {
class CWndDragHelper : public CWnd
{
private:
    typedef CWnd super;

    AFLibGui::TDragField* m_pDf;  // host drag field

public:
    CWndDragHelper(AFLibGui::TDragField* pDf, int ms);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibPrivate
