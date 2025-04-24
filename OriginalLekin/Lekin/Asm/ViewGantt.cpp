#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewGantt.h"

#include "DlgJob.h"
#include "DocTmpSmart.h"
#include "FrmSmart.h"
#include "MarginGantt.h"

#include "Draws.h"
#include "Job.h"
#include "Schedule.h"
#include "Sequence.h"
#include "Workcenter.h"
#include "Workspace.h"

namespace {
const CString keyGrid = _T("Grid");
}

CViewGantt::TDDGantt::TDDGantt(bool bPrinting) : TDDSmart(bPrinting)
{
    m_scaleX = m_scaleY = 1;
    m_yBlk = m_ySep = m_yMax = m_pagesY = 1;
    m_xFrom = m_xTo = 0;
    m_iMark = m_iWave = m_iHeight = m_iBegin = m_iEnd;
}

int CViewGantt::TDDGantt::GetX(double x)
{
    return int((x - m_xFrom) * m_scaleX);
}

double CViewGantt::TDDGantt::GetAntiX(int x)
{
    return x / m_scaleX;
}

void CViewGantt::TDDGantt::InitDrawData(CDC* pDC, int maxX)
{
    CFont font;
    GetWaveFont(pDC, font);
    CSize szFont = GetFontSize(pDC, &font, IntToStr(maxX));

    double wave = GetAntiX(szFont.cx * 3 / 2);
    m_iHeight = szFont.cy;

    m_iMark = m_iWave = 1;
    for (int i = 0; m_iWave < wave; ++i)
    {
        m_iMark = m_iWave;
        m_iWave *= 2;
        if (i % 3 == 1)
        {
            m_iWave += m_iWave / 4;
            m_iMark /= 2;
        }
    }
}

void CViewGantt::TDDGantt::GetWaveFont(CDC* pDC, CFont& font)
{
    theApp.CreateAppFont(font, pDC, 1);
}

void CViewGantt::TDDGantt::GetBeginEnd(CDC* pDC, int maxX)
{
    int left, right;

    if (m_bPrinting)
    {
        left = m_xFrom;
        right = m_xTo;
    }
    else
    {
        CRect rectClip;
        pDC->GetClipBox(&rectClip);
        left = int(GetAntiX(rectClip.left)) - m_iWave;
        right = int(GetAntiX(rectClip.right)) + m_iWave + m_iMark;
    }

    m_iBegin = left / m_iMark * m_iMark;
    if (!m_bPrinting && m_iBegin <= 0) m_iBegin = m_iMark;
    m_iEnd = min(right, maxX);
}

bool CViewGantt::TDDGantt::FillPageArray(double scaleY, int height, int pagesY, bool bNoSplit)
{
    int page = 0;
    int y = 0;

    m_scaleY = scaleY;
    m_yBlk = int(m_scaleY * 2.8);
    m_ySep = int(m_scaleY * 1.2);

    m_mapPos.RemoveAll();
    if (height < m_yBlk + 2 * m_ySep) return false;

    EnumerateWkc y += m_ySep;
    if (bNoSplit)
    {
        for (int i = 0;; ++i)
        {
            if (y + pWkc->GetMchCount() * (m_yBlk + m_ySep) <= height) break;
            if (i == 1) return false;
            ++page;
            y = m_ySep;
        }
    }

    for (int i = 0; i < pWkc->GetMchCount(); ++i)
    {
        TMachine* pMch = pWkc->m_arrMch[i];
        if (y + m_yBlk + m_ySep > height)
        {
            ++page;
            y = m_ySep;
        }

        if (page >= pagesY) return false;
        TPosition pos;
        pos.m_page = page;
        pos.m_yFrom = y;
        pos.m_yTo = y + m_yBlk;
        m_mapPos.SetAt(pMch, pos);
        y += m_yBlk + m_ySep;
    }
    EnumerateEnd

        m_yMax = y;
    m_pagesY = page + 1;
    return true;
}

bool CViewGantt::TDDGantt::FindScale(
    double minScaleY, double maxScaleY, int height, int pagesY, bool bNoSplit)
{
    if (FillPageArray(maxScaleY, height, pagesY, bNoSplit)) return true;
    if (!FillPageArray(minScaleY, height, pagesY, bNoSplit)) return false;
    bool bOK = true;

    while (maxScaleY - minScaleY > 1)
    {
        double scale = (maxScaleY + minScaleY) / 2;
        if (FillPageArray(scale, height, pagesY, bNoSplit))
            minScaleY = scale;
        else
        {
            bOK = false;
            maxScaleY = scale;
        }
    }

    if (!bOK) FillPageArray(minScaleY, height, pagesY, bNoSplit);
    return true;
}

