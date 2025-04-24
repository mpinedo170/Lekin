#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgAddOper.h"

#include "DataUnits.h"
#include "Job.h"
#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgAddOper dialog

CDlgAddOper::CDlgAddOper() : super(CDlgAddOper::IDD)
{
    m_section = _T("DefaultOp");
    AddDU(new TCbsIDDataUnit(_T("Wkc"), IDC_WKC_ID, m_idWkc, IDS_WKC));
    AddDU(new TIntDataUnit(_T("Processing"), IDC_PROCESSING, m_proc, 1, 0, 0, MX9));
    AddDU(new TStatusDataUnit(_T("Status"), IDC_STATUS, m_status));

    m_number = 0;
    m_pOp = NULL;
    m_pJob = NULL;
}

void CDlgAddOper::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_JOB_ID, m_idJob);
    DDX_Text(pDX, IDC_NUMBER, m_number);
    DDV_MinMaxInt(pDX, m_number, 1, m_maxNumber);
    if (!pDX->m_bSaveAndValidate) return;

    if (m_pJob->FindOperation(m_idWkc, m_pOp))
    {
        pDX->PrepareCtrl(IDC_WKC_ID);
        AfxMB(IDP_DUP_OPER);
        pDX->Fail();
    }
}

BEGIN_MESSAGE_MAP(CDlgAddOper, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAddOper message handlers

BOOL CDlgAddOper::OnInitDialog()
{
    m_cmbWkc.SubclassDlgItem(IDC_WKC_ID, this);
    m_spinNumber.SubclassDlgItem(IDC_NUMBER_SPIN, this);

    if (m_pOp)
    {
        m_pJob = m_pOp->m_pJob;
        m_number = m_pJob->m_arrOp.FindExact(m_pOp) + 1;
        m_idWkc = m_pOp->m_idWkc;
        m_proc = m_pOp->m_proc;
        m_status = m_pOp->m_status;
    }
    else
    {
        SetWindowText(_T("Add Operation"));
        TWorkcenter* pWkc = arrWorkcenter.FindKey(m_idWkc);
        if (!pWkc || m_pJob->FindOperation(pWkc->m_id))
        {
            EnumerateWkc if (!m_pJob->FindOperation(pWkc->m_id))
            {
                m_idWkc = pWkc->m_id;
                break;
            }
            EnumerateEnd
        }
    }

    m_idJob = m_pJob->m_id;
    m_maxNumber = m_pJob->GetOpCount() + 1;
    super::OnInitDialog();
    SetStdLimitText();

    if (TJob::GetShop() == FlowShop)
    {
        m_cmbWkc.EnableWindow(false);
        m_spinNumber.EnableWindow(false);
        EnableDlgItem(IDC_NUMBER, false);
        GetDlgItem(IDC_PROCESSING)->SetFocus();
    }
    else
    {
        m_spinNumber.SetRange(1, m_maxNumber);
        m_cmbWkc.SetFocus();
    }
    return false;
}

void CDlgAddOper::OnOK()
{
    if (!UpdateData(true)) return;
    if (m_pOp != NULL) m_pOp->DestroyThis();

    m_pOp = new TOperation(m_pJob, m_idWkc, m_proc, m_status[0]);
    m_pJob->m_arrOp.InsertAt(m_number - 1, m_pOp);

    SaveData();
    ValidateData();
    EndDialog(IDOK);
}
