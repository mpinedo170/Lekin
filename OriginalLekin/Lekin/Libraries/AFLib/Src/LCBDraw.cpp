#include "StdAfx.h"

#include "AFLibLCBDraw.h"

#include "AFLibGuiGlobal.h"
#include "AFLibStringTokenizerAlt.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibGui::TLCBDraw;
using AFLibIO::TStringTokenizerAlt;

TLCBDraw::TLCBDraw(UINT CtlType)
{
    m_CtlType = CtlType;
    m_bDelFont = false;
    m_bChecked = false;
    PrivateClear();
    m_pRgnOld = NULL;
}

TLCBDraw::~TLCBDraw()
{
    PrivateClear();
}

void TLCBDraw::PrivateClear()
{
    if (m_bDelFont) delete m_pFont;
    m_index = -1;
    m_itemData = 0;
    m_rect.SetRectEmpty();
    m_bMeasure = m_bDelFont = false;
    m_bCtrlFocused = m_bSelected = m_bFocused = m_bDisabled = false;
    m_colBack = m_colText = colorBlack;
    m_hText = m_yText = 0;
    m_pFont = m_pFontOld = NULL;
    m_hWnd = NULL;
    m_hMenu = NULL;
}

void TLCBDraw::StartFont()
{
    CreateFont();
    m_pFontOld = m_dc.SelectObject(m_pFont);
}

void TLCBDraw::CreateFont()
{
    if (!IsMenu())
    {
        m_pFont = GetWnd()->GetFont();
        return;
    }

    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, false);
    m_pFont = new CFont;
    m_pFont->CreateFontIndirect(&metrics.lfMenuFont);
    m_bDelFont = true;
}

void TLCBDraw::ComputeHYText()
{
    m_hText = m_dc.GetTextExtent(strFontTest).cy;
    m_yText = m_rect.top + (m_rect.Height() - m_hText - 1) / 2;
}

bool TLCBDraw::IsLC()
{
    return m_CtlType == ODT_COMBOBOX || m_CtlType == ODT_LISTBOX;
}

bool TLCBDraw::IsButton()
{
    return m_CtlType == ODT_BUTTON;
}

bool TLCBDraw::IsMenu()
{
    return m_CtlType == ODT_MENU;
}

bool TLCBDraw::IsValid()
{
    return IsButton() || m_index != -1;
}

LPMEASUREITEMSTRUCT TLCBDraw::GetMIS()
{
    static MEASUREITEMSTRUCT mis;
    mis.CtlType = m_CtlType;
    mis.itemData = 0;
    mis.itemID = -1;
    return &mis;
}

bool TLCBDraw::HasStrings()
{
    int flag = m_CtlType == ODT_COMBOBOX ? CBS_HASSTRINGS : LBS_HASSTRINGS;
    return (GetWnd()->GetStyle() & flag) != 0;
}

void TLCBDraw::MiniStart(HANDLE handle)
{
    if (m_CtlType == ODT_MENU)
    {
        m_hMenu = HMENU(handle);
        m_hWnd = HWND_DESKTOP;
    }
    else
        m_hWnd = HWND(handle);
}

bool TLCBDraw::StartDraw(LPDRAWITEMSTRUCT lpDrawItemStruct, bool bFocusDetected)
{
    ASSERT(m_CtlType == lpDrawItemStruct->CtlType);
    m_itemData = lpDrawItemStruct->itemData;
    m_index = lpDrawItemStruct->itemID;
    MiniStart(lpDrawItemStruct->hwndItem);

    bool bAcceptSelection = m_CtlType != ODT_LISTBOX || (GetWnd()->GetStyle() & LBS_NOSEL) == 0;

    m_bCtrlFocused = GetFocus() == m_hWnd;
    if (IsMenu()) m_bChecked = (lpDrawItemStruct->itemState & ODS_CHECKED) != 0;
    m_bSelected = bAcceptSelection && (lpDrawItemStruct->itemState & ODS_SELECTED);
    m_bFocused = (lpDrawItemStruct->itemState & ODS_FOCUS) != 0;
    m_bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;
    m_rect = lpDrawItemStruct->rcItem;

    m_colBack = GetSysColor(IsButton()    ? COLOR_BTNFACE
                            : m_bSelected ? COLOR_HIGHLIGHT
                            : IsLC()      ? COLOR_WINDOW
                                          : COLOR_MENU);

    m_colText = GetSysColor(m_bDisabled   ? COLOR_GRAYTEXT
                            : IsButton()  ? COLOR_BTNTEXT
                            : m_bSelected ? COLOR_HIGHLIGHTTEXT
                                          : COLOR_WINDOWTEXT);

    if (bFocusDetected && m_bSelected && !m_bCtrlFocused)
    {
        m_colBack = colorNormal();
        m_colText = colorWindowText();
    }

    m_dc.Attach(lpDrawItemStruct->hDC);
    m_dc.FillSolidRect(m_rect, m_colBack);
    m_dc.SetBkMode(TRANSPARENT);
    m_dc.SetTextColor(m_colText);
    m_dc.SetTextAlign(TA_LEFT | TA_TOP);

    if (bFocusDetected && m_bFocused && m_bCtrlFocused) DrawFocusRect(&m_dc, m_rect);

    StartFont();
    return IsValid();
}