void CViewGantt::TDDGantt::GetPosition(const TMachine* pMch, TPosition& pos)
{
    VERIFY(m_mapPos.Lookup(const_cast<TMachine*>(pMch), pos));
}

/////////////////////////////////////////////////////////////////////////////
// CViewGantt

IMPLEMENT_DYNCREATE(CViewGantt, CViewSmart)

CViewGantt::CViewGantt()
{
    m_dragOp = NULL;
    m_bDragging = false;
    m_cursor = NULL;
    InitDragging();
    m_grid = theApp.GetUserInt(secView, keyGrid, 0);
    m_pagesX = m_pagesY = 1;
    m_maxX = 1;
}

CDocTmpSmart* CViewGantt::GetFileTmp()
{
    return NULL;
}

BEGIN_MESSAGE_MAP(CViewGantt, super)
ON_COMMAND(ID_ZOOM_LONGER, OnZoomLonger)
ON_COMMAND(ID_ZOOM_SHORTER, OnZoomShorter)
ON_COMMAND(ID_ZOOM_WIDER, OnZoomWider)
ON_COMMAND(ID_ZOOM_NARROWER, OnZoomNarrower)
ON_UPDATE_COMMAND_UI(ID_ZOOM_LONGER, OnUpdateZoomLonger)
ON_UPDATE_COMMAND_UI(ID_ZOOM_SHORTER, OnUpdateZoomShorter)
ON_UPDATE_COMMAND_UI(ID_ZOOM_WIDER, OnUpdateZoomWider)
ON_UPDATE_COMMAND_UI(ID_ZOOM_NARROWER, OnUpdateZoomNarrower)
ON_COMMAND(ID_ZOOM_FIT, OnZoomFit)
ON_UPDATE_COMMAND_UI(ID_ZOOM_FIT, OnUpdateZoomFit)
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_TIMER()
ON_WM_KEYDOWN()
ON_WM_DESTROY()
ON_COMMAND_RANGE(ID_VIEW_GRID, ID_VIEW_STAT, OnGrid)
ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_GRID, ID_VIEW_STAT, OnUpdateGrid)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewGantt drawing

CViewSmart::TDDSmart* CViewGantt::CreateDD(bool bPrinting)
{
    return new TDDGantt(bPrinting);
}

CSize CViewGantt::OnScreenUpdate(CDC* pDC, TDDSmart* pDD)
{
    TDDGantt* pDD2 = (TDDGantt*)pDD;
    pDD2->FillPageArray(pDD2->m_scaleY, INT_MAX, 1, false);
    pDD2->InitDrawData(pDC, m_maxX);
    pDD2->m_xFrom = 0;
    pDD2->m_xTo = m_maxX;
    return CSize(pDD2->GetX(m_maxX), pDD2->m_yMax);
}

void CViewGantt::GetFont(CDC* pDC, CFont& font, bool bBold, TDDGantt* pDD)
{
    theApp.CreateAppFont(font, pDC, 1., bBold);
    int h1 = GetFontHeight(pDC, &font);
    double h2 = max(0.1, double(pDD->m_yBlk - 2) / h1);

    if (h2 < 1)
    {
        font.DeleteObject();
        theApp.CreateAppFont(font, pDC, h2, bBold);
    }
}

int CViewGantt::Visibility(const TWorkcenter* pWkc, CRect& rectClip, TDDGantt* pDD)
{
    int vis = Visibility(pWkc->m_arrMch[pWkc->GetMchCount() - 1], rectClip, pDD);
    if (vis < 0) return -1;

    if (pWkc->GetMchCount() > 1) vis = Visibility(pWkc->m_arrMch[0], rectClip, pDD);

    if (vis == 1) return 1;
    return 0;
}

int CViewGantt::Visibility(const TMachine* pMch, CRect& rectClip, TDDGantt* pDD)
{
    TPosition pos;
    pDD->GetPosition(pMch, pos);
    if (pDD->m_bPrinting)
    {
        if (pos.m_page < m_curPageY) return -1;
        if (pos.m_page > m_curPageY) return 1;
    }

    if (pos.m_yTo < rectClip.top) return -1;
    if (pos.m_yFrom > rectClip.bottom) return 1;
    return 0;
}

int CViewGantt::GetApproxY()
{
    return arrWorkcenter.GetSize() + GetTotalMchCount() * 4;
}

