#pragma once

#include "AFLibViewAF.h"
#include "AFLibWndAFSplitter.h"

/////////////////////////////////////////////////////////////////////////////
// CViewSplitted form view
// A view with a splitter in the middle

namespace AFLibGui {
class AFLIB CViewSplitted : public CViewAF
{
private:
    typedef CViewAF super;
    friend CWndAFSplitter;
    DEFINE_COPY_AND_ASSIGN(CViewSplitted);

    CWndAFSplitter m_wndSplitter;  // Splitter stripe
    CWnd m_wndTopBorder;           // if there is a header, we need a small border for the splitter

protected:
    explicit CViewSplitted(UINT nIDTemplate = IDD_EMPTY);

    bool m_bSplitterOn;  // splitting functionality is on
    int m_xSplit;        // x-coordinate of the splitter
    int m_ySplit;        // y-coordinate of the splitter -- in case the window has a header
    int m_wSplit;        // width of the splitter
    int m_xLeftInset;    // left margin for the splitter
    int m_xRightInset;   // right margin for the splitter

    // user moved the splitter
    void Split();

    // user double-clicked the splitter
    void Fit();

    virtual void OnInit();
    virtual void OnResize();

    // computes the right position of the splitter
    virtual int GetFitWidth();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CViewSplitted)
};
}  // namespace AFLibGui
