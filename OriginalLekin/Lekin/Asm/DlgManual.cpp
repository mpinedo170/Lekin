#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgManual.h"

#include "DataUnits.h"
#include "Job.h"
#include "Machine.h"
#include "Schedule.h"
#include "Sequence.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgManual dialog

CDlgManual::CDlgManual(TSchedule* pSch) : super(CDlgManual::IDD), m_pSch(pSch)
{
    m_pMch = NULL;
    AddDU(new TIDDataUnit(strEmpty, IDC_SCH_ID, m_pSch->m_id, m_pSch->m_id, IDS_SCH));
    AddDU(new TIntDataUnit(
        strEmpty, IDC_TIME, m_pSch->m_arrObj[objTime], m_pSch->m_arrObj[objTime], 0, 1, MX9));
}

BEGIN_MESSAGE_MAP(CDlgManual, super)
ON_CBN_SELCHANGE(IDC_MCH_ID, OnSelchangeMchId)
ON_CONTROL_RANGE(BN_CLICKED, IDC_PREV, IDC_NEXT, OnNext)
END_MESSAGE_MAP()

bool CDlgManual::ReadSequence()
{
    if (m_pMch == NULL) return true;

    CString s;
    m_wndSeq.GetWindowText(s);
    LPCTSTR ss = s;
    TSequence seq(m_pMch);
    TSequence* pSeq = NULL;

    try
    {
        while (ss != NULL)
        {
            while (isspace(*ss)) ++ss;
            CString idJob = NextToken(ss, chrSemicolon);
            if (idJob.IsEmpty()) continue;
            TOperation* pOp = m_pMch->FindOper(idJob);
            seq.m_arrOp.Add(pOp);
        }
        pSeq = m_pSch->FindMch(m_pMch);
        ASSERT(pSeq != NULL);
        *pSeq = seq;
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
        m_cmbMch.SelData(m_pMch);
        m_wndSeq.SetFocus();
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgManual message handlers

BOOL CDlgManual::OnInitDialog()
{
    m_cmbMch.SubclassDlgItem(IDC_MCH_ID, this);
    m_wndSeq.SubclassDlgItem(IDC_SEQUENCE, this);
    super::OnInitDialog();

    CString s;
    GetWindowText(s);
    s += " - ";
    s += m_pSch->m_id;
    SetWindowText(s);

    m_cmbMch.SetCurSel(0);
    OnSelchangeMchId();
    m_wndSeq.SetFocus();
    return false;
}

void CDlgManual::OnSelchangeMchId()
{
    if (!ReadSequence()) return;
    m_pMch = reinterpret_cast<TMachine*>(m_cmbMch.GetData());
    ASSERT(m_pMch);
    TSequence* pSeq = m_pSch->FindMch(m_pMch);
    ASSERT(pSeq);

    CString s;
    for (int i = 0; i < pSeq->GetOpCount(); ++i)
    {
        if (i > 0) s += strSemicolon;
        s += pSeq->m_arrOp[i]->m_pJob->m_id;
    }

    m_wndSeq.SetWindowText(s);

    int sel = m_cmbMch.GetCurSel();
    EnableDlgItem(IDC_PREV, sel > 0);

    bool bLast = sel == m_cmbMch.GetCount() - 1;
    EnableDlgItem(IDC_NEXT, !bLast);
    SetDefID(bLast ? IDOK : IDC_NEXT);
}

void CDlgManual::OnOK()
{
    if (!ReadSequence()) return;

    if (!m_pSch->Recompute(false))
    {
        m_pSch->FormatError();
        return;
    }

    UpdateData(true);
    if (arrSchedule.Exists(m_pSch->m_id))
    {
        if (AfxMB2(MB_OKCANCEL, IDP_OVERWRITE_SCH, m_pSch->m_id) != IDOK) return;
    }
    EndDialog(IDOK);
}

void CDlgManual::OnNext(UINT id)
{
    int sel = m_cmbMch.GetCurSel();
    int dy = id == IDC_NEXT ? 1 : -1;
    m_cmbMch.SetCurSel(sel + dy);
    OnSelchangeMchId();
}
