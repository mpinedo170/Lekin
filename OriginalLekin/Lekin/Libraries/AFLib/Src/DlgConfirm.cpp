#include "StdAfx.h"

#include "DlgConfirm.h"

#include "AFLibDlgRun.h"
#include "Local.h"

using AFLibPrivate::CDlgConfirm;
using namespace AFLibGui;

CDlgConfirm::CDlgConfirm(CDlgRun* pDlgParent) : m_pDlgParent(pDlgParent)
{}

BEGIN_MESSAGE_MAP(CDlgConfirm, super)
END_MESSAGE_MAP()

void CDlgConfirm::SDCreate()
{
    Create(IDD_CONFIRM, m_pDlgParent);
}

BOOL CDlgConfirm::OnInitDialog()
{
    super::OnInitDialog();
    CRect rectParent;
    m_pDlgParent->GetWindowRect(rectParent);
    CRect rect;
    GetWindowRect(rect);
    rect.MoveToXY(rectParent.left + (rectParent.Width() - rect.Width()) / 2,
        rectParent.top + (rectParent.Height() - rect.Height()) / 2);
    MoveWindow(rect);
    return false;
}

void CDlgConfirm::OnOK()
{
    m_pDlgParent->SendMessage(WM_TIMER, timerReallyCancel);
    super::OnOK();
}
