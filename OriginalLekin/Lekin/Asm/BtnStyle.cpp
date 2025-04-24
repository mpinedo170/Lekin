#include "StdAfx.h"

#include "AppAsm.h"
#include "BtnStyle.h"

#include "DlgStyle.h"
#include "Draws.h"

BEGIN_MESSAGE_MAP(CBtnStyle, super)
ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
END_MESSAGE_MAP()

CBtnStyle::CBtnStyle(TType ty, UINT idc) : m_style(ty), m_idc(idc)
{}

void CBtnStyle::Draw(CDC* pDC)
{
    CRect rect(m_draw.m_rect);
    rect.DeflateRect(1, 1);
    TID style(m_style.m_ty);
    if (!m_draw.m_bDisabled) style.m_color = m_style.m_color;
    style.DrawRect(pDC, rect, strEmpty);
}

void CBtnStyle::OnClicked()
{
    TID style2(m_style);
    if (m_idc != 0) GetParent()->GetDlgItemText(m_idc, style2.m_id);

    if (!CDlgStyle::EditColor(&style2)) return;
    m_style.m_color = style2.m_color;
    RedrawWindow();
    GetParent()->PostMessage(WM_TIMER, timerStyle);
}
