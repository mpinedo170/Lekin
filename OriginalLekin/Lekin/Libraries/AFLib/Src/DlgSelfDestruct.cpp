#include "StdAfx.h"

#include "AFLibDlgSelfDestruct.h"

#include "AFLibBlockMBCounter.h"

using AFLibGui::CDlgSelfDestruct;

CDlgSelfDestruct::CDlgSelfDestruct()
{
    m_pBlock = NULL;
    m_bParentDisabled = false;
}

CDlgSelfDestruct::~CDlgSelfDestruct()
{
    delete m_pBlock;
}

BEGIN_MESSAGE_MAP(CDlgSelfDestruct, super)
ON_WM_CREATE()
ON_WM_ACTIVATE()
ON_WM_DESTROY()
END_MESSAGE_MAP()

int CDlgSelfDestruct::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    m_pBlock = new TBlockMBCounter;
    return super::OnCreate(lpCreateStruct);
}

// Destroy window on enter, cancel, or lost focus

void CDlgSelfDestruct::OnOK()
{
    DestroyWindow();
}

void CDlgSelfDestruct::OnCancel()
{
    DestroyWindow();
}

void CDlgSelfDestruct::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    super::OnActivate(nState, pWndOther, bMinimized);

    if (nState != WA_INACTIVE)
    {
        if (m_bParentDisabled) GetParent()->EnableWindow(true);
        m_bParentDisabled = false;
        return;
    }

    // detect when a modal dialog box is summoned
    for (; pWndOther != NULL; pWndOther = pWndOther->GetParent())
        if (pWndOther == this)
        {
            GetParent()->EnableWindow(false);
            m_bParentDisabled = true;
            return;
        }

    PostMessage(WM_CLOSE);
}

void CDlgSelfDestruct::OnDestroy()
{
    delete m_pBlock;
    m_pBlock = NULL;
    super::OnDestroy();
}
