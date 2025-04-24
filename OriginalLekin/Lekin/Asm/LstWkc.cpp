#include "StdAfx.h"

#include "AppAsm.h"
#include "LstWkc.h"

#include "Workcenter.h"

void CLstWkc::Draw(CDC* pDC)
{
    TWorkcenter* pWkc = (TWorkcenter*)m_draw.m_itemData;
    if (!pWkc) return;

    CFont font;
    theApp.CreateSysFont(font, pDC);
    CFont* pFontOld = pDC->SelectObject(&font);
    CRect rect(m_draw.m_rect);
    rect.DeflateRect(4, 2);
    pWkc->DrawRect(pDC, rect);
    pDC->SelectObject(pFontOld);
}

int CLstWkc::Measure(int index)
{
    return super::Measure(index) + 4;
}
