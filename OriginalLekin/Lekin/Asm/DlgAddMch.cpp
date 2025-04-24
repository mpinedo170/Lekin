#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgAddMch.h"

#include "DataUnits.h"
#include "Misc.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgAddMch dialog

CDlgAddMch::CDlgAddMch() : super(CDlgAddMch::IDD)
{
    m_bIDChange = false;
    m_mode = modeAdd;
    m_pWkc = NULL;

    AddDU(new TIDDataUnit(strEmpty, IDC_MCH_ID, m_idMch, strEmpty, IDS_MCH));
    AddDU(new TStrDataUnit(strEmpty, IDC_MCH_NAME, m_mchName, strEmpty, strEmpty, 0, MXI));
    AddDU(new TCountDataUnit(
        strEmpty, IDC_MCH_COUNT, IDC_MCH_SPIN, m_mchCount, 0, MaxMch, GetTotalMchCount(), IDS_MCH));
    AddDU(new TIntDataUnit(strEmpty, IDC_RELEASE, m_release, 0, 0, 0, MX9));
    AddDU(new TStatusDataUnit(strEmpty, IDC_STATUS, m_status));
}

void CDlgAddMch::DoDataExchange(CDataExchange* pDX)
{
    DDX_CBString(pDX, IDC_WKC_ID, m_idWkc);
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;

    switch (m_mode)
    {
        case modeAdd:
            if (m_mchCount > 1) return;
            break;

        case modeSingle:
            m_bIDChange = !equals(m_idMch, m_idOld);
            if (!m_bIDChange) return;
            break;

        case modeMultiple:
            return;
    }

    if (m_pWkc->FindMch(m_idMch)) ReportDupID(pDX, IDC_MCH_ID, IDS_MCH);
}

BEGIN_MESSAGE_MAP(CDlgAddMch, super)
END_MESSAGE_MAP()

void CDlgAddMch::AddMch(TMachine& mch)
{
    if (m_mchCount == 0)
        m_idMch = mch.m_id;
    else
        m_idMch.Empty();

    FillMultiSel(m_mchCount, m_idWkc, mch.m_pWkc->m_id);
    FillMultiSel(m_mchCount, m_mchName, mch.m_comment);
    FillMultiSel(m_mchCount, m_release, mch.m_release);
    FillMultiSel(m_mchCount, m_status, mch.m_status);

    ++m_mchCount;
    m_mode = m_mchCount == 1 ? modeSingle : modeMultiple;
}

void CDlgAddMch::ReadMch(TMachine& mch)
{
    if (m_mode != modeMultiple) mch.m_id = m_idMch;

    ReadMultiSel(m_mchName, mch.m_comment);
    ReadMultiSel(m_release, mch.m_release);
    ReadMultiSel(m_status, mch.m_status);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAddMch message handlers

BOOL CDlgAddMch::OnInitDialog()
{
    m_idOld = m_idMch;
    m_cmbWkc.SubclassDlgItem(IDC_WKC_ID, this);

    super::OnInitDialog();
    SetStdLimitText();
    EnableDlgItem(IDC_MCH_ID, m_mode != modeMultiple);

    if (!m_idWkc.IsEmpty())
    {
        m_pWkc = arrWorkcenter.FindKey(m_idWkc);
        ASSERT(m_pWkc != NULL);
    }

    if (m_mode == modeAdd)
        ModifyLab();
    else
    {
        SetDlgItemReadOnly(IDC_MCH_COUNT, true);
        EnableDlgItem(IDC_MCH_SPIN, false);
    }

    return true;
}

bool CDlgAddMch::IsDUNeeded(const TDataUnit* pDu)
{
    if (pDu->m_idc == IDC_JOB_COUNT) return m_mode == modeAdd;

    return super::IsDUNeeded(pDu);
}

bool CDlgAddMch::IsDUMultiSel(const TDataUnit* pDu)
{
    return m_mode == modeMultiple;
}
