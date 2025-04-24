#include "StdAfx.h"

#include "AppAsm.h"
#include "CmbWkc.h"

#include "Workcenter.h"

void CCmbWkc::OnInit()
{
    super::OnInit();
    EnumerateWkc AddString(pWkc->m_id);
    EnumerateEnd
}

void CCmbWkc::Draw(CDC* pDC)
{
    if (!m_draw.IsValid()) return;
    TWorkcenter* pWkc = arrWorkcenter.FindKey(LPCTSTR(m_draw.m_line));
    if (!pWkc) return;

    CFont font;
    theApp.CreateSysFont(font, pDC);
    CFont* pFontOld = pDC->SelectObject(&font);
    CRect rect(m_draw.m_rect);
    rect.DeflateRect(2, 2);
    pWkc->DrawRect(pDC, rect);
    pDC->SelectObject(pFontOld);
}

int CCmbWkc::Measure(int index)
{
    return super::Measure(index) + 2;
}
