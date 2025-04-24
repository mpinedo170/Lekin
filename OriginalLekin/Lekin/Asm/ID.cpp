#include "StdAfx.h"

#include "AppAsm.h"
#include "ID.h"

#include "Draws.h"
#include "Job.h"
#include "Misc.h"
#include "Schedule.h"
#include "TokenSet.h"
#include "Workcenter.h"

////////////////////////////////////////////
// TID

extern const CString tokRGB = _T("RGB");

TID::TID(TType ty) : m_ty(ty), m_color(colorWhite)
{}

TID::TID(TType ty, LPCTSTR id, COLORREF color) : m_ty(ty), m_id(id), m_color(color)
{}

TID::~TID()
{}

CString TID::GetToken2(UINT id)
{
    CString s(MAKEINTRESOURCE(id));
    Capitalize(s);
    return s;
}

CString TID::GetToken(TType ty)
{
    if (ty > tySch) return strEmpty;
    return GetToken2(IDS_JOB + ty);
}

CString TID::GetTokenWkc2()
{
    return GetToken2(IDS_WKC2);
}

void TID::Write(CArchive& ar)
{
    CString s = GetToken();
    int tab = m_ty == tyMch ? 2 : 0;

    if (IsShort())
    {
        WriteToken(ar, tab, s, m_idGen);
        return;
    }

    WriteToken(ar, 0, s, m_id);
    if (!m_comment.IsEmpty()) WriteToken(ar, tab + 2, tokComment, m_comment);
    if (m_ty == tyMch) return;

    WriteToken(ar, 2, tokRGB,
        IntToStr(GetRValue(m_color)) + strSemicolon + IntToStr(GetGValue(m_color)) + strSemicolon +
            IntToStr(GetBValue(m_color)));
}

void TID::Read(TTokenSet2& ts)
{
    m_id = ts.GetAt(GetToken());
    CheckID(m_id, IDS_JOB + m_ty);

    m_comment = ts.GetAt(tokComment);
    if (m_ty == tyMch) return;

    CString s = ts.GetAt(tokRGB);
    if (s.IsEmpty())
    {
        RandomColor();
        return;
    }

    LPCTSTR ss = s;
    int r = StrToInt(NextToken(ss, chrSemicolon));
    int g = StrToInt(NextToken(ss, chrSemicolon));
    int b = StrToInt(NextToken(ss, chrSemicolon));
    m_color = RGB(r, g, b);
}

void TID::Save(TStringWriter& sw) const
{
    sw.WriteSafeStr(GetToken() + strSpace + m_id);
    if (!m_comment.IsEmpty())
    {
        sw.Write(_T("("));
        sw.WriteCStrNoDelim(m_comment);
        sw.WriteNoDelim(_T(")"));
    }

    sw.Write(tokRGB + strEqual + _T("(") + IntToStr(GetRValue(m_color)) + strComma +
             IntToStr(GetGValue(m_color)) + strComma + IntToStr(GetBValue(m_color)) + _T(")"));
}

void TID::Load(TStringSpacer& sp)
{
    sp.TestSymbolHard(GetToken());
    m_id = sp.ReadSafeStr();
    CheckID(m_id, IDS_JOB + m_ty);

    if (sp.TestSymbol(_T("(")))
    {
        m_comment = sp.ReadCStr();
        sp.TestSymbolHard(_T(")"));
    }

    if (!sp.TestSymbol(tokRGB))
    {
        RandomColor();
        return;
    }

    sp.TestSymbolHard(strEqual);
    sp.TestSymbolHard(_T("("));

    int r = sp.ReadInt();
    sp.TestSymbolHard(strComma);

    int g = sp.ReadInt();
    sp.TestSymbolHard(strComma);

    int b = sp.ReadInt();
    sp.TestSymbolHard(_T(")"));
    m_color = RGB(r, g, b);
}

const int GrayMchMax = 140;
const int GrayJobMin = 150;

