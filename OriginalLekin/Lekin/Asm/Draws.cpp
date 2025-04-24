#include "StdAfx.h"

#include "AppAsm.h"
#include "Draws.h"

#include "Misc.h"

/////////////////////////////////////////////////
// draw

void DrawUnder(CDC* pDC, const CRect& rect)
{
    CBrush brush;
    CreateHalftoneBrush(brush);
    pDC->FillRect(rect, &brush);
}

static void DrawHelper(CDC* pDC, const CRect& rect, bool bDraw, int& x, LPCTSTR s, bool bSub)
{
    CSize sz = pDC->GetTextExtent(s);
    int y = bSub ? rect.bottom - sz.cy : rect.top + (rect.Height() - sz.cy) / 2;
    if (bDraw) pDC->TextOut(x, y, s);
    x += sz.cx;
}

static int DrawFancy2(CDC* pDC, const CRect& rect, TObjective ob, bool bBold, bool bDraw)
{
    int fntSize = rect.Height();
    CFont fontNormal;
    CreateAbsoluteFont(fontNormal, strFntTimes, int(fntSize * 0.8), bBold, true);
    CFont fontSub;
    CreateAbsoluteFont(fontSub, strFntTimes, int(fntSize * 0.5), bBold, true);

    CFont* pFontOld = pDC->SelectObject(&fontNormal);
    pDC->SetTextAlign(TA_TOP | TA_LEFT);
    int x = rect.left;

    if (IsObjSum(ob))
    {
        CFont fontFancy;
        CreateAbsoluteFont(fontFancy, strFntFancy, fntSize, bBold, false);
        pDC->SelectObject(&fontFancy);
        DrawHelper(pDC, rect, bDraw, x, _T("S"), false);
        pDC->SelectObject(&fontNormal);
    }

    if (IsObjWeighted(ob))
    {
        DrawHelper(pDC, rect, bDraw, x, _T("w"), false);
        pDC->SelectObject(&fontSub);
        DrawHelper(pDC, rect, bDraw, x, _T("j"), true);
    }

    LPCTSTR labels[] = {_T("Time"), _T("C"), _T("T"), _T("U"), _T("C"), _T("T"), _T("C"), _T("T")};

    pDC->SelectObject(&fontNormal);
    DrawHelper(pDC, rect, bDraw, x, labels[ob], false);

    if (ob != objTime)
    {
        pDC->SelectObject(&fontSub);
        DrawHelper(pDC, rect, bDraw, x, IsObjMax(ob) ? _T("max") : _T("j"), true);
    }

    pDC->SelectObject(pFontOld);
    return x - rect.left;
}

int DrawFancy(CDC* pDC, const CRect& rect, TObjective ob, bool bBold)
{
    int align = pDC->GetTextAlign();
    if (align & TA_LEFT) return DrawFancy2(pDC, rect, ob, bBold, true);

    int dx = DrawFancy2(pDC, rect, ob, bBold, false);
    CRect rect2(rect);

    if (align & TA_CENTER)
        rect2.left += (rect.Width() - dx) / 2;
    else if (align & TA_RIGHT)
        rect2.left = rect.right - dx;

    int sz = DrawFancy2(pDC, rect2, ob, bBold, true);
    pDC->SetTextAlign(align);
    return sz;
}

void DrawCheck(CDC* pDC, const CRect& rect, bool bCheck)
{
    CRect rect2(rect);
    int pw = max(rect.Width() / 16, 2);
    CPen pen(PS_SOLID, pw, pDC->IsPrinting() ? colorBlack : colorWindowText());
    CPen* pPenOld = pDC->SelectObject(&pen);

    if (!pDC->IsPrinting())
    {
        pDC->Draw3dRect(rect2, colorDark(), colorLight());
        rect2.DeflateRect(1, 1);
        pDC->Draw3dRect(rect2, colorBlack, colorNormal());
        rect2.DeflateRect(1, 1);
        pDC->FillSolidRect(rect2, colorWindow());
    }
    else
    {
        CBrush brush;
        brush.CreateStockObject(NULL_BRUSH);
        CBrush* pBrushOld = pDC->SelectObject(&brush);
        pDC->Rectangle(rect2);
        rect2.DeflateRect(pw * 2, pw * 2);
        pDC->SelectObject(pBrushOld);
    }

    if (bCheck)
    {
        rect2.DeflateRect(pw, pw);
        pDC->MoveTo(rect2.left, rect2.top);
        pDC->LineTo(rect2.right - 1, rect2.bottom - 1);
        pDC->MoveTo(rect2.left, rect2.bottom - 1);
        pDC->LineTo(rect2.right - 1, rect2.top);
    }
    pDC->SelectObject(pPenOld);
}
