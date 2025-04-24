#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewObj.h"

#include "DlgStyle.h"
#include "DocTmpSmart.h"

#include "Draws.h"
#include "Misc.h"
#include "Schedule.h"

int CViewObj::TDDObj::FindSch(const TSchedule* pSch)
{
    for (int i = 0; i < m_arrRD.GetSize(); ++i)
        if (m_arrRD[i].m_pSch == pSch) return i;
    return -1;
}

bool CViewObj::TRectData::IsShown()
{
    return m_pSch->m_bShown;
}

/////////////////////////////////////////////////////////////////////////////
// CViewObj

IMPLEMENT_DYNCREATE(CViewObj, CViewSmart)

CViewObj::CViewObj() :
    m_penGray(PS_SOLID, 5, colorGrayM),
    m_penDkGray(PS_SOLID, 5, colorGrayD),
    m_penBlack(PS_SOLID, 5, colorBlack),
    m_axisMask(0)
{}

CDocTmpSmart* CViewObj::GetFileTmp()
{
    return NULL;
}

BEGIN_MESSAGE_MAP(CViewObj, super)
END_MESSAGE_MAP()

void CViewObj::SetMask(int axisMask)
{
    m_axisMask = axisMask;
    RedrawWindow();
}

int CViewObj::GetAxisCount()
{
    int s = 0;
    for (int i = 0; i < 32; ++i) s += (m_axisMask & (1 << i)) != 0;
    return s;
}

CPoint CViewObj::AxisToXY(int axis, double val)
{
    double degree = 2 * pi / GetAxisCount() * axis;
    double fR = val / m_arrMax[axis] + 0.05;
    return CPoint(int(-fR * sin(degree) * 450.), int(fR * cos(degree) * 450.));
}

CPoint CViewObj::ObjToXY(int axis, const TSchedule* pSch)
{
    return AxisToXY(axis, GetObj(axis, pSch));
}

TObjective CViewObj::GetRealIndex(int axis)
{
    for (int i = 0;; ++i)
    {
        if (m_axisMask & (1 << i) && --axis < 0) return TObjective(i);
        ASSERT(i < objN);
    }
}

int CViewObj::GetObj(int axis, const TSchedule* pSch)
{
    return pSch->m_arrObj[GetRealIndex(axis)];
}

CSize CViewObj::CalcLegendSize(CDC* pDC,
    const CSize& szTotal,
    const CSize& szRect,
    const CSize& szMargin,
    const CSize& szSpace,
    bool bVertical,
    TDDObj* pDD)
{
    LONG(CSize::*pTotal) = bVertical ? &CSize::cy : &CSize::cx;
    LONG(CSize::*pMinus) = bVertical ? &CSize::cx : &CSize::cy;

    int count = max(
        int((szTotal.*pTotal - 2 * szMargin.*pTotal + szSpace.*pTotal) / (szRect.*pTotal + szSpace.*pTotal)),
        1);
    int lines = (arrSchedule.GetSize() - 1) / count + 1;

    CSize szMinus(0, 0);
    szMinus.*pMinus = szRect.*pMinus * lines + szSpace.*pMinus * (lines - 1) + szMargin.*pMinus * 2;
    return szMinus;
}