void TLCBDraw::StartDrawButton()
{
    if (m_bChecked)
    {
        CBrush brush;
        CreateHalftoneBrush(brush);
        m_dc.FillRect(m_rect, &brush);
    }

    if (m_bSelected || m_bChecked)
    {
        m_dc.Draw3dRect(m_rect, colorDarkest(), colorDarkest());
        m_rect.DeflateRect(1, 1);
        m_dc.Draw3dRect(m_rect, colorDark(), colorDark());
    }
    else
    {
        m_dc.Draw3dRect(m_rect, colorLight(), colorDarkest());
        m_rect.DeflateRect(1, 1);
        m_dc.Draw3dRect(m_rect, colorLighty(), colorDark());
    }

    if (m_bSelected)
    {
        ++m_rect.left;
        ++m_rect.top;
    }
    else
    {
        --m_rect.right;
        --m_rect.bottom;
    }

    m_rect.DeflateRect(1, 1);
}

void TLCBDraw::StartRegion()
{
    m_pRgnOld = new CRgn;
    m_pRgnOld->CreateRectRgn(0, 0, 0, 0);
    GetClipRgn(m_dc, *m_pRgnOld);
    CRgn rgn;
    rgn.CreateRectRgnIndirect(m_rect);
    m_dc.SelectClipRgn(&rgn, RGN_AND);
    ComputeHYText();
}

void TLCBDraw::StartMeasure(HANDLE handle, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    ASSERT(m_CtlType == lpMeasureItemStruct->CtlType);
    m_itemData = lpMeasureItemStruct->itemData;
    m_index = lpMeasureItemStruct->itemID;
    lpMeasureItemStruct->itemWidth = 0;
    lpMeasureItemStruct->itemHeight = 0;

    MiniStart(handle);
    m_bMeasure = true;
    HDC hDC = ::GetDC(m_hWnd);
    m_dc.Attach(hDC);
    StartFont();
}

void TLCBDraw::SelectFont(CFont* pFont, bool bDelFont)
{
    m_dc.SelectObject(pFont);
    if (m_bDelFont) delete m_pFont;
    m_pFont = pFont;
    m_bDelFont = bDelFont;
    ComputeHYText();
}

void TLCBDraw::GetItemText()
{
    m_line.Empty();
    if (IsMenu()) return;

    if (IsButton())
    {
        GetWnd()->GetWindowText(m_line);
        return;
    }

    if (!IsValid() || !HasStrings()) return;

    int msg = m_CtlType == ODT_COMBOBOX ? CB_GETLBTEXTLEN : LB_GETTEXTLEN;
    int length = GetWnd()->SendMessage(msg, m_index);

    msg = m_CtlType == ODT_COMBOBOX ? CB_GETLBTEXT : LB_GETTEXT;
    GetWnd()->SendMessage(msg, m_index, LPARAM(m_line.GetBuffer(length + 1)));
    m_line.ReleaseBuffer();
}

void TLCBDraw::Draw(const CUIntArray& arrTab)
{
    TStringTokenizerAlt sp(m_line, _T("\t\n"));
    int x = m_rect.left;

    for (int tab = 0; !sp.IsEof(); ++tab)
    {
        CString s = sp.ReadStr();
        m_dc.TextOut(x, m_yText, s);

        if (tab >= arrTab.GetSize()) break;
        if (sp.GetLastSeparator() == chrNewLine) break;
        x += arrTab[tab];
    }
}

void TLCBDraw::Finish()
{
    if (m_pRgnOld != NULL)
    {
        m_dc.SelectClipRgn(m_pRgnOld);
        delete m_pRgnOld;
        m_pRgnOld = NULL;
    }

    if (m_dc.m_hDC != NULL)
    {
        m_dc.SelectObject(m_pFontOld);
        HDC hDC = m_dc.m_hDC;
        m_dc.Detach();
        if (m_bMeasure) ::ReleaseDC(m_hWnd, hDC);
    }

    PrivateClear();
}

CSize TLCBDraw::Measure()
{
    CString s = m_line;
    if (s.IsEmpty()) s = strFontTest;
    return m_dc.GetTextExtent(s);
}

int TLCBDraw::CompareItems(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
    MiniStart(lpCompareItemStruct->hwndItem);
    if (!HasStrings()) return lpCompareItemStruct->itemData1 - lpCompareItemStruct->itemData2;

    LPCTSTR str1 = LPCTSTR(lpCompareItemStruct->itemData1);
    LPCTSTR str2 = LPCTSTR(lpCompareItemStruct->itemData2);
    return _tcsicmp(str1, str2);
}
