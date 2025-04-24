#include "StdAfx.h"

#include "AFLibBtnImg.h"

#include "AFLibGuiGlobal.h"
#include "AFLibImageAF.h"

using AFLibGui::CBtnImg;

IMPLEMENT_DYNAMIC(CBtnImg, super)

CBtnImg::CBtnImg()
{
    m_pImgRegular = new CImageAF;
    m_pImgDisabled = new CImageAF;
}

CBtnImg::~CBtnImg()
{
    if (m_pImgDisabled != m_pImgRegular) delete m_pImgDisabled;
    delete m_pImgRegular;
}

void CBtnImg::OnInit()
{
    super::OnInit();
    int nID = ::GetWindowLong(m_hWnd, GWL_ID);
    m_pImgRegular->LoadNeatBitmap(nID, CImageAF::bmcButton);
    m_pImgDisabled->LoadDisabledBitmap(nID, CImageAF::bmcButton);
}

BEGIN_MESSAGE_MAP(CBtnImg, super)
END_MESSAGE_MAP()

void CBtnImg::Draw(CDC* pDC)
{
    CImageAF* pImg = m_draw.m_bDisabled ? m_pImgDisabled : m_pImgRegular;
    if (pImg == NULL || !HBITMAP(*pImg)) return;

    const CRect& rect = m_draw.m_rect;
    CSize sz = pImg->GetSize();
    pImg->Draw(*pDC, rect.left + (rect.Width() - sz.cx + 1) / 2, rect.top + (rect.Height() - sz.cy + 1) / 2);
}