CSize CViewObj::CalcLegend(CDC* pDC, const CSize& szTotal, TDDObj* pDD)
{
    static const int minSize = 150;
    CSize szRect, szMargin, szSpace;
    CSize szMinus[2];
    int mSize[2];
    int vertical;

    CFont font;
    theApp.CreateAppFont(font, pDC, 1);
    CFont* pFontOld = pDC->SelectObject(&font);

    int h = pDC->GetTextExtent(_T("W")).cy + 8;
    int w = 0;

    EnumerateSch w = max(w, int(pDC->GetTextExtent(pSch->m_id).cx));
    EnumerateEnd pDC->SelectObject(pFontOld);

    szRect.cx = w + h * 3;
    szRect.cy = szMargin.cy = szMargin.cx = h;
    szSpace.cx = h;
    szSpace.cy = h / 2;

    for (vertical = 0; vertical <= 1; ++vertical)
    {
        szMinus[vertical] = CalcLegendSize(pDC, szTotal, szRect, szMargin, szSpace, vertical != 0, pDD);
        CSize sz = szTotal - szMinus[vertical];
        mSize[vertical] = min(sz.cx, sz.cy);
    }

    vertical = mSize[1] > mSize[0];
    CSize szNew(szTotal);

    if (mSize[vertical] < minSize)
    {
        vertical = szTotal.cx > szTotal.cy;
        CSize sz(minSize, minSize);
        sz += szMinus[vertical];
        sz.cx = max(sz.cx, szMinus[!vertical].cx);
        sz.cy = max(sz.cy, szMinus[!vertical].cy);

        szNew.cx = max(szNew.cx, sz.cx);
        szNew.cy = max(szNew.cy, sz.cy);
        szMinus[vertical] = CalcLegendSize(pDC, szNew, szRect, szMargin, szSpace, vertical != 0, pDD);
    }

    CSize& szMain = pDD->m_szMain;

    szMain = szNew;
    szMain -= szMinus[vertical];
    szMain.cx = szMain.cy = min(szMain.cx, szMain.cy);

    CRect legendRect(0, 0, szNew.cx, szNew.cy);
    if (vertical)
        legendRect.left = szMain.cx;
    else
        legendRect.top = szMain.cy;

    legendRect.DeflateRect(szMargin);
    int count = max(int((legendRect.Height() + szSpace.cy) / (szRect.cy + szSpace.cy)), 1);

    TScheduleArray& arrSch = theApp.GetSchList();
    pDD->m_arrRD.RemoveAll();

    for (int i = 0; i < arrSch.GetSize(); ++i)
    {
        TSchedule* pSch = arrSch[i];
        CPoint TopLeft((i / count) * (szRect.cx + szSpace.cx), (i % count) * (szRect.cy + szSpace.cy));

        CRect rect(TopLeft, szRect);
        rect.OffsetRect(legendRect.TopLeft());

        CRect rectCheck(rect);
        CRect rectMain(rect);
        rectMain.left = rectCheck.right = rect.left + h;
        rectCheck.DeflateRect(h / 6, h / 6);

        TRectData data;
        data.m_pSch = pSch;
        data.m_rectCheck = rectCheck;
        data.m_rectMain = rectMain;
        pDD->m_arrRD.Add(data);
    }

    return szNew;
}

/////////////////////////////////////////////////////////////////////////////
// CViewObj drawing

static CSize titleSize(120, 70);

void CViewObj::DrawAxisRect(CDC* pDC, TDDObj* pDD)
{
    CPen* pPenOld = pDC->SelectObject(&m_penBlack);
    pDC->MoveTo(-350, -450);
    pDC->LineTo(-350, +350);
    pDC->LineTo(+450, +350);

    pDC->SelectObject(&m_penDkGray);
    AndrewLineTo(pDC, +450, -450);
    AndrewLineTo(pDC, -350, -450);

    for (int i = 1; i < 10; ++i)
    {
        int x = -350 + i * 80;
        int y = 350 - i * 80;
        pDC->SelectObject(&m_penGray);
        pDC->MoveTo(x, -450);
        AndrewLineTo(pDC, x, +350);
        pDC->MoveTo(-350, y);
        AndrewLineTo(pDC, +450, y);
        pDC->SelectObject(&m_penBlack);
        pDC->MoveTo(x, +340);
        AndrewLineTo(pDC, x, +360);
        pDC->MoveTo(-340, y);
        AndrewLineTo(pDC, -360, y);
    }
    pDC->SelectObject(pPenOld);

    CRect rect(CPoint(-490, -450), titleSize);
    pDC->SetTextAlign(TA_TOP | TA_RIGHT);
    DrawFancy(pDC, rect, GetRealIndex(GetAxisCount() - 1), false);
}

