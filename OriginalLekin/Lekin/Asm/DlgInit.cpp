#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgInit.h"

#include "DataUnits.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgInit dialog

CDlgInit::CDlgInit() : super(CDlgInit::IDD)
{
    m_section = secSettings;
    m_wkt = WktGet();

    int maxMch = 0;
    UINT idsMch = 0;

    switch (m_wkt)
    {
        case wktSingle:
            break;

        case wktParallel:
            maxMch = MaxMch;
            idsMch = IDS_MCH;
            break;

        default:
            maxMch = MaxWkc;
            idsMch = IDS_WKC;
            break;
    }

    AddDU(new TCountDataUnit(
        _T("Job Count"), IDC_JOB_COUNT, IDC_JOB_SPIN, m_jobCount, MaxJob, MaxJob, 0, IDS_JOB));
    if (m_wkt != wktSingle)
        AddDU(new TCountDataUnit(
            _T("Machine Count"), IDC_MCH_COUNT, IDC_MCH_SPIN, m_mchCount, maxMch, maxMch, 0, idsMch));
}

BEGIN_MESSAGE_MAP(CDlgInit, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInit message handlers

BOOL CDlgInit::OnInitDialog()
{
    super::OnInitDialog();
    SetWindowText(WktGetLabel(m_wkt));
    SetStdLimitText();

    if (m_wkt == wktSingle)
    {
        static const UINT arrIDC[] = {IDC_MCH_COUNT, IDC_MCH_SPIN, 0};
        EnableDlgItems(arrIDC, false);
        SetDlgItemText(IDC_MCH_COUNT, _T("1"));
    }

    if (m_wkt > wktParallel) SetDlgItemText(IDC_LAB_MCH, _T("Number of workcenters"));

    return true;
}
