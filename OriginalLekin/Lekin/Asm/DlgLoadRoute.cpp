#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgLoadRoute.h"

#include "Job.h"
#include "Misc.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadRoute dialog

CDlgLoadRoute::CDlgLoadRoute() : super(CDlgLoadRoute::IDD)
{}

void CDlgLoadRoute::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;
    if (m_route.GetCount() == 0)
    {
        LekinBeep();
        pDX->Fail();
    }
}

BEGIN_MESSAGE_MAP(CDlgLoadRoute, super)
ON_LBN_DBLCLK(IDC_ROUTE, OnDblclkRoute)
ON_LBN_SELCHANGE(IDC_ROUTE, OnSelchangeRoute)
END_MESSAGE_MAP()

void CDlgLoadRoute::GetRoute(int i, TRoute& route)
{
    route.Clear();
    TJob* pJob = (TJob*)m_lstRoute.GetItemDataPtr(i);
    if (!pJob) return;
    route.Load(*pJob);
}

void CDlgLoadRoute::SaveSelection()
{
    m_route.Clear();
    int i = m_lstRoute.GetCurSel();
    if (i < 0) return;
    GetRoute(i, m_route);
}

void CDlgLoadRoute::LoadSelection()
{
    int i = m_lstRoute.GetCount();
    while (--i >= 0)
    {
        TRoute route;
        GetRoute(i, route);
        if (route == m_route) break;
    }

    m_lstRoute.SetCurSel(i);
    EnableDlgItem(IDOK, i >= 0);
}

void CDlgLoadRoute::FillListBox()
{
    m_lstRoute.ResetContent();
    m_lstRoute.m_total = 1;
    EnumerateJob if (m_bNoDup)
    {
        TRoute route;
        route.Load(*pJob);
        for (int i = m_lstRoute.GetCount(); --i >= 0;)
        {
            TRoute route2;
            GetRoute(i, route2);
            if (route2 == route) goto NEXT;
        }
    }
    m_lstRoute.AddData(pJob);
    m_lstRoute.m_total = max(m_lstRoute.m_total, pJob->GetProcTime());
NEXT:;
    EnumerateEnd LoadSelection();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadRoute message handlers

BOOL CDlgLoadRoute::OnInitDialog()
{
    m_lstRoute.m_firstTabIDC = IDC_LAB0;
    m_lstRoute.SubclassDlgItem(IDC_ROUTE, this);
    super::OnInitDialog();
    m_lstRoute.SetFocus();
    return false;
}

void CDlgLoadRoute::OnDblclkRoute()
{
    OnOK();
}

void CDlgLoadRoute::OnSelchangeRoute()
{
    EnableDlgItem(IDOK, true);
}