void CViewObj::DrawAxisStar(CDC* pDC, TDDObj* pDD)
{
    CPen* pPenOld = pDC->SelectObject(&m_penBlack);
    for (int i = 0; i < GetAxisCount(); ++i)
    {
        pDC->MoveTo(0, 0);
        CPoint pt = AxisToXY(i, m_arrMax[i]);
        pDC->LineTo(pt);
        CSize dpt = AndrewGetOffset(pt, CPoint(0, 0), 10);

        int sgnX = sgn(pt.x);
        int sgnY = sgn(pt.y);

        if (sgnX == 1)
            pt.x -= titleSize.cx;
        else
            pt.x += titleSize.cx / 8;

        if ((sgnX && sgnY == -1) || (sgnX == 0 && sgnY == 1))
            pt.y -= titleSize.cy;
        else
            pt.y += titleSize.cy / 8;

        CRect rect(pt, titleSize);
        pDC->SetTextAlign(TA_TOP | TA_LEFT);
        DrawFancy(pDC, rect, GetRealIndex(i), false);

        for (int j = 0; j <= 10; ++j)
        {
            pt = AxisToXY(i, m_arrMax[i] * j / 10.);
            pDC->MoveTo(pt - dpt);
            pDC->LineTo(pt + dpt);
        }
    }
    pDC->SelectObject(pPenOld);
}

void CViewObj::DrawGraph1(CDC* pDC, TDDObj* pDD)
{
    int count = 0;

    DrawAxisRect(pDC, pDD);
    for (int i = 0; i < pDD->m_arrRD.GetSize(); ++i)
        if (pDD->m_arrRD[i].m_pSch->m_bShown) ++count;
    if (count == 0) return;

    int w = 800 / (count * 3 + 1);
    int x = -350 + w;

    for (int i = 0; i < pDD->m_arrRD.GetSize(); ++i)
    {
        TRectData& rd = pDD->m_arrRD[i];
        if (!rd.IsShown()) continue;

        int y = 350 - max(20, int(GetObj(0, rd.m_pSch) * 800. / m_arrMax[0]));
        rd.m_pSch->DrawRect(pDC, CRect(x, y, x + w * 2, 350), strEmpty);
        x += w * 3 + 1;
    }
}

void CViewObj::DrawGraph2(CDC* pDC, TDDObj* pDD)
{
    DrawAxisRect(pDC, pDD);
    CRect rect(CPoint(330, 370), titleSize);
    pDC->SetTextAlign(TA_TOP | TA_RIGHT);
    DrawFancy(pDC, rect, GetRealIndex(0), false);

    for (int i = 0; i < pDD->m_arrRD.GetSize(); ++i)
    {
        TRectData& rd = pDD->m_arrRD[i];
        if (!rd.IsShown()) continue;

        int x = -350 + int(GetObj(0, rd.m_pSch) * 800. / m_arrMax[0]);
        int y = +350 - int(GetObj(1, rd.m_pSch) * 800. / m_arrMax[1]);
        rd.m_pSch->DrawRect(pDC, CRect(x - 15, y - 15, x + 15, y + 15), strEmpty);
    }
}

void CViewObj::DrawGraphM(CDC* pDC, TDDObj* pDD)
{
    for (int i = pDD->m_arrRD.GetSize(); --i >= 0;)
    {
        TRectData& rd = pDD->m_arrRD[i];
        if (!rd.IsShown()) continue;

        CPen pen(PS_SOLID, 15, rd.m_pSch->m_color);
        CPen* pPenOld = pDC->SelectObject(&pen);

        pDC->MoveTo(ObjToXY(0, rd.m_pSch));
        for (int i = 1; i < GetAxisCount(); ++i) AndrewLineTo(pDC, ObjToXY(i, rd.m_pSch));
        AndrewLineTo(pDC, ObjToXY(0, rd.m_pSch));

        pDC->SelectObject(pPenOld);
    }
    DrawAxisStar(pDC, pDD);
}

void CViewObj::DrawGraph(CDC* pDC, TDDObj* pDD)
{
    for (int i = 0; i < GetAxisCount(); ++i)
    {
        m_arrMax[i] = 1;
        EnumerateSch m_arrMax[i] = max(double(GetObj(i, pSch)), m_arrMax[i]);
        EnumerateEnd m_arrMax[i] /= 0.9;
    }

    switch (GetAxisCount())
    {
        case 1:
            DrawGraph1(pDC, pDD);
            break;
        case 2:
            DrawGraph2(pDC, pDD);
            break;
        default:
            DrawGraphM(pDC, pDD);
            break;
    }
}

