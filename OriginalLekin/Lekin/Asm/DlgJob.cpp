#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgJob.h"

#include "DocTmpSmart.h"

#include "Job.h"
#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgJob dialog

CDlgJob::CDlgJob() : m_btnStyle(tyJob, 0)
{
    m_due = 0;
    m_release = 0;
    m_start = 0;
    m_stop = 0;
    m_tardiness = 0;
    m_weight = 0;
    m_WT = 0;

    m_pOp = NULL;
    Create(CDlgJob::IDD, NULL);
    m_btnStyle.SubclassDlgItem(IDC_STYLE, this);
    m_lstSeq.m_firstTabIDC = IDC_LAB0;
    m_lstSeq.SubclassDlgItem(IDC_ROUTE, this);
    SetIcon(theApp.LoadIcon(IDR_JOB), true);
    SetStdLimitText();
}

void CDlgJob::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_DUE, m_due);
    DDX_Text(pDX, IDC_RELEASE, m_release);
    DDX_Text(pDX, IDC_START, m_start);
    DDX_Text(pDX, IDC_STOP, m_stop);
    DDX_Text(pDX, IDC_T, m_tardiness);
    DDX_Text(pDX, IDC_WEIGHT, m_weight);
    DDX_Text(pDX, IDC_WT, m_WT);
}

BEGIN_MESSAGE_MAP(CDlgJob, super)
ON_LBN_SELCHANGE(IDC_ROUTE, OnSelchangeRoute)
ON_WM_TIMER()
ON_EN_KILLFOCUS(IDC_DUE, OnKillfocusDue)
ON_EN_KILLFOCUS(IDC_WEIGHT, OnKillfocusWeight)
ON_EN_KILLFOCUS(IDC_RELEASE, OnKillfocusRelease)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgJob message handlers

void CDlgJob::FillData(TOperation* pOpNew)
{
    if (pOpNew) m_pOp = pOpNew;
    if (!m_pOp) return;

    TJob* pJob = m_pOp->m_pJob;
    m_release = pJob->m_release;
    m_due = pJob->m_due;
    m_weight = pJob->m_weight;
    m_btnStyle.m_style = *pJob;

    m_start = pJob->GetStart();
    m_stop = pJob->GetStop();
    m_tardiness = pJob->GetT();
    m_WT = m_tardiness * m_weight;

    m_lstSeq.SetRedraw(false);
    m_lstSeq.ResetContent();

    for (int i = 0; i < pJob->GetOpCount(); ++i)
    {
        TOperation* pOp = pJob->m_arrOp[i];
        m_lstSeq.AddData(pOp);
        if (pOp == m_pOp) m_lstSeq.SetCurSel(i);
    }

    m_lstSeq.SetRedraw(true);
    SetWindowText(pJob->m_id);
    m_btnStyle.RedrawWindow();
    UpdateData(false);
}

void CDlgJob::OnCancel()
{
    theApp.CloseDlgJob();
}

void CDlgJob::Modify()
{
    theApp.m_pTmpJob->Modify();
    theApp.m_pTmpSeq->Modify(false);
}

void CDlgJob::Recompute()
{
    EnumerateSch pSch->Recompute(pSch == pSchActive);
    EnumerateEnd Modify();
}

bool CDlgJob::TestFocus()
{
    CWnd* pWndFocus = GetFocus();
    if (pWndFocus == this || (pWndFocus != NULL && pWndFocus->GetParent() == this)) return true;

    UpdateData(false);
    return false;
}

void CDlgJob::OnSelchangeRoute()
{
    m_pOp = (TOperation*)m_lstSeq.GetSingleData();
    theApp.UpdateFloaters();
}

void CDlgJob::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerStyle)
    {
        m_pOp->m_pJob->m_color = m_btnStyle.m_style.m_color;
        Modify();
    }
}

void CDlgJob::OnKillfocusDue()
{
    if (!TestFocus()) return;

    int due = m_due;
    if (!UpdateData(true))
    {
        m_due = due;
        UpdateData(false);
        return;
    }
    if (m_due == due) return;

    m_pOp->m_pJob->m_due = m_due;
    Recompute();
}

void CDlgJob::OnKillfocusWeight()
{
    if (!TestFocus()) return;

    int weight = m_weight;
    if (!UpdateData(true))
    {
        m_weight = weight;
        UpdateData(false);
        return;
    }
    if (m_weight == weight) return;

    m_pOp->m_pJob->m_weight = m_weight;
    Recompute();
}

void CDlgJob::OnKillfocusRelease()
{
    if (!TestFocus()) return;

    int release = m_release;
    if (!UpdateData(true))
    {
        m_release = release;
        UpdateData(false);
        return;
    }
    if (m_release == release) return;

    m_pOp->m_pJob->m_release = m_release;
    Recompute();
}

BOOL CDlgJob::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        CWnd* pWnd = GetFocus();
        if (pWnd)
        {
            GetDlgItem(IDCANCEL)->SetFocus();
            pWnd->SetFocus();
        }
        return 1;
    }
    return super::PreTranslateMessage(pMsg);
}
