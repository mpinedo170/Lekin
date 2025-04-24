#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgAddJob.h"

#include "DlgLoadRoute.h"
#include "DlgRoute.h"

#include "DataUnits.h"
#include "Job.h"
#include "Misc.h"
#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgAddJob dialog

CDlgAddJob::CDlgAddJob() : super(IDD), m_btnStyle(tyJob, IDC_JOB_ID)
{
    m_section = _T("DefaultJob");
    m_mode = modeAdd;

    AddDU(new TIDDataUnit(_T("ID"), IDC_JOB_ID, JobID(), _T("J001"), IDS_JOB));
    AddDU(new TStrDataUnit(_T("Name"), IDC_JOB_NAME, JobName(), strEmpty, strEmpty, 0, MXI));
    AddDU(new TCountDataUnit(
        strEmpty, IDC_JOB_COUNT, IDC_JOB_SPIN, m_jobCount, 0, MaxJob, arrJob.GetSize(), IDS_JOB));
    AddDU(new TIntDataUnit(_T("Release"), IDC_RELEASE, m_release, 0, 0, 0, MX9));
    AddDU(new TIntDataUnit(_T("Due"), IDC_DUE, m_due, 0, 0, 0, MX9));
    AddDU(new TIntDataUnit(_T("Weight"), IDC_WEIGHT, m_weight, 1, 0, 0, MX9));
    AddDU(new TIntDataUnit(strEmpty, IDC_PROCESSING, m_proc, 1, 0, 0, MX9));
    AddDU(new TStatusDataUnit(strEmpty, IDC_STATUS, m_status));

    m_btnStyle.m_style.RandomColor();
    m_rectRoute.SetRectEmpty();
    m_bSingle = TJob::GetShop() == OneShop;
}

void CDlgAddJob::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;

    // check whether ID is unique if adding or changing a single job
    switch (m_mode)
    {
        case modeAdd:
            if (m_jobCount > 1) return;
            break;

        case modeSingle:
            if (equals(JobID(), m_idOld)) return;
            break;

        case modeMultiple:
            return;
    }

    if (arrJob.Exists(JobID())) ReportDupID(pDX, IDC_JOB_ID, IDS_JOB);
}

BEGIN_MESSAGE_MAP(CDlgAddJob, super)
ON_EN_CHANGE(IDC_JOB_COUNT, OnChangeJobCount)
ON_BN_CLICKED(IDC_LOAD, OnLoad)
ON_BN_CLICKED(IDC_ROUTE, OnRoute)
ON_WM_PAINT()
END_MESSAGE_MAP()

void CDlgAddJob::AddJob(TJob& job)
{
    if (m_jobCount == 0)
    {
        m_btnStyle.m_style = job;
        m_route.Load(job);
    }
    else
    {
        JobID().Empty();
        TRoute route;
        route.Load(job);
        if (route != m_route) m_route.Clear();
    }

    FillMultiSel(m_jobCount, m_release, job.m_release);
    FillMultiSel(m_jobCount, m_due, job.m_due);
    FillMultiSel(m_jobCount, m_weight, job.m_weight);

    if (job.GetOpCount() > 0)
    {
        FillMultiSel(m_jobCount, m_proc, job.GetProcTime());
        FillMultiSel(m_jobCount, m_status, job.m_arrOp[0]->m_status);
    }

    ++m_jobCount;
    m_mode = m_jobCount == 1 ? modeSingle : modeMultiple;
}

void CDlgAddJob::ReadJob(TJob& job)
{
    if (m_mode != modeMultiple)
    {
        job.m_id = JobID();
        job.m_color = m_btnStyle.m_style.m_color;
    }

    ReadMultiSel(m_release, job.m_release);
    ReadMultiSel(m_due, job.m_due);
    ReadMultiSel(m_weight, job.m_weight);

    if (m_route.GetCount() > 0)
        m_route.Save(job);
    else if (m_bSingle && job.GetOpCount() > 0)
    {
        ReadMultiSel(m_proc, job.m_arrOp[0]->m_proc);
        ReadMultiSel(m_status, job.m_arrOp[0]->m_status);
    }
}

bool CDlgAddJob::Route1()
{
    ASSERT(!m_bSingle);
    if (m_route.GetCount() != 1) return true;

    CString s;
    m_editProc.GetWindowText(s);
    int proc = StrToInt(s);
    if (proc <= 0) return false;

    CString idWkc = m_route.GetWkcID(0);
    TCHAR status = m_route.GetStatus(0);
    m_route.Clear();
    m_route.Add(idWkc, proc, status);
    return true;
}