CRect CViewGantt::GetRect(const TOperation* pOp, TDDGantt* pDD)
{
    if (pOp->m_tm.m_pSeq == NULL) return CRect(-1, -1, -1, -1);

    TPosition pos;
    pDD->GetPosition(pOp->m_tm.m_pSeq->m_pMch, pos);
    if (pDD->m_bPrinting && pos.m_page != m_curPageY) return CRect(-1, -1, -1, -1);

    int x1 = pDD->GetX(pOp->GetStart());
    int x2 = max(x1 + 1, pDD->GetX(pOp->GetStop()));
    return CRect(x1, pos.m_yFrom, x2, pos.m_yTo);
}

void CViewGantt::DrawMark(CDC* pDC, int cy, int where, COLORREF color, TDDGantt* pDD)
{
    int d = cy / 3;
    CPen pen(PS_SOLID, d / 2, color);
    CPen* pPenOld = pDC->SelectObject(&pen);
    int x = pDD->GetX(where);
    pDC->MoveTo(x, cy - 3);
    pDC->LineTo(x - d / 2, cy - d);
    pDC->LineTo(x + d / 2, cy - d);
    pDC->LineTo(x, cy - 3);
    pDC->SelectObject(pPenOld);
}

void CViewGantt::DrawTimes(CDC* pDC, CSize sz, TDDGantt* pDD)
{
    CFont font;
    pDD->GetWaveFont(pDC, font);
    pDD->GetBeginEnd(pDC, m_maxX);

    pDC->SetTextAlign(TA_TOP | TA_CENTER);
    CPen pen;
    pen.CreateStockObject(BLACK_PEN);
    CFont* pFontOld = pDC->SelectObject(&font);
    CPen* pPenOld = pDC->SelectObject(&pen);

    if (pDD->m_bPrinting)
    {
        pDC->MoveTo(0, sz.cy - 1);
        pDC->LineTo(sz.cx, sz.cy - 1);
    }
    else
    {
        CRect rectClip;
        pDC->GetClipBox(&rectClip);
        DrawUnder(pDC, rectClip);
        if (theApp.m_pDlgJob != NULL)
        {
            DrawMark(pDC, sz.cy, theApp.m_pDlgJob->m_release, RGB(0, 192, 0), pDD);
            DrawMark(pDC, sz.cy, theApp.m_pDlgJob->m_stop, RGB(0, 0, 192), pDD);
            DrawMark(pDC, sz.cy, theApp.m_pDlgJob->m_due, RGB(192, 0, 0), pDD);
        }
    }

    for (int i = pDD->m_iBegin; i < pDD->m_iEnd; i += pDD->m_iMark)
    {
        int x = pDD->GetX(i);
        bool bDigit = i % pDD->m_iWave == 0;
        if (bDigit) pDC->TextOut(x, 2, IntToStr(i));

        // draw mark
        int dy = sz.cy - pDD->m_iHeight - 2;
        if (!bDigit) dy = dy / 2 + 1;
        pDC->MoveTo(x, sz.cy - 1);
        pDC->LineTo(x, sz.cy - dy);
    }
    pDC->SelectObject(pFontOld);
    pDC->SelectObject(pPenOld);
}

void CViewGantt::DrawMch(CDC* pDC, CSize sz, TDDGantt* pDD)
{
    CFont font;
    GetFont(pDC, font, true, pDD);
    CRect rectClip;
    pDC->GetClipBox(&rectClip);

    if (!pDD->m_bPrinting) DrawUnder(pDC, rectClip);

    pDC->SetTextAlign(TA_TOP | TA_LEFT);
    CFont* pFontOld = pDC->SelectObject(&font);
    CRect rect2(0, 0, sz.cx, 0);
    rect2.DeflateRect(rect2.Width() / 10, 0);

    EnumerateWkc int vis = Visibility(pWkc, rectClip, pDD);
    if (vis == -1) continue;
    if (vis == 1) break;

    for (int i = 0; i < pWkc->GetMchCount(); ++i)
    {
        TMachine* pMch = pWkc->m_arrMch[i];
        TPosition pos;
        pDD->GetPosition(pMch, pos);
        if (pDD->m_bPrinting && pos.m_page != m_curPageY) continue;

        rect2.top = pos.m_yFrom;
        rect2.bottom = pos.m_yTo;
        pWkc->DrawRect(pDC, rect2, pMch->m_id);
    }
    EnumerateEnd pDC->SelectObject(pFontOld);
}

