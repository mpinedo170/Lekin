#include "StdAfx.h"

#include "AppAsm.h"
#include "DocTmpSmart.h"

#include "DlgFlowRoute.h"
#include "DlgWorkspace.h"
#include "DocSmart.h"

#include "Job.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgWorkspace dialog

CDlgWorkspace::CDlgWorkspace() : super(CDlgWorkspace::IDD)
{
    AddDU(new TRadDataUnit(strEmpty, IDC_WKT0, *reinterpret_cast<int*>(&m_Workspace), WktGet(), 0, 6));

    m_bOldFlex = TWorkcenter::IsFlexible();
    m_oldShop = TJob::GetShop();
    m_route.Copy(arrWorkcenter);
    m_bRouteChanged = false;
}

BEGIN_MESSAGE_MAP(CDlgWorkspace, super)
ON_BN_CLICKED(IDC_ROUTE, OnRoute)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWorkspace message handlers

BOOL CDlgWorkspace::OnInitDialog()
{
    super::OnInitDialog();
    FillWorkspaces();
    if (arrWorkcenter.GetSize() <= 1) EnableDlgItem(IDC_ROUTE, false);
    return true;
}

void CDlgWorkspace::OnRoute()
{
    CDlgFlowRoute dlg(&m_route[0]);
    if (dlg.DoModal() == IDOK) m_bRouteChanged = true;
}

void CDlgWorkspace::OnOK()
{
    UpdateData(true);
    bool bNewFlex = WktIsFlexible(m_Workspace);
    TShop newShop = WktGetShop(m_Workspace);

    if (m_bOldFlex && !bNewFlex)
    {
        CString msg;
        AfxFormatString2(msg, IDP_DEGRADING, _T("machine"), strEmpty);
        if (!theApp.AskConfirmation(IDP_DEGRADING, 1, msg)) return;
    }

    if (newShop < m_oldShop)
    {
        CString s1 = TJob::GetLabel(m_oldShop);
        CString s2 = TJob::GetLabel(newShop);
        CString msg;
        AfxFormatString2(msg, IDP_DEGRADING, _T("job"), _T(" from ") + s1 + _T(" to ") + s2);
        if (!theApp.AskConfirmation(IDP_DEGRADING, 1, msg)) return;
    }

    if (!theApp.CheckSequences(true)) return;

    bool bModifyMch = false;

    if (m_bRouteChanged)
    {
        arrWorkcenter.Copy(m_route);
        bModifyMch = true;
    }

    if (bNewFlex != m_bOldFlex)
    {
        TWorkcenter::SetFlexible(bNewFlex);
        bModifyMch = true;
    }

    if (newShop == OneShop && arrWorkcenter.GetSize() > 1)
    {
        arrWorkcenter.DestroyAt(1, 9999);
        bModifyMch = true;
    }

    if (newShop != m_oldShop)
    {
        TJob::SetShop(newShop);
        theApp.m_pTmpJob->Modify();
    }

    if (bModifyMch) theApp.m_pTmpMch->Modify();

    EndDialog(IDOK);
}