void TID::RandomColor()
{
    int bestDiff = 0;
    int rep = 7, threshold = INT_MAX;

    TCollection* pArray = NULL;
    switch (m_ty)
    {
        case tyWkc:
            pArray = (TCollection*)&arrWorkcenter;
            break;
        case tyJob:
            pArray = (TCollection*)&arrJob;
            break;
        case tySch:
            pArray = (TCollection*)&arrSchedule;
            break;
    }

    if (pArray->GetSize() == 0)
        pArray = NULL;
    else
        threshold = int(300 / pow(double(pArray->GetSize()), 0.33));

    while (true)
    {
        int r = TRandom::Get().Uniform(256);
        int g = TRandom::Get().Uniform(256);
        int b = TRandom::Get().Uniform(256);
        COLORREF color = RGB(r, g, b);
        int gray = GetGray(color);

        if (gray < 48 || gray >= 240) continue;
        if (gray > GrayMchMax && m_ty == tyWkc) continue;
        if (gray < GrayJobMin && m_ty == tyJob) continue;
        if (!pArray)
        {
            m_color = color;
            break;
        }

        int diff = INT_MAX;
        for (int i = pArray->GetSize(); --i >= 0;) diff = min(diff, (*pArray)[i]->GetDiff(color));

        if (diff > bestDiff)
        {
            bestDiff = diff;
            m_color = color;
            if (bestDiff > threshold) break;
        }
        if (--rep <= 0) break;
    }
}

int TID::GetGray(COLORREF color)
{
    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);
    int mx = max(r, max(g, b));
    return (2 * r + 2 * g + b + 2 * mx) / 7;
}

static COLORREF colorLate(bool bLate)
{
    return bLate ? RGB(128, 0, 0) : colorBlack;
}

COLORREF TID::GetTextColor(bool bLate) const
{
    return GetGray() > GrayMchMax ? colorLate(bLate) : colorWhite;
}

COLORREF TID::GetHatchColor(bool bLate) const
{
    return m_ty == tySpec ? colorLate(bLate) : colorWhite - GetTextColor(bLate);
}

int TID::GetDiff(COLORREF color) const
{
    int s = 0;
    for (int i = 0; i < 24; i += 8) s += abs(int((color >> i) & 0xFF) - int((m_color >> i) & 0xFF));
    return s;
}

namespace {
void RoundDown(LONG& value, LONG where, int r)
{
    if (where >= value) return;
    value += (where - value) / r * r;
}

void RoundUp(LONG& value, LONG where, int r)
{
    if (where <= value) return;
    value -= (value - where) / r * r;
}

void Intersect(CRect& rect, const CRect& clip, int r)
{
    RoundUp(rect.left, clip.left, r);
    RoundUp(rect.top, clip.top, r);
    RoundDown(rect.right, clip.right, r);
    RoundDown(rect.bottom, clip.bottom, r);
}
}  // namespace

