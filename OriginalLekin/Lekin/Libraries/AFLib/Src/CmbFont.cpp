#include "StdAfx.h"

#include "AFLibCmbFont.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::CCmbFont;

IMPLEMENT_DYNAMIC(CCmbFont, super)

CCmbFont::CCmbFont()
{}

BEGIN_MESSAGE_MAP(CCmbFont, super)
END_MESSAGE_MAP()

int CALLBACK CCmbFont::EnumFontsProc(const LOGFONT* lplf, const TEXTMETRIC* lptm, DWORD dwType, LPARAM lpData)
{
    CCmbFont& combo = *reinterpret_cast<CCmbFont*>(lpData);

    if (!(lptm->tmPitchAndFamily & TMPF_TRUETYPE)) return 1;
    if (lplf->lfCharSet != ANSI_CHARSET) return 1;

    combo.AddString(lplf->lfFaceName);
    return 1;
}

void CCmbFont::OnInit()
{
    super::OnInit();
    CDC* pDC = GetDC();
    EnumFonts(pDC->m_hDC, NULL, EnumFontsProc, long(this));
    ReleaseDC(pDC);
}

void CCmbFont::Draw(CDC* pDC)
{
    if (!m_draw.IsValid()) return;

    const CRect& rect = m_draw.m_rect;
    CFont* pFont = new CFont;
    CreateAbsoluteFont(*pFont, m_draw.m_line, rect.Height() - 1, false, false);
    m_draw.SelectFont(pFont);
    super::Draw(pDC);
}
