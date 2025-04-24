#include "StdAfx.h"

#include "AFLibViewEx.h"

#include "AFLibDCStateSaver.h"
#include "AFLibDlgPrint.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibMargin.h"
#include "AFLibMathGlobal.h"
#include "AFLibThread.h"

using namespace AFLibGui;
using namespace AFLibThread;
using namespace AFLib;
using AFLibMath::Round;

TViewEx::TViewEx()
{
    m_bUseBgUpdate = m_bViewExUsePreviewDC = true;
    m_pageMax = m_pageReady = 0;
    m_pSync = new CCriticalSection;
    m_pThread = new CThread;
    m_t3Status = false;
}

TViewEx::~TViewEx()
{
    delete m_pThread;
    delete m_pSync;
}

void TViewEx::ViewExSetPageMax(int pageMax)
{
    CSingleLock lock(m_pSync, true);
    m_pageMax = pageMax;
}

void TViewEx::ViewExSetPageReady(int pageReady)
{
    CSingleLock lock(m_pSync, true);
    m_pageReady = pageReady;
}

void TViewEx::ViewExAddPageReady(int addPageReady)
{
    CSingleLock lock(m_pSync, true);
    m_pageReady += addPageReady;
}

void TViewEx::FinalizeUpdate(bool bOk)
{
    CSingleLock lock(m_pSync, true);
    if (!bOk) m_pageReady = 0;
    m_pageMax = m_pageReady;
    m_t3Status = bOk && m_pageMax > 0;
}

CView* TViewEx::ViewExGetView()
{
    return dynamic_cast<CView*>(this);
}

TViewEx::TSelButtonState TViewEx::ViewExSelectionState()
{
    return sbsHide;
}

T3State TViewEx::ViewExUpdateFg(CDC* pDC, const TMargin* pMargin)
{
    return false;
}

bool TViewEx::ViewExUpdateBg(CDC* pDC, const TMargin* pMargin)
{
    return false;
}

void TViewEx::ViewExPrintPage(CDC* pDC, int page)
{}

UINT TViewEx::ViewExUpdate2(LPVOID _pInit)
{
    TInitProcess* pInit = reinterpret_cast<TInitProcess*>(_pInit);
    TViewEx* pViewEx = pInit->m_pViewEx;

    CDC dc;
    dc.Attach(::CreateCompatibleDC(pInit->m_hDC));
    dc.m_bPrinting = true;

    CBitmap bmp;
    bmp.Attach(::CreateCompatibleBitmap(pInit->m_hDC, 10, 10));
    CBitmap* pBmpOld = dc.SelectObject(&bmp);

    CAutoPtr<TMargin> pMargin(pInit->m_pMargin->Clone());
    HANDLE hEventInit = pInit->m_hEventInit;
    delete pInit;
    pInit = NULL;
    SetEvent(hEventInit);

    try
    {
        bool bOk = pViewEx->ViewExUpdateBg(&dc, pMargin);
        pViewEx->FinalizeUpdate(bOk);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        pViewEx->FinalizeUpdate(false);
    }

    dc.SelectObject(pBmpOld);
    return 0;
}

void TViewEx::ViewExUpdate(CDC* pDC, const TMargin* pMargin)
{
    ViewExAbortUpdate();

    m_t3Status = t3Undef;
    m_pageMax = m_pageReady = 0;

    m_t3Status = ViewExUpdateFg(pDC, pMargin);

    if (m_t3Status != t3Undef)
    {
        m_pageReady = m_pageMax;
        if (m_pageMax <= 0) m_t3Status = false;
        return;
    }

    if (m_pageMax <= 0)
    {
        FinalizeUpdate(false);
        return;
    }

    if (!m_bUseBgUpdate)
    {
        bool bOk = ViewExUpdateBg(pDC, pMargin);
        FinalizeUpdate(bOk);
        return;
    }

    HANDLE hEventInit = CreateEvent(NULL, true, false, NULL);
    TInitProcess* pInit = new TInitProcess;
    pInit->m_pViewEx = this;
    pInit->m_hDC = pDC->m_hAttribDC;
    pInit->m_pMargin = pMargin;
    pInit->m_hEventInit = hEventInit;

    *m_pThread = CThread::Start(ViewExUpdate2, pInit, THREAD_PRIORITY_BELOW_NORMAL, 1 << 18);
    WaitForSingleObject(hEventInit, INFINITE);
    CloseHandle(hEventInit);
    return;
}

void TViewEx::ViewExAbortUpdate()
{
    m_pThread->Stop();
    m_pThread->Clear();
}

void TViewEx::ViewExWaitUpdate()
{
    m_pThread->WaitFor();
    m_pThread->Clear();
}

bool TViewEx::ViewExHasUpdateFinished() const
{
    CSingleLock lock(m_pSync, true);
    return m_t3Status != t3Undef;
}