CRect CViewGantt::GetViewRect(CDC* pDC, const CRect& rectFrame, TDDSmart* pDD)
{
    CFont font;
    theApp.CreateAppFont(font, pDC, 1., true);
    CFont* pFontOld = pDC->SelectObject(&font);

    int margin = pDC->GetTextExtent(_T("W")).cy;
    CRect rect(rectFrame);

    int w = 0;
    EnumerateWkc for (int i = 0; i < pWkc->GetMchCount(); ++i) w =
        max(int(pDC->GetTextExtent(pWkc->m_arrMch[i]->m_id).cx), w);
    EnumerateEnd rect.left += min(int(w * 1.5), int(rect.Width() * 0.4));

    rect.top += int(margin * 1.7);
    pDC->SelectObject(pFontOld);
    return rect;
}

void CViewGantt::DrawHeader(THeader header, CDC* pDC, CSize sz, TDDSmart* pDD)
{
    TDDGantt* pDD2 = (TDDGantt*)pDD;
    if (pSchActive == NULL) return;

    switch (header)
    {
        case Top:
            DrawTimes(pDC, sz, pDD2);
            return;
        case Left:
            DrawMch(pDC, sz, pDD2);
            return;
    }
}

void CViewGantt::DrawView(CDC* pDC, CSize sz, TDDSmart* pDD)
{
    TDDGantt* pDD2 = (TDDGantt*)pDD;
    if (!pSchActive) return;

    if (pDD2->m_bPrinting) SetClipRect(pDC, CRect(CPoint(0, 0), sz));

    CRect rectClip;
    pDC->GetClipBox(&rectClip);

    // draw grid
    if (m_grid & viGrid)
    {
        pDD2->GetBeginEnd(pDC, m_maxX);
        CPen pen(PS_DOT, 0, pDD->m_bPrinting ? colorBlack : colorDark());
        CPen* pPenOld = pDC->SelectObject(&pen);
        int y = pDD2->m_bPrinting && m_curPageY < m_pagesY - 1 ? sz.cy : pDD2->m_yMax;

        for (int i = max(pDD2->m_iBegin / pDD2->m_iWave, 1) * pDD2->m_iWave; i < pDD2->m_iEnd;
             i += pDD2->m_iWave)
        {
            int x = pDD2->GetX(i);
            pDC->MoveTo(x, 0);
            pDC->LineTo(x, y);
        }

        EnumerateWkc TPosition pos;
        pDD2->GetPosition(pWkc->m_arrMch[pWkc->GetMchCount() - 1], pos);
        if (pDD->m_bPrinting && pos.m_page != m_curPageY) continue;

        int y = pos.m_yTo + pDD2->m_ySep;
        pDC->MoveTo(0, y);
        pDC->LineTo(pDD2->GetX(m_maxX), y);
        EnumerateEnd

            pDC->SelectObject(pPenOld);
    }

    // draw operations & setups
    CFont font;
    GetFont(pDC, font, false, pDD2);
    CFont* pFontOld = pDC->SelectObject(&font);
    pDC->SetTextAlign(TA_TOP | TA_CENTER);
    bool bShowLate = (m_grid & viLate) && TJob::DuePresent();

    for (int i = 0; i < pSchActive->m_arrSequence.GetSize(); ++i)
    {
        TSequence* pSeq = pSchActive->m_arrSequence[i];
        int vis = Visibility(pSeq->m_pMch, rectClip, pDD2);
        if (vis == -1) continue;
        if (vis == 1) break;

        for (int j = 0; j < pSeq->m_arrOp.GetSize(); ++j)
        {
            TOperation* pOp = pSeq->m_arrOp[j];
            CRect rect = GetRect(pOp, pDD2);
            if (rect.right < rectClip.left) continue;
            int pre = pDD2->GetX(pOp->GetPreStart());
            if (pre > rectClip.right) break;

            bool bCross = pOp->m_tm.m_rel > 0;
            bool bLate = bShowLate && pOp->m_pJob->IsLate();
            TID style = *pOp->m_pJob;
            style.DrawRect(pDC, rect, m_grid & viJobs ? NULL : LPCTSTR(strEmpty), bCross, bLate);
            if (pOp->m_tm.m_setup == 0) continue;

            rect.right = rect.left;
            rect.left = pre;
            style.m_ty = tySpec;
            style.DrawRect(pDC, rect, m_grid & viStats ? pOp->GetStatString() : strEmpty, false, bLate);
        }
    }

    pDC->SelectObject(pFontOld);

    // draw selection
    if (!pDD->m_bPrinting && theApp.m_pDlgJob)
    {
        TOperation* pOp = theApp.m_pDlgJob->m_pOp;
        CRect rect = GetRect(pOp, pDD2);
        CPen pen(PS_SOLID, 2, colorBlack);
        CBrush brush;
        brush.CreateStockObject(NULL_BRUSH);
        rect.InflateRect(int(0.077 * rect.Width()), rect.Height() / 2);
        CPen* pPenOld = pDC->SelectObject(&pen);
        CBrush* pBrushOld = pDC->SelectObject(&brush);
        pDC->Ellipse(rect);
        pDC->SelectObject(pPenOld);
        pDC->SelectObject(pBrushOld);
    }

    // draw the time when machines are unavailable
    TID style(tySpec);
    EnumerateWkc int vis = Visibility(pWkc, rectClip, pDD2);
    if (vis == -1) continue;
    if (vis == 1) break;

    for (int i = 0; i < pWkc->GetMchCount(); ++i)
    {
        TMachine* pMch = pWkc->m_arrMch[i];
        int vis = Visibility(pMch, rectClip, pDD2);
        if (vis != 0) continue;

        TPosition pos;
        pDD2->GetPosition(pMch, pos);
        CRect mchRect(0, pos.m_yFrom, pDD2->GetX(pMch->m_release), pos.m_yTo);
        style.DrawRect(pDC, mchRect);
    }
    EnumerateEnd

        if (pDD->m_bPrinting)
    {
        CPen pen;
        pen.CreateStockObject(BLACK_PEN);
        CPen* pPenOld = pDC->SelectObject(&pen);
        pDC->MoveTo(0, 0);
        pDC->LineTo(0, sz.cy);
        pDC->SelectObject(pPenOld);
    }
    pDC->SelectClipRgn(NULL);
}

