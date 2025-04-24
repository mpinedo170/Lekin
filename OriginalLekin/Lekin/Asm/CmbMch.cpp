#include "StdAfx.h"

#include "AppAsm.h"
#include "CmbMch.h"

#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CCmbMch

void CCmbMch::OnInit()
{
    super::OnInit();
    EnumerateWkc for (int i = 0; i < pWkc->GetMchCount(); ++i) AddData(pWkc->m_arrMch[i]);
    EnumerateEnd
}

void CCmbMch::Draw(CDC* pDC)
{
    TMachine* pMch = (TMachine*)m_draw.m_itemData;
    if (!pMch) return;

    CFont font;
    theApp.CreateSysFont(font, pDC);
    CFont* pFontOld = pDC->SelectObject(&font);
    CRect rect(m_draw.m_rect);
    rect.DeflateRect(2, 2);
    pMch->m_pWkc->DrawRect(pDC, rect, pMch->m_id);
    pDC->SelectObject(pFontOld);
}

int CCmbMch::Measure(int index)
{
    return super::Measure(index) + 2;
}
