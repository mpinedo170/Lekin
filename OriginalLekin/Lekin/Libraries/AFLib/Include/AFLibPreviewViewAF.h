#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CPreviewViewAF class
// Print Preview view for CViewTempEx.
// Supports background view rendering.

namespace AFLibGui {
class TViewEx;

class AFLIB CPreviewViewAF : public CPreviewView
{
private:
    typedef CPreviewView super;
    DEFINE_COPY_AND_ASSIGN(CPreviewViewAF);

protected:
    // current page has already been rendered
    bool m_bPageValid;

    // ViewEx of this view
    TViewEx* GetViewEx();

    // check if currently displayed pages are valid; update m_bPageValid
    void UpdatePageValid();

public:
    CPreviewViewAF();

    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
    virtual void OnDraw(CDC* pDC);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CPreviewViewAF)
};
}  // namespace AFLibGui
