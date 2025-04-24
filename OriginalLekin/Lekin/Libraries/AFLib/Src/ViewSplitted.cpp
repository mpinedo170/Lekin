#include "StdAfx.h"

#include "AFLibViewSplitted.h"

#include "AFLibStrings.h"

using namespace AFLib;
using AFLibGui::CViewSplitted;

IMPLEMENT_DYNAMIC(CViewSplitted, super)

CViewSplitted::CViewSplitted(UINT nIDTemplate) : super(nIDTemplate)
{
    m_bSplitterOn = true;
    m_xSplit = m_ySplit = 0;
    m_wSplit = 4;
    m_xLeftInset = 0;
    m_xRightInset = GetSystemMetrics(SM_CXVSCROLL);
}

void CViewSplitted::Split()
{
    if (m_bSplitterOn)
    {
        CRect rect;
        m_wndSplitter.GetWindowRect(rect);
        ScreenToClient(rect);
        m_xSplit = rect.left;
    }
    Resize();
}

void CViewSplitted::Fit()
{
    if (!m_bSplitterOn) return;
    m_xSplit = GetFitWidth();
    Resize();
}

BEGIN_MESSAGE_MAP(CViewSplitted, super)
END_MESSAGE_MAP()

void CViewSplitted::OnInit()
{
    CRect rect(0, 0, 4, 4);
    m_wndSplitter.Create(NULL, strEmpty, WS_CHILD | WS_VISIBLE, rect, this, -1);
    m_wndTopBorder.CreateEx(
        WS_EX_STATICEDGE, _T("Static"), strEmpty, WS_CHILD | WS_VISIBLE | SS_BLACKFRAME, rect, this, -1);
}

void CViewSplitted::OnResize()
{
    CRect rectCli;
    GetClientRect(rectCli);
    if (m_xSplit == 0) m_xSplit = max(rectCli.Width() / 4, 150);
    m_xSplit = max(m_xSplit, m_xLeftInset);
    m_xSplit = min(m_xSplit, rectCli.Width() - m_xRightInset - m_wSplit);

    AndrewMoveWindow(&m_wndTopBorder, 0, m_ySplit - 2, rectCli.Width(), 2);
    AndrewMoveWindow(&m_wndSplitter, m_xSplit, m_ySplit, m_wSplit, rectCli.Height() - m_ySplit);
}

int CViewSplitted::GetFitWidth()
{
    return 1;
}