void CViewGantt::DrawMisc(CDC* pDC, TDDSmart* pDD)
{
    CPen pen;
    pen.CreateStockObject(BLACK_PEN);
    CPen* pPenOld = pDC->SelectObject(&pen);

    pDC->MoveTo(m_pWndParent->m_rectFrame.left, m_pWndParent->m_rectView.top);
    pDC->LineTo(m_pWndParent->m_rectView.left, m_pWndParent->m_rectView.top);
    pDC->LineTo(m_pWndParent->m_rectView.left, m_pWndParent->m_rectFrame.top);

    pDC->SelectObject(pPenOld);
}

int CViewGantt::OnPrintUpdate(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    TDDGantt* pDD2 = (TDDGantt*)pDD;
    m_perPageX = (m_maxX + m_pagesX - 1) / m_pagesX;

    int height = m_rectCenter.Height();
    double maxScaleY = min(GetFontHeight(pDC, m_pFontTitle) * 2, (m_pagesY * height) / GetApproxY());
    double minScaleY = 1;

    if (!pDD2->FindScale(minScaleY, maxScaleY, height, m_pagesY, m_bNoSplit)) return -1;
    m_pagesY = pDD2->m_pagesY;

    int maxScaleX = int(pDD2->m_scaleY * 2);
    pDD2->m_scaleX = double(m_rectCenter.Width()) / m_perPageX;
    if (pDD2->m_scaleX < 1) return -1;

    if (pDD2->m_scaleX > maxScaleX)
    {
        pDD2->m_scaleX = maxScaleX;
        m_perPageX = m_rectCenter.Width() / maxScaleX;
        m_pagesX = (m_maxX + m_perPageX - 1) / m_perPageX;
    }

    pDD2->InitDrawData(pDC, m_maxX);
    return m_pagesX * m_pagesY;
}

int CViewGantt::GetPrintYBottom(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    TMarginGantt* pMar2 = (TMarginGantt*)pMargin;
    m_pagesX = pMar2->m_nX;
    m_pagesY = pMar2->m_nY;
    m_bNoSplit = pMar2->m_bNoSplit;

    int wh = GetFontHeight(pDC, m_pFontTitle) / 2;
    double coef = double(m_rectPage.Width()) / m_rectPage.Height();
    if (coef > 1)
    {
        m_wSquare = wh;
        m_hSquare = int(wh / coef);
    }
    else
    {
        m_hSquare = wh;
        m_wSquare = int(wh * coef);
    }

    return m_pagesX * m_pagesY == 1 ? 0 : max(m_yTop, m_hSquare * m_pagesY);
}

