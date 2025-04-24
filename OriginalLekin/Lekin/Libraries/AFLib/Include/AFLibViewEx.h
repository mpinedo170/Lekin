#pragma once

#include "AFLib3State.h"
#include "AFLibDefine.h"

class CCriticalSection;

namespace AFLibThread {
class CThread;
}

/////////////////////////////////////////////////////////////////////////////
// TViewEx abstract structure
// A small print preview class.  Used by CDlgPrint to display page preview.
// To make it work, derive a class from both CView and TViewEx;
//   redefine ViewExUpdate and ViewExPrintPage.

namespace AFLibGui {
class TMargin;
class CDlgPrint;

class AFLIB TViewEx
{
private:
    DEFINE_COPY_AND_ASSIGN(TViewEx);

    // init structure for background updating
    struct TInitProcess
    {
        TViewEx* m_pViewEx;
        HDC m_hDC;
        const TMargin* m_pMargin;
        HANDLE m_hEventInit;
    };

    // synchronization element
    CCriticalSection* m_pSync;

    // worker thread for background updating
    AFLibThread::CThread* m_pThread;

    // # of pages returned by foreground updating
    int m_pageMax;

    // # of pages that have been updated in the background
    int m_pageReady;

    // status of updation: true if OK, false of invalid, undefined if still updating
    AFLib::T3State m_t3Status;

    // calls ViewExUpdate()
    static UINT ViewExUpdate2(LPVOID _pInit);

    // finish updation
    void FinalizeUpdate(bool bOk);

protected:
    bool m_bUseBgUpdate;  // use background update

    bool m_bViewExUsePreviewDC;  // Use standard MFC CPreviewDC?  Default - yes.
                                 // WHY?  A simple DC does not properly translate all DC functions
                                 //    from output DC to attribute DC
                                 // WHY NOT?  CPreviewDC is slower;
                                 //    CPreviewDC is defined in AfxPriv.h, so its interface may change later.

    // it is advisable to use this object in derived classes
    CCriticalSection* ViewExGetSyncObject() const
    {
        return m_pSync;
    }

    // update max page
    void ViewExSetPageMax(int pageMax);

    // update number of ready pages
    void ViewExSetPageReady(int pageReady);
    void ViewExAddPageReady(int addPageReady);

    // virtual part of ViewExUpdate (foreground)
    virtual AFLib::T3State ViewExUpdateFg(CDC* pDC, const TMargin* pMargin);

    // virtual part of ViewExUpdate (background, if m_bUseBgUpdate is set)
    virtual bool ViewExUpdateBg(CDC* pDC, const TMargin* pMargin);

public:
    TViewEx();
    virtual ~TViewEx();

    // max number of pages
    static const int pageMaxHigh = 0x7FFF;

    // returns CView derived from this TViewEx
    CView* ViewExGetView();

    // updates internal view data so that the drawing goes faster afterwards
    void ViewExUpdate(CDC* pDC, const TMargin* pMargin);

    // abort the updation
    void ViewExAbortUpdate();

    // wait till the updation finishes
    void ViewExWaitUpdate();

    // background updation finished?
    bool ViewExHasUpdateFinished() const;

    // updation ended in success?
    bool ViewExHasUpdateSucceeded() const;

    // updation ended in success?
    bool ViewExHasUpdateFailed() const;

    // check if the page is ready
    bool ViewExIsPageReady(int page) const;

    // get max number of pages
    int ViewExGetPageMax() const;

    //*** rare overridables ***

    // draw a cross, if data is not valid
    virtual void ViewExDrawCross(CDC* pDC, const CRect& rect) const;

    // draw a question, if page was not computed yet
    virtual void ViewExDrawQuestion(CDC* pDC, const CRect& rect) const;

    // creates a DC for page preview (result depends on m_bViewExUsePreviewDC)
    virtual CDC* ViewExCreateDC(
        HDC hDCScreen, HDC hDCPrinter, const CRect& rectScreen, const CSize& szPrinter) const;

    // destroys page preview DC
    virtual void ViewExDeleteDC(CDC* pDC) const;

    // call this from CView::OnPreparePrinting: initializes pInfo, calls DoPreparePrinting
    virtual bool ViewExPreparePrinting(CPrintInfo* pInfo, TMargin* pMargin, CDlgPrint* pDlg = NULL);

    // call this from CView::OnPrint
    virtual void ViewExOnPrint(CDC* pDC, CPrintInfo* pInfo);

    //*** frequent overridables ***

    // draws a page
    virtual void ViewExPrintPage(CDC* pDC, int page);

    // Hide, disable, or enable "Selection" button
    enum TSelButtonState
    {
        sbsHide,
        sbsDisable,
        sbsEnable
    };
    virtual TSelButtonState ViewExSelectionState();
};
}  // namespace AFLibGui