void CViewObj::DrawLegends(CDC* pDC, TDDObj* pDD)
{
    CFont font;
    theApp.CreateAppFont(font, pDC, 1);
    CFont* pFontOld = pDC->SelectObject(&font);
    pDC->SetTextAlign(TA_TOP | TA_LEFT);

    for (int i = 0; i < pDD->m_arrRD.GetSize(); ++i)
    {
        TRectData& rd = pDD->m_arrRD[i];
        DrawCheck(pDC, rd.m_rectCheck, rd.IsShown());
        CRect rect(rd.m_rectMain);

        if (!pDD->m_bPrinting)
        {
            bool bDown = rd.m_pSch == pSchActive;
            pDC->Draw3dRect(rect, bDown ? colorDarkest() : colorLight(), colorDarkest());
            rect.DeflateRect(1, 1);
            pDC->Draw3dRect(rect, bDown ? colorDark() : colorLighty(), bDown ? colorLight() : colorDark());
            rect.DeflateRect(1, 1);

            if (bDown)
            {
                ++rect.left;
                ++rect.top;
            }
            else
            {
                --rect.right;
                --rect.bottom;
            }
        }

        rect.DeflateRect(1, 1);
        int h = rect.Height();
        CRect rect2(rect);
        rect2.right = rect.left + h;
        rd.m_pSch->DrawRect(pDC, rect2, strEmpty);

        rect2.left = rect2.right + h / 2;
        rect2.right = rect.right;
        TextInRect(pDC, rect2, rd.m_pSch->m_id);
    }

    pDC->SelectObject(pFontOld);
}

/////////////////////////////////////////////////////////////////////////////
// CViewObj message handlers

CViewSmart::TDDSmart* CViewObj::CreateDD(bool bPrinting)
{
    return new TDDObj(bPrinting);
}

CSize CViewObj::OnScreenUpdate(CDC* pDC, TDDSmart* pDD)
{
    if (!pSchActive) return super::OnScreenUpdate(pDC, pDD);
    CRect rect;
    GetClientRect(rect);
    CSize sz = CalcLegend(pDC, rect.Size(), (TDDObj*)pDD);
    return sz;
}

void CViewObj::Click(UINT nFlags, CPoint point)
{
    TDDObj* pDD = (TDDObj*)m_pDDPaint;

    for (int i = 0; i < pDD->m_arrRD.GetSize(); ++i)
    {
        TRectData& rd = pDD->m_arrRD[i];
        if (rd.m_rectMain.PtInRect(point))
        {
            theApp.SetSchActive(rd.m_pSch, false);
            break;
        }
        else if (rd.m_rectCheck.PtInRect(point))
        {
            rd.m_pSch->m_bShown = !rd.IsShown();
            RedrawWindow();
            break;
        }
    }
}

void CViewObj::DblClick(UINT nFlags, CPoint point)
{
    TDDObj* pDD = (TDDObj*)m_pDDPaint;
    int i = pSchActive == NULL ? -1 : pDD->FindSch(pSchActive);
    bool bInRect = false;

    if (i != -1) bInRect = pDD->m_arrRD[i].m_rectMain.PtInRect(point) != 0;

    AfxGetMainWnd()->SendMessage(WM_COMMAND, bInRect ? ID_LOG_EDIT : ID_TOOL_OBJ);
}

void CViewObj::DrawView(CDC* pDC, CSize sz, TDDSmart* pDD)
{
    TDDObj* pDD2 = (TDDObj*)pDD;
    if (!m_axisMask || !pSchActive) return;

    DrawLegends(pDC, pDD2);

    CPoint shift = pDC->GetViewportOrg();
    pDC->SetMapMode(MM_ANISOTROPIC);

    CSize& szMain = pDD2->m_szMain;
    CSize szMain2(szMain);
    szMain2.cx /= 2;
    szMain2.cy /= 2;

    pDC->SetWindowExt(1000, 1000);
    pDC->SetViewportExt(szMain);
    pDC->SetViewportOrg(shift + szMain2);
    DrawGraph(pDC, pDD2);
    pDC->SetWindowExt(1, 1);
    pDC->SetViewportExt(1, 1);
}

int CViewObj::OnPrintUpdate(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    if (!pSchActive) return 1;
    CalcLegend(pDC, m_rectCenter.Size(), (TDDObj*)pDD);
    return 1;
}