void CViewGantt::PrintBottom(CDC* pDC, TDDSmart* pDD)
{
    TDDGantt* pDD2 = (TDDGantt*)pDD;
    m_curPageX = (m_page - 1) % m_pagesX;
    m_curPageY = (m_page - 1) / m_pagesX;

    pDD2->m_xFrom = m_curPageX * m_perPageX;
    pDD2->m_xTo = pDD2->m_xFrom + m_perPageX;
    if (m_pagesX * m_pagesY == 1) return;

    PrintPageNo(pDC);
    CPoint pt(m_rectPage.CenterPoint().x + pDC->GetTextExtent(_T("W")).cy * 5,
        m_rectPage.bottom + (m_yBottom - m_hSquare * m_pagesY) / 2);

    CPen pen;
    pen.CreateStockObject(BLACK_PEN);
    CPen* pPenOld = pDC->SelectObject(&pen);

    for (int i = 0; i <= m_pagesX; ++i)
    {
        pDC->MoveTo(pt.x + i * m_wSquare, pt.y);
        pDC->LineTo(pt.x + i * m_wSquare, pt.y + m_pagesY * m_hSquare);
    }

    for (int i = 0; i <= m_pagesY; ++i)
    {
        pDC->MoveTo(pt.x, pt.y + i * m_hSquare);
        pDC->LineTo(pt.x + m_pagesX * m_wSquare, pt.y + i * m_hSquare);
    }

    pDC->FillSolidRect(
        pt.x + m_curPageX * m_wSquare, pt.y + m_curPageY * m_hSquare, m_wSquare, m_hSquare, colorBlack);

    pDC->SelectObject(pPenOld);
}

void CViewGantt::AAA(bool bX, bool bUp)
{
    TDDGantt* pDD = (TDDGantt*)m_pDDPaint;
    double& x = bX ? pDD->m_scaleX : pDD->m_scaleY;

    double scale;
    for (scale = 1; scale < 64; scale *= 1.5)
        if (scale > x - 0.1) break;
    x = bUp ? scale * 1.5 : scale / 1.5;
}

/////////////////////////////////////////////////////////////////////////////
// CViewGantt message handlers

void CViewGantt::OnDestroy()
{
    super::OnDestroy();
    theApp.WriteUserInt(secView, keyGrid, m_grid);
}

bool CViewGantt::ZoomFit()
{
    TDDGantt* pDD = (TDDGantt*)m_pDDPaint;
    if (!pSchActive || !IsVisible()) return false;
    CRect rect;
    GetClientRect(rect);

    pDD->m_scaleX = max(0.1, double(rect.Width()) / m_maxX);
    pDD->m_scaleY = min(32., max(1., double(rect.Height()) / GetApproxY()));

    SetScrollPos(SB_HORZ, 0);
    SetScrollPos(SB_VERT, 0);
    ScreenUpdate();
    return true;
}

void CViewGantt::Update()
{
    if (pSchActive)
        m_maxX = pSchActive->m_arrObj[objCmax] + 10;  // makespan
    else
    {
        m_maxX = 1;
        TDDGantt* pDD = (TDDGantt*)m_pDDPaint;
        pDD->m_scaleX = 8;
        pDD->m_scaleY = 8;
    }

    super::Update();
}

void CViewGantt::OnZoomLonger()
{
    AAA(true, true);
    ScreenUpdate();
}

void CViewGantt::OnZoomShorter()
{
    AAA(true, false);
    ScreenUpdate();
}

void CViewGantt::OnZoomWider()
{
    AAA(false, true);
    ScreenUpdate();
}

void CViewGantt::OnZoomNarrower()
{
    AAA(false, false);
    ScreenUpdate();
}

void CViewGantt::OnZoomFit()
{
    ZoomFit();
}

void CViewGantt::OnGrid(UINT nID)
{
    m_grid ^= 1 << (nID - ID_VIEW_GRID);
    RedrawWindow();
}

void CViewGantt::OnUpdateZoomLonger(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(pSchActive && ((TDDGantt*)m_pDDPaint)->m_scaleX < 31.9);
}

void CViewGantt::OnUpdateZoomShorter(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(pSchActive && ((TDDGantt*)m_pDDPaint)->m_scaleX > 1.1);
}

void CViewGantt::OnUpdateZoomWider(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(pSchActive && ((TDDGantt*)m_pDDPaint)->m_scaleY < 31.9);
}

void CViewGantt::OnUpdateZoomNarrower(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(pSchActive && ((TDDGantt*)m_pDDPaint)->m_scaleY > 1.1);
}

void CViewGantt::OnUpdateZoomFit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(pSchActive != NULL);
}