bool TViewEx::ViewExHasUpdateSucceeded() const
{
    CSingleLock lock(m_pSync, true);
    return m_t3Status == true;
}

bool TViewEx::ViewExHasUpdateFailed() const
{
    CSingleLock lock(m_pSync, true);
    return m_t3Status == false;
}

bool TViewEx::ViewExIsPageReady(int page) const
{
    CSingleLock lock(m_pSync, true);
    return m_t3Status != t3Undef || page < m_pageReady;
}

int TViewEx::ViewExGetPageMax() const
{
    CSingleLock lock(m_pSync, true);
    return m_pageMax;
}

void TViewEx::ViewExDrawCross(CDC* pDC, const CRect& rect) const
{
    CPen pen;
    TDCStateSaver dcss(pDC);
    pen.CreatePen(PS_SOLID, min(rect.Width(), rect.Height()) / 20, colorBlack);
    pDC->SelectObject(&pen);

    pDC->MoveTo(rect.TopLeft());
    pDC->LineTo(rect.BottomRight());
    pDC->MoveTo(rect.right, rect.top);
    pDC->LineTo(rect.left, rect.bottom);
}

void TViewEx::ViewExDrawQuestion(CDC* pDC, const CRect& rect) const
{
    CFont font;
    TDCStateSaver dcss(pDC);

    CreateAbsoluteFont(font, strRoman, rect.Height() * 3 / 4, false, false);
    pDC->SelectObject(&font);
    pDC->SetTextAlign(TA_TOP | TA_CENTER);
    pDC->TextOut(rect.left + rect.Width() / 2, rect.top + rect.Height() / 8, strQuestion);
}

CDC* TViewEx::ViewExCreateDC(
    HDC hDCScreen, HDC hDCPrinter, const CRect& rectScreen, const CSize& szPrinter) const
{
    int xP = GetDeviceCaps(hDCPrinter, HORZRES);
    int yP = GetDeviceCaps(hDCPrinter, VERTRES);

    if (m_bViewExUsePreviewDC)
    {
        CPreviewDC* pDC = new CPreviewDC;
        pDC->SetAttribDC(hDCPrinter);

        double scale1 = (double(rectScreen.Width()) / GetDeviceCaps(hDCScreen, LOGPIXELSX)) /
                        (double(szPrinter.cx) / GetDeviceCaps(hDCPrinter, LOGPIXELSX));
        double scale2 = (double(rectScreen.Height()) / GetDeviceCaps(hDCScreen, LOGPIXELSY)) /
                        (double(szPrinter.cy) / GetDeviceCaps(hDCPrinter, LOGPIXELSY));
        double scale = min(scale1, scale2);

        pDC->SetTopLeftOffset(rectScreen.TopLeft());
        pDC->SetScaleRatio(int(Round(scale * 0x4000)), 0x4000);

        pDC->SetOutputDC(hDCScreen);
        pDC->m_bPrinting = true;
        return pDC;
    }
    else
    {
        CDC* pDC = new CDC;
        pDC->SetOutputDC(hDCScreen);
        pDC->SetMapMode(MM_ANISOTROPIC);
        pDC->SetWindowExt(szPrinter);
        pDC->SetViewportExt(rectScreen.Size());
        pDC->SetViewportOrg(rectScreen.TopLeft());
        pDC->SetAttribDC(hDCPrinter);
        pDC->m_bPrinting = true;
        return pDC;
    }

    return NULL;
}

void TViewEx::ViewExDeleteDC(CDC* pDC) const
{
    pDC->ReleaseAttribDC();
    pDC->ReleaseOutputDC();
    delete pDC;
}

bool TViewEx::ViewExPreparePrinting(CPrintInfo* pInfo, TMargin* pMargin, CDlgPrint* pDlg)
{
    if (pDlg == NULL) pDlg = new CDlgPrint(false);

    pDlg->m_pViewEx = this;
    pDlg->m_pMargin = pMargin;
    delete pInfo->m_pPD;
    pInfo->m_pPD = pDlg;

    if (!ViewExGetView()->DoPreparePrinting(pInfo)) return false;

    pDlg->FinalizeDC();
    return true;
}

void TViewEx::ViewExOnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    int page = pInfo->m_nCurPage;
    bool bFinished = ViewExHasUpdateFinished();

    if (!bFinished && !ViewExIsPageReady(page))
    {
        if (pInfo->m_bPreview)
        {
            ViewExDrawQuestion(pDC, pInfo->m_rectDraw);
            return;
        }

        while (!ViewExIsPageReady(page)) Sleep(100);
    }

    if (bFinished) pInfo->SetMaxPage(max(int(pInfo->GetMinPage()), ViewExGetPageMax()));

    ViewExPrintPage(pDC, page);
    if (page >= int(pInfo->GetMaxPage())) pInfo->m_bContinuePrinting = false;
}