void CDlgAddJob::UpdateProcessing()
{
    ASSERT(!m_bSingle);
    int count = m_route.GetCount();
    if (count > 0) m_editProc.SetWindowText(IntToStr(m_route.GetTotal()));
    m_editProc.SetReadOnly(count != 1);
    RedrawWindow(m_rectRoute);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAddJob message handlers

BOOL CDlgAddJob::OnInitDialog()
{
    m_idOld = JobID();

    m_editProc.SubclassDlgItem(IDC_PROCESSING, this);
    m_btnStyle.SubclassDlgItem(IDC_STYLE, this);
    m_btnRoute.SubclassDlgItem(IDC_ROUTE, this);
    m_btnLoad.SubclassDlgItem(IDC_LOAD, this);

    m_btnLoad.SetTitle(_T("Load Route"));
    m_btnRoute.SetTitle(_T("Edit Route"));
    m_btnLoad.EnableWindow(arrJob.GetSize() > 0);

    if (m_jobCount == 0) m_jobCount = 1;

    if (m_bSingle)
    {
        if (m_route.IsValid())
        {
            m_proc = m_route.GetTotal();
            m_status = m_route.GetStatus(0);
        }
        m_route.Clear();
    }

    super::OnInitDialog();
    SetStdLimitText();

    CRect rect;
    GetWindowRect(rect);
    CRect rect2;
    GetDlgItem(IDC_STATUS)->GetWindowRect(rect2);
    rect.bottom = rect2.top;
    MoveWindow(rect, false);

    if (m_mode == modeAdd)
        ModifyLab();
    else
    {
        SetDlgItemReadOnly(IDC_JOB_COUNT, true);
        EnableDlgItem(IDC_JOB_SPIN, false);
    }

    if (m_bSingle)
    {
        static const UINT ones[] = {IDC_LAB_STATUS, IDC_STATUS, 0};

        static const UINT shops[] = {IDC_LAB_ROUTE, IDC_ROUTE, IDC_LOAD, 0};

        GetDlgItem(IDC_LAB_ROUTE)->GetWindowRect(rect);
        GetDlgItem(IDC_LAB_STATUS)->GetWindowRect(rect2);
        int dy = rect2.top - rect.top;

        ShowDlgItems(shops, false);

        for (int i = 0; ones[i] != 0; ++i)
        {
            CWnd* pWnd = GetDlgItem(ones[i]);
            pWnd->GetWindowRect(rect);
            ScreenToClient(rect);
            rect.OffsetRect(0, -dy);
            pWnd->MoveWindow(rect);
            pWnd->ShowWindow(SW_SHOW);
        }
    }
    else
    {
        GetDlgItem(IDC_LAB_ROUTE)->GetWindowRect(m_rectRoute);
        m_rectRoute.DeflateRect(0, 2);
        m_rectRoute.left = m_rectRoute.right + 10;

        GetDlgItem(IDC_LOAD)->GetWindowRect(rect);
        m_rectRoute.right = rect.right;
        ScreenToClient(m_rectRoute);

        UpdateProcessing();
    }

    CString s;
    GetWindowText(s);
    s += " (";
    s += TJob::GetSettingName();
    s += ")";
    SetWindowText(s);

    EnableDlgItem(IDC_JOB_ID, m_mode != modeMultiple);
    OnChangeJobCount();
    return true;
}

void CDlgAddJob::OnChangeJobCount()
{
    CString s;
    GetDlgItemText(IDC_JOB_COUNT, s);
    bool bOk = s == "1";
    if (m_btnStyle.GetSafeHwnd() == NULL) return;
    EnableDlgItem(IDC_LAB_STYLE, bOk);
    m_btnStyle.EnableWindow(bOk);
}

void CDlgAddJob::OnRoute()
{
    if (m_bSingle) return;
    Route1();
    CDlgRoute dlg(m_route);
    if (dlg.DoModal() != IDOK) return;
    UpdateProcessing();
}

void CDlgAddJob::OnLoad()
{
    if (m_bSingle) return;
    Route1();
    CDlgLoadRoute dlg;
    dlg.m_route = m_route;
    if (dlg.DoModal() != IDOK) return;
    m_route = dlg.m_route;
    UpdateProcessing();
}

void CDlgAddJob::OnOK()
{
    if (!UpdateData(true)) return;

    if (m_bSingle)
    {
        m_route.Clear();  // create a single-operation route if applicable
        if (m_proc > 0 && !m_status.IsEmpty()) m_route.Add(arrWorkcenter[0]->m_id, m_proc, m_status[0]);
    }
    else
    {
        if (!Route1())
        {
            AfxMB(IDP_POSITIVE);
            GetDlgItem(IDC_PROCESSING)->SetFocus();
            return;
        }

        if (m_mode != modeMultiple && m_route.GetTotal() == 0)
        {
            AfxMB(IDP_EMPTY_ROUTE);
            return;
        }
    }

    SaveData();
    ValidateData();
    EndDialog(IDOK);
}

void CDlgAddJob::OnPaint()
{
    CPaintDC dc(this);
    if (m_bSingle) return;
    m_route.DrawRect(&dc, m_rectRoute);
}

bool CDlgAddJob::IsDUNeeded(const TDataUnit* pDu)
{
    switch (pDu->m_idc)
    {
        case IDC_JOB_ID:
            return m_mode != modeMultiple;

        case IDC_JOB_COUNT:
            return m_mode == modeAdd;

        case IDC_PROCESSING:
        case IDC_STATUS:
            return m_bSingle;
    }

    return super::IsDUNeeded(pDu);
}

bool CDlgAddJob::IsDUMultiSel(const TDataUnit* pDu)
{
    return m_mode == modeMultiple;
}

void CDlgAddJob::SaveDU(const TDataUnit* pDu)
{
    if (pDu->m_idc == IDC_JOB_NAME && m_mode != modeAdd) return;
    super::SaveDU(pDu);
}
