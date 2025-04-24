#include "StdAfx.h"

#include "AFLibDCStateSaver.h"

using AFLibGui::TDCStateSaver;

TDCStateSaver::TDCStateSaver(CDC* pDC) :
    m_pDC(pDC),
    m_pBrushOld(pDC->GetCurrentBrush()),
    m_pFontOld(pDC->GetCurrentFont()),
    m_pPalOld(pDC->GetCurrentPalette()),
    m_pPenOld(pDC->GetCurrentPen()),
    m_colorText(pDC->GetTextColor()),
    m_colorBack(pDC->GetBkColor()),
    m_modeBack(pDC->GetBkMode()),
    m_align(pDC->GetTextAlign())
{
    m_pDC->SetBkMode(TRANSPARENT);
    m_pDC->SetTextAlign(TA_LEFT | TA_TOP);
}

TDCStateSaver::~TDCStateSaver()
{
    Close();
}

void TDCStateSaver::Close()
{
    if (m_pDC == NULL) return;
    m_pDC->SetTextAlign(m_align);
    m_pDC->SetBkMode(m_modeBack);
    m_pDC->SetBkColor(m_colorBack);
    m_pDC->SetTextColor(m_colorText);
    m_pDC->SelectObject(m_pPenOld);
    m_pDC->SelectObject(m_pPalOld);
    m_pDC->SelectObject(m_pFontOld);
    m_pDC->SelectObject(m_pBrushOld);
    m_pDC = NULL;
}