void CViewGantt::OnUpdateGrid(CCmdUI* pCmdUI)
{
    int id = pCmdUI->m_nID - ID_VIEW_GRID;
    bool bYes = (m_grid & (1 << id)) != 0;
    bool bActive = pSchActive != NULL;

    switch (id)
    {
        case 2:
            bActive &= TJob::DuePresent();
            break;
        case 3:
            bActive &= TWorkcenter::SetupPresent();
            break;
    }

    pCmdUI->Enable(bActive);
    pCmdUI->SetCheck(bActive & bYes);
}

// drag-drop

void CViewGantt::OnLButtonDown(UINT nFlags, CPoint point)
{
    super::OnLButtonDown(nFlags, point);
    if (!pSchActive || m_bDragging) return;

    TDDGantt* pDD = (TDDGantt*)m_pDDPaint;
    InitDragging();
    point += GetScrollPosition();
    m_dragOp = NULL;

    EnumerateJob for (int i = 0; i < pJob->GetOpCount(); ++i)
    {
        TOperation* pOp = pJob->m_arrOp[i];
        CRect opRect = GetRect(pOp, pDD);
        if (opRect.PtInRect(point))
        {
            m_bDragging = false;
            m_dragOp = pOp;
            m_cursor = CreateCursor(pOp->m_pJob->m_color);
            SetTimer(timerUpdate, 100, NULL);
            SetCapture();
            theApp.UpdateFloaters(m_dragOp);
            return;
        }
    }
    EnumerateEnd
}

void CViewGantt::DblClick(UINT nFlags, CPoint point)
{
    if (m_dragOp == NULL) return;
    theApp.OpenDlgJob();
    theApp.UpdateFloaters(m_dragOp);
}

void CViewGantt::OnLButtonUp(UINT nFlags, CPoint point)
{
    super::OnLButtonUp(nFlags, point);
    if (!m_bDragging)
    {
        StopDragging();
        return;
    }

    TSequence* pSeq = m_dropSeq;
    TOperation* pOp = m_dragOp;
    int tag = m_dropTag;
    int time = m_dropTime;

    StopDragging();
    if (!pSchActive || !pSeq) return;

    TSequence* pSeq2 = pOp->m_tm.m_pSeq;
    int tag2 = pOp->m_tm.m_tag;
    if (pSeq2 == pSeq && tag > tag2) --tag;

    TSchedule* schBackup = new TSchedule(*pSchActive);
    pSeq2->m_arrOp.RemoveAt(tag2);
    pSeq->m_arrOp.InsertAt(tag, pOp);
    pSchActive->DelDelay(pOp);
    if (time != -1) pSchActive->AddDelay(pOp, time);

    if (!pSchActive->Recompute(true))
    {
        pSchActive->FormatError();
        *pSchActive = *schBackup;
        pSchActive->Recompute(true);
        theApp.m_pTmpSeq->Modify(false);
    }
    else
        theApp.m_pTmpSeq->Modify();

    delete schBackup;
}

void CViewGantt::OnMouseMove(UINT nFlags, CPoint point)
{
    int xCursor = 0;
    int yCursor = 0;
    int i = 0;
    TWorkcenter* pWkc = NULL;
    TMachine* pMch = NULL;

    super::OnMouseMove(nFlags, point);

    if (!m_bDragging) return;
    if (!pSchActive)
    {
        StopDragging();
        return;
    }

    TDDGantt* pDD = (TDDGantt*)m_pDDPaint;
    DrawCursor();
    InitDragging();

    CRect rect;
    GetClientRect(rect);
    if (!rect.PtInRect(point)) goto CURSOR;
    point += GetScrollPosition();

    pWkc = m_dragOp->m_pWkc;
    if (Visibility(pWkc, CRect(0, point.y, 0, point.y), pDD) != 0) goto CURSOR;

    for (int i = pWkc->GetMchCount(); --i >= 0;)
    {
        pMch = pWkc->m_arrMch[i];
        TPosition pos;
        pDD->GetPosition(pMch, pos);
        yCursor = pos.m_yFrom;
        if (yCursor < point.y) break;
    }

    m_dropSeq = pSchActive->FindMch(pMch);
    for (m_dropTag = 0; m_dropTag < m_dropSeq->GetOpCount(); ++m_dropTag)
    {
        TOperation* pOp = m_dropSeq->m_arrOp[m_dropTag];
        if (pDD->GetX(pOp->GetStop()) >= point.x) break;
    }

    xCursor = pDD->GetX(
        m_dropTag > 0 ? m_dropSeq->m_arrOp[m_dropTag - 1]->GetStop() : m_dropSeq->m_pMch->m_release);

    if (::GetKeyState(VK_SHIFT) < 0)
    {
        m_dropTime = int(pDD->GetAntiX(point.x));
        xCursor = pDD->GetX(m_dropTime);
    }

    m_lastPt = CPoint(xCursor, yCursor) - GetScrollPosition();
    DrawCursor();

CURSOR:
    ::SetCursor(m_lastPt.x == -1 ? theApp.LoadCursor(IDU_NODROP) : m_cursor);
}

