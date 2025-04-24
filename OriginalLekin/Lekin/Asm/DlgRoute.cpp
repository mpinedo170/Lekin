#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgRoute.h"

#include "DlgLoadRoute.h"
#include "DlgOppy.h"

#include "Job.h"
#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRoute dialog

CDlgRoute::CDlgRoute(TRoute& route) : super(CDlgRoute::IDD), m_route(route)
{}

CDlgRoute::~CDlgRoute()
{
    delete m_pDlgInternal;
}

void CDlgRoute::Disability()
{
    int sel = m_pDlgInternal->GetSel();
    int count = m_pDlgInternal->GetGoodCount();

    if (TJob::GetShop() == FlowShop)
    {
        for (int i = 0; i < cmN; ++i) m_arrButton[i].EnableWindow(false);
        return;
    }

    m_arrButton[cmUp].EnableWindow(sel > 0 && sel < count);
    m_arrButton[cmDown].EnableWindow(sel < count - 1);
    m_arrButton[cmNew].EnableWindow(count < arrWorkcenter.GetSize());
    m_arrButton[cmDel].EnableWindow(sel < count);
}

BEGIN_MESSAGE_MAP(CDlgRoute, super)
ON_BN_CLICKED(IDB_NEW, OnNew)
ON_BN_CLICKED(IDB_DEL, OnDel)
ON_BN_CLICKED(IDB_UP, OnUp)
ON_BN_CLICKED(IDB_DOWN, OnDown)
ON_BN_CLICKED(IDC_LOAD, OnLoad)
ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRoute message handlers

BOOL CDlgRoute::OnInitDialog()
{
    for (int i = 0; i < cmN; ++i) m_arrButton[i].SubclassDlgItem(IDB_UP + i, this);
    EnableDlgItem(IDC_LOAD, arrJob.GetSize() > 0);

    CRect rect;
    m_arrButton[cmN - 1].GetWindowRect(rect);
    ScreenToClient(rect);
    m_pDlgInternal = new CDlgOppy(this, rect.bottom, m_route);

    super::OnInitDialog();
    Disability();
    return false;
}

void CDlgRoute::OnNew()
{
    m_pDlgInternal->InsertNewRow();
    Disability();
}

void CDlgRoute::OnOK()
{
    if (!m_pDlgInternal->IsValid()) return;
    m_pDlgInternal->DlgToRoute(m_route);
    EndDialog(IDOK);
}

void CDlgRoute::OnDel()
{
    m_pDlgInternal->DeleteRow(m_pDlgInternal->GetSel());
    Disability();
}

void CDlgRoute::OnUp()
{
    m_pDlgInternal->MoveUp();
    Disability();
}

void CDlgRoute::OnDown()
{
    m_pDlgInternal->MoveDown();
    Disability();
}

void CDlgRoute::OnLoad()
{
    CDlgLoadRoute dlg;
    m_pDlgInternal->DlgToRoute(dlg.m_route);
    if (dlg.DoModal() != IDOK) return;
    m_pDlgInternal->RouteToDlg(dlg.m_route);
    Disability();
}

void CDlgRoute::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerRoute) Disability();
}