void TID::DrawRect(CDC* pDC, const CRect& rect, LPCTSTR text, bool bCross, bool bLate) const
{
    if (rect.IsRectEmpty()) return;
    CRect clip;
    pDC->GetClipBox(&clip);
    if ((rect & clip).IsRectEmpty()) return;

    int sv = pDC->SaveDC();
    CPoint shift = -pDC->GetViewportOrg();
    pDC->SetBrushOrg(7 - shift.x % 8, 7 - shift.y % 8);

    int k = pDC->GetDeviceCaps(LOGPIXELSX) / 10;
    CBrush brushMain;
    brushMain.CreateSolidBrush(m_color);
    CPen penMain(PS_SOLID, k / 8, colorLate(bLate));

    CPen* pPenOld = pDC->SelectObject(&penMain);
    CBrush* pBrushOld = pDC->SelectObject(&brushMain);
    pDC->Rectangle(rect);

    // hatch
    bool bInnerRect = false;
    CRect rectHatch(rect);
    rectHatch.DeflateRect(1, 1);

    switch (m_ty)
    {
        case tyWkc:
        {
            CPen penHatch(PS_SOLID, k / 8, GetHatchColor(bLate));
            pDC->SelectObject(&penHatch);

            Intersect(rectHatch, clip, k);
            int w = rectHatch.Width();
            int h = rectHatch.Height();

            for (int x = -h; x < w; x += k)
            {
                int x1 = x, y1 = rectHatch.top;
                if (x1 < 0)
                {
                    y1 += -x1;
                    x1 = 0;
                }

                int x2 = x + h, y2 = rectHatch.bottom;
                if (x2 > w)
                {
                    y2 -= x2 - w;
                    x2 = w;
                }

                pDC->MoveTo(rectHatch.left + x1, y1);
                pDC->LineTo(rectHatch.left + x2, y2);
                pDC->MoveTo(rectHatch.right - 1 - x1, y1);
                pDC->LineTo(rectHatch.right - 1 - x2, y2);
            }
            pDC->SelectObject(&penMain);
            break;
        }

        case tySpec:
        {
            if (k < 2) break;
            CPen penHatch(PS_SOLID, 0, GetHatchColor(bLate));
            pDC->SelectObject(&penHatch);
            CBrush brushHatch(GetHatchColor(bLate));
            pDC->SelectObject(&brushHatch);

            int r = max(k / 8, 1);
            int m = r * 2, s = r * 4;
            int dx = (rectHatch.Width() - 2 * m) % s / 2 + m;
            int dy = (rectHatch.Height() - 2 * m) % s / 2 + m;

            clip.InflateRect(s, s);
            Intersect(rectHatch, clip, s);

            for (int x = rectHatch.left + dx; x < rectHatch.right - dx + 2; x += s)
                for (int y = rectHatch.top + dy; y < rectHatch.bottom - dy + 2; y += s)
                    pDC->Ellipse(x - r, y - r, x + r + 1, y + r + 1);

            pDC->SelectObject(&penMain);
            pDC->SelectObject(&brushMain);
            bInnerRect = true;
            break;
        }
    }

    if (bCross)
    {
        pDC->MoveTo(rect.left, rect.top);
        pDC->LineTo(rect.right - 1, rect.bottom - 1);
        pDC->MoveTo(rect.left, rect.bottom - 1);
        pDC->LineTo(rect.right - 1, rect.top);
        bInnerRect = true;
    }

    if (text == NULL) text = m_id;
    if (text[0])
    {
        CRect rectText(rect);
        rectText.DeflateRect(2, 1);
        pDC->SetTextColor(GetTextColor(bLate));
        CSize szText = pDC->GetTextExtent(text, _tcslen(text)) + CSize(2, 1);

        CSize sz = rectText.Size() - szText;
        sz.cy = max(int(sz.cy) / 2, 0);
        sz.cx = max(int(sz.cx), 0);
        rectText.DeflateRect(0, sz.cy);

        int align = pDC->GetTextAlign();
        if (sz.cx == 0)
        {
            if (pDC->GetTextExtent(text, (_tcslen(text) + 1) / 2).cx <= rectText.Width())
                pDC->SetTextAlign(TA_LEFT);
            else
                goto NO_TEXT;
        }
        else if (align & TA_CENTER)
        {
            sz.cx /= 2;
            rectText.DeflateRect(sz.cx, 0);
        }
        else if (align & TA_RIGHT)
            rectText.left += sz.cx;
        else
            rectText.right -= sz.cx;

        if (bInnerRect) pDC->FillRect(rectText, &brushMain);
        if (bLate)
        {
            ++rectText.top;
            ++rectText.left;
            TextInRect(pDC, rectText, text);
            rectText.OffsetRect(-1, -1);
        }

        TextInRect(pDC, rectText, text);
        pDC->SetTextAlign(align);
    NO_TEXT:;
    }
    pDC->SelectObject(pPenOld);
    pDC->SelectObject(pBrushOld);
    pDC->RestoreDC(sv);
}

TObjective TID::m_IOFlag = objTime;

void TID::SetIOFlag(TObjective IOFlag)
{
    if (IOFlag == m_IOFlag) return;
    m_IOFlag = IOFlag;

    if (IsShort())
    {
        arrWorkcenter.GenerateIDs(_T("Wkc"));
        EnumerateWkc pWkc->m_arrMch.GenerateIDs(pWkc->m_idGen + strDot);
        EnumerateEnd arrJob.GenerateIDs(_T("Job"));
    }
    else
    {
        arrWorkcenter.ClearIDs();
        if (TWorkcenter::IsFlexible()) EnumerateWkc pWkc->m_arrMch.ClearIDs();
        EnumerateEnd arrJob.ClearIDs();
    }
}

bool TID::IsShort()
{
    return m_IOFlag != objTime;
}
