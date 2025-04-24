#include "StdAfx.h"

#include "AFLibBtnOwner.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::CBtnOwner;

IMPLEMENT_DYNAMIC(CBtnOwner, super)

CBtnOwner::CBtnOwner() : m_draw(ODT_BUTTON)
{
    m_pToolTip = new CToolTipCtrl;
    m_bCheckbox = false;
}

CBtnOwner::~CBtnOwner()
{
    delete m_pToolTip;
}

void CBtnOwner::InitToolTip()
{
    GetWindowText(m_tip);
    if (m_tip.IsEmpty()) return;

    m_pToolTip->Create(this);
    m_pToolTip->AddTool(this, m_tip);
}

void CBtnOwner::SetCheckbox(bool bCheckbox, bool bRedraw)
{
    m_bCheckbox = bCheckbox;
    if (!m_bCheckbox) m_draw.m_bChecked = false;
    if (bRedraw) RedrawWindow();
}

void CBtnOwner::EnableButtonToolTip(bool bEnable)
{
    if (IsButtonToolTipEnabled() == bEnable) return;

    if (bEnable)
    {
        m_pToolTip = new CToolTipCtrl;
        if (m_hWnd != NULL) InitToolTip();
    }
    else
    {
        if (m_pToolTip->m_hWnd != NULL) m_pToolTip->DestroyToolTipCtrl();
        m_pToolTip = NULL;
    }
}

bool CBtnOwner::IsButtonToolTipEnabled()
{
    return m_pToolTip != NULL;
}

void CBtnOwner::PreSubclassWindow()
{
    super::PreSubclassWindow();
    OnInit();
}

void CBtnOwner::OnInit()
{
    ASSERT(GetStyle() & BS_OWNERDRAW);
    if (IsButtonToolTipEnabled()) InitToolTip();
}

BEGIN_MESSAGE_MAP(CBtnOwner, super)
ON_WM_CREATE()
ON_MESSAGE(BM_GETCHECK, OnBmGetCheck)
ON_MESSAGE(BM_SETCHECK, OnBmSetCheck)
END_MESSAGE_MAP()

LRESULT CBtnOwner::OnBmGetCheck(WPARAM wParam, LPARAM lParam)
{
    return m_draw.m_bChecked ? BST_CHECKED : BST_UNCHECKED;
}

LRESULT CBtnOwner::OnBmSetCheck(WPARAM wParam, LPARAM lParam)
{
    if (!m_bCheckbox) return 0;
    bool bChecked = wParam == BST_CHECKED;
    if (bChecked == m_draw.m_bChecked) return 0;

    m_draw.m_bChecked = bChecked;
    RedrawWindow();
    return 0;
}

void CBtnOwner::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    m_draw.StartDraw(lpDrawItemStruct);

    try
    {
        GetItemText();
        m_draw.StartDrawButton();
        m_draw.StartRegion();
        Draw(&m_draw.m_dc);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    m_draw.Finish();
}

void CBtnOwner::GetItemText()
{
    m_draw.GetItemText();
}

void CBtnOwner::Draw(CDC* pDC)
{}

BOOL CBtnOwner::PreTranslateMessage(MSG* pMsg)
{
    if (m_pToolTip->GetSafeHwnd() != NULL) m_pToolTip->RelayEvent(pMsg);
    return super::PreTranslateMessage(pMsg);
}

int CBtnOwner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;
    OnInit();
    return 0;
}