void CViewGantt::OnTimer(UINT nIDEvent)
{
    if (!pSchActive)
    {
        StopDragging();
        return;
    }

    if (nIDEvent == timerUpdate)
    {
        ASSERT(m_dragOp);
        m_bDragging = true;

        CRect rect;
        GetClientRect(rect);
        CPoint pt;
        ::GetCursorPos(&pt);
        ScreenToClient(&pt);

        if (pt.x < 0)
            PostMessage(WM_HSCROLL, SB_LINELEFT, 0);
        else if (pt.x > rect.right)
            PostMessage(WM_HSCROLL, SB_LINERIGHT, 0);

        if (pt.y < 0)
            PostMessage(WM_VSCROLL, SB_LINEUP, 0);
        else if (pt.y > rect.bottom)
            PostMessage(WM_VSCROLL, SB_LINEDOWN, 0);
    }
}

void CViewGantt::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_bDragging && nChar == VK_ESCAPE)
    {
        StopDragging();
        return;
    }

    super::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CViewGantt::InitDragging()
{
    m_lastPt = CPoint(-1, -1);
    m_dropSeq = NULL;
    m_dropTag = m_dropTime = -1;
}

void CViewGantt::StopDragging()
{
    DrawCursor();
    KillTimer(timerUpdate);
    InitDragging();
    m_bDragging = false;
    if (m_cursor)
    {
        ::SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
        ::DestroyCursor(m_cursor);
        m_cursor = NULL;
    }
    ReleaseCapture();
}

void CViewGantt::DrawCursor()
{
    if (m_lastPt.x == -1) return;
    TDDGantt* pDD = (TDDGantt*)m_pDDPaint;

    CDC* pDC = GetDC();
    int dy = pDD->m_yBlk;
    pDC->SetROP2(R2_XORPEN);
    CPen pen(PS_SOLID, 3, colorWhite);
    CPen* pPenOld = pDC->SelectObject(&pen);

    pDC->MoveTo(m_lastPt);
    pDC->LineTo(m_lastPt.x, m_lastPt.y + dy);

    pDC->MoveTo(m_lastPt);
    pDC->LineTo(m_lastPt.x - 3, m_lastPt.y - 3);
    pDC->MoveTo(m_lastPt);
    pDC->LineTo(m_lastPt.x + 3, m_lastPt.y - 3);

    pDC->MoveTo(m_lastPt.x, m_lastPt.y + dy);
    pDC->LineTo(m_lastPt.x - 3, m_lastPt.y + dy + 3);
    pDC->MoveTo(m_lastPt.x, m_lastPt.y + dy);
    pDC->LineTo(m_lastPt.x + 3, m_lastPt.y + dy + 3);

    pDC->SelectObject(pPenOld);
    ReleaseDC(pDC);
}

HCURSOR CViewGantt::CreateCursor(COLORREF color)
{
    CSize szCursor(32, 32);
    CSize szOffset(4, 10);

    CPoint ptOrigin(0, 0);
    CRect rectLarge(ptOrigin, szCursor);
    CRect rectSmall(ptOrigin + szOffset, szCursor - szOffset - szOffset);

    CImageAF imgMask;
    imgMask.CreateMonochrome(szCursor);
    CImageAF imgColor;
    imgColor.CreateScreen(szCursor);

    imgMask.Fill(colorWhite);
    imgColor.Fill(colorBlack);

    CDC* pDC = imgMask.GetCDC();
    pDC->FillSolidRect(rectSmall, colorBlack);
    imgMask.ReleaseCDC(pDC);

    CBrush brush(color);
    rectSmall.DeflateRect(1, 1);
    pDC = imgColor.GetCDC();
    pDC->FillRect(rectSmall, &brush);
    imgColor.ReleaseCDC(pDC);

    ICONINFO ico = {false, 15, 15, imgMask, imgColor};
    return HCURSOR(CreateIconIndirect(&ico));
}
