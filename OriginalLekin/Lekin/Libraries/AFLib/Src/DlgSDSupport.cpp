#include "StdAfx.h"

#include "AFLibDlgSDSupport.h"

#include "AFLibDlgSelfDestruct.h"

using namespace AFLibGui;

IMPLEMENT_DYNAMIC(CDlgSDSupport, super)

CDlgSDSupport::CDlgSDSupport()
{
    m_pDlgSD = NULL;
}

CDlgSDSupport::CDlgSDSupport(UINT nIDTemplate, CWnd* pWndParent) : super(nIDTemplate, pWndParent)
{
    m_pDlgSD = NULL;
}

void CDlgSDSupport::DestroyDlgSD()
{
    if (m_pDlgSD == NULL) return;
    if (m_pDlgSD->m_hWnd != NULL) m_pDlgSD->DestroyWindow();
    delete m_pDlgSD;
    m_pDlgSD = NULL;
}

void CDlgSDSupport::ShowDlgSD(CDlgSelfDestruct* pDlg)
{
    DestroyDlgSD();
    m_pDlgSD = pDlg;
    m_pDlgSD->SDCreate();
}

BEGIN_MESSAGE_MAP(CDlgSDSupport, super)
ON_WM_DESTROY()
END_MESSAGE_MAP()

void CDlgSDSupport::OnDestroy()
{
    DestroyDlgSD();
    super::OnDestroy();
}
