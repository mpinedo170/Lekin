#include "StdAfx.h"

#include "AFLibDlgDitemDetails.h"

#include "AFLibDitem.h"

using AFLibGui::CDlgDitemDetails;
using namespace AFLib;

CDlgDitemDetails::CDlgDitemDetails()
{}

CDlgDitemDetails::CDlgDitemDetails(UINT nIDTemplate, CWnd* pWndParent) : super(nIDTemplate, pWndParent)
{
    m_pDitem = NULL;
}

CDlgDitemDetails::~CDlgDitemDetails()
{
    delete m_pDitem;
}

void CDlgDitemDetails::DoDataExchange(CDataExchange* pDX)
{
    m_pDitem->DDX(pDX);
    if (!pDX->m_bSaveAndValidate) return;

    bool bFound = false;
    BSearchStringsIC(m_pDitem->GetTitle(), m_arrBadNames, bFound);

    if (bFound)
    {
        AfxMBDupTitle();
        pDX->Fail();
    }
}

BEGIN_MESSAGE_MAP(CDlgDitemDetails, super)
END_MESSAGE_MAP()
