#include "StdAfx.h"

#include "AFLibBtnTxt.h"

#include "AFLibGuiGlobal.h"
#include "AFLibStrings.h"

using AFLibGui::CBtnTxt;
using namespace AFLib;

IMPLEMENT_DYNAMIC(CBtnTxt, super)

CBtnTxt::CBtnTxt()
{
    m_color = colorAuto;
}

void CBtnTxt::SetTitle(LPCSTR title)
{
    m_title = title;
    if (m_hWnd != NULL) RedrawWindow();
}

void CBtnTxt::SetTitle(LPCWSTR title)
{
    m_title = title;
    if (m_hWnd != NULL) RedrawWindow();
}

void CBtnTxt::SetColor(COLORREF color, bool bRedraw)
{
    m_color = color;
    if (bRedraw) RedrawWindow();
}

void CBtnTxt::EnableButtonToolTip(bool bEnable)
{
    super::EnableButtonToolTip(bEnable);

    if (!bEnable)
    {
        int bufSize = GetWindowTextLength() + 1;
        ::GetWindowTextW(m_hWnd, m_title.GetBuffer(bufSize), bufSize);
        m_title.ReleaseBuffer();
    }
}

void CBtnTxt::SetArrowTitle(TArrow ar)
{
    static const WCHAR arrArrow[] = {chrTriangleUpW, chrTriangleDownW, chrTriangleLeftW, chrTriangleRightW};

    if (m_font.m_hObject == NULL)
    {
        CFont* pFont = GetParent()->GetFont();
        LOGFONT logFont = {0};
        pFont->GetLogFont(&logFont);
        _tcscpy_s(logFont.lfFaceName, LF_FACESIZE, strRoman);
        m_font.CreateFontIndirect(&logFont);
        SetFont(&m_font);
    }

    WCHAR title[] = {arrArrow[ar], 0};
    SetTitle(title);
}

BEGIN_MESSAGE_MAP(CBtnTxt, super)
END_MESSAGE_MAP()

void CBtnTxt::Draw(CDC* pDC)
{
    const CRect& rect = m_draw.m_rect;
    pDC->SetTextAlign(TA_CENTER | TA_TOP);

    int x = (rect.left + rect.right + 1) / 2;
    int y = m_draw.m_yText;

    if (m_draw.m_bDisabled)
    {
        pDC->SetTextColor(m_color == colorAuto ? colorLight() : GetColorLighter(m_color, 2));
        ::TextOutW(pDC->m_hDC, x + 1, y + 1, m_title, m_title.GetLength());
        pDC->SetTextColor(m_color == colorAuto ? colorDark() : GetColorLighter(m_color, 1));
    }
    else
        pDC->SetTextColor(m_color == colorAuto ? colorText() : m_color);

    ::TextOutW(pDC->m_hDC, x, y, m_title, m_title.GetLength());
}
