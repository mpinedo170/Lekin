#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgFlowRoute.h"

#include "ViewTree.h"

#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgFlowRoute dialog

CDlgFlowRoute::CDlgFlowRoute(TWorkcenter** route) : super(CDlgFlowRoute::IDD), m_route(route)
{}

BEGIN_MESSAGE_MAP(CDlgFlowRoute, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFlowRoute message handlers

BOOL CDlgFlowRoute::OnInitDialog()
{
    m_lstRoute.m_bDragEnabled = true;
    m_lstRoute.SubclassDlgItem(IDC_ROUTE, this);
    super::OnInitDialog();

    for (int i = 0; i < arrWorkcenter.GetSize(); ++i) m_lstRoute.AddData(m_route[i]);

    m_lstRoute.SetCurSel(0);
    m_lstRoute.SetFocus();
    return false;
}

void CDlgFlowRoute::OnOK()
{
    for (int i = 0; i < m_lstRoute.GetCount(); ++i)
        m_route[i] = reinterpret_cast<TWorkcenter*>(m_lstRoute.GetItemDataPtr(i));
    super::OnOK();
}
