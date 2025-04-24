#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgAddWkc.h"

#include "DlgSetup.h"

#include "DataUnits.h"
#include "Job.h"
#include "Misc.h"
#include "Schedule.h"
#include "Workcenter.h"
#include "Workspace.h"

namespace {
const CString keySetupDim = _T("SetupDim");
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAddWkc dialog

CDlgAddWkc::CDlgAddWkc() : super(CDlgAddWkc::IDD), m_btnStyle(tyWkc, IDC_WKC_ID)
{
    m_section = _T("DefaultWkc");
    m_mode = modeAdd;
    m_wkcCount = 0;
    m_bSetupChanged = false;
    m_bIDReadOnly = false;

    AddDU(new TIDDataUnit(_T("ID"), IDC_WKC_ID, WkcID(), _T("W001"), IDS_WKC));
    AddDU(new TStrDataUnit(_T("Name"), IDC_WKC_NAME, WkcName(), strEmpty, strEmpty, 0, MXI));
    AddDU(new TCountDataUnit(
        _T("MchCount"), IDC_MCH_COUNT, IDC_MCH_SPIN, m_mchCount, 1, MaxMch, GetTotalMchCount(), IDS_MCH));
    AddDU(new TIntDataUnit(_T("Release"), IDC_RELEASE, m_release, 0, 0, 0, MX9));
    AddDU(new TStatusDataUnit(_T("Status"), IDC_STATUS, m_status));

    m_btnStyle.m_style.RandomColor();
}

void CDlgAddWkc::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;

    switch (m_mode)
    {
        case modeSingle:
            if (equals(WkcID(), m_idOld)) return;
            break;

        case modeMultiple:
            return;
    }

    if (arrWorkcenter.Exists(WkcID())) ReportDupID(pDX, IDC_WKC_ID, IDS_WKC);
}

void CDlgAddWkc::AddWkc(TWorkcenter& wkc)
{
    if (m_wkcCount == 0)
    {
        m_btnStyle.m_style = wkc;
        m_setup = wkc.m_setup;
    }
    else
    {
        WkcID().Empty();
        if (m_setup != wkc.m_setup) m_setup.SetDim(0);
    }

    FillMultiSel(m_wkcCount, WkcName(), wkc.m_comment);
    FillMultiSel(m_wkcCount, m_mchCount, wkc.GetMchCount());
    FillMultiSel(m_wkcCount, m_release, wkc.m_release);
    FillMultiSel(m_wkcCount, m_status, wkc.m_status);

    ++m_wkcCount;
    m_mode = m_wkcCount == 1 ? modeSingle : modeMultiple;
}

void CDlgAddWkc::ReadWkc(TWorkcenter& wkc)
{
    if (m_mode != modeMultiple) wkc.m_color = m_btnStyle.m_style.m_color;

    ReadMultiSel(WkcName(), wkc.m_comment);
    ReadMultiSel(m_release, wkc.m_release);
    ReadMultiSel(m_status, wkc.m_status);

    if (m_bSetupChanged) wkc.m_setup = m_setup;
}

BEGIN_MESSAGE_MAP(CDlgAddWkc, super)
ON_BN_CLICKED(IDC_SETUP, OnSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAddWkc message handlers

BOOL CDlgAddWkc::OnInitDialog()
{
    m_idOld = WkcID();
    m_btnStyle.SubclassDlgItem(IDC_STYLE, this);

    if (!TWorkcenter::IsFlexible())
    {
        m_mchCount = 1;
        CString s;
        GetWindowText(s);
        s += _T(" (single machine)");
        SetWindowText(s);

        UINT arrIDC[] = {IDC_MCH_COUNT, IDC_MCH_SPIN, 0};
        EnableDlgItems(arrIDC, false);
    }
    else if (m_mode != modeAdd)
    {
        SetDlgItemReadOnly(IDC_MCH_COUNT, true);
        EnableDlgItem(IDC_MCH_SPIN, false);
    }

    super::OnInitDialog();
    SetStdLimitText();
    if (m_mode == modeAdd) SetWindowText(_T("New Workcenter"));

    if (m_mode == modeMultiple)
    {
        static const UINT NoMulti[] = {IDC_WKC_ID, IDC_STYLE, IDC_LAB_STYLE, 0};
        EnableDlgItems(NoMulti, false);
    }

    m_bIDReadOnly = pSchActive != NULL || TJob::GetShop() == OneShop;
    if (m_bIDReadOnly) SetDlgItemReadOnly(IDC_WKC_ID, true);

    return true;
}

void CDlgAddWkc::OnSetup()
{
    CDlgSetup dlg(m_setup, theApp.GetUserInt(m_section, keySetupDim, 5));
    if (dlg.DoModal() != IDOK) return;
    m_bSetupChanged = true;
}

void CDlgAddWkc::OnOK()
{
    if (!UpdateData(true)) return;
    SaveData();
    ValidateData();

    int dim = m_setup.GetDim();
    if (dim > 0) theApp.WriteUserInt(m_section, keySetupDim, dim);

    EndDialog(IDOK);
}

bool CDlgAddWkc::IsDUNeeded(const TDataUnit* pDu)
{
    switch (pDu->m_idc)
    {
        case IDC_WKC_ID:
            return !m_bIDReadOnly && m_mode != modeMultiple;

        case IDC_MCH_COUNT:
            return m_mode == modeAdd;
    }
    return super::IsDUNeeded(pDu);
}

bool CDlgAddWkc::IsDUMultiSel(const TDataUnit* pDu)
{
    return m_mode == modeMultiple;
}

void CDlgAddWkc::SaveDU(const TDataUnit* pDu)
{
    if (pDu->m_idc == IDC_WKC_NAME && m_mode != modeAdd) return;
    super::SaveDU(pDu);
}
