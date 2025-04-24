#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgPrintMan.h"

#include "DocSmart.h"
#include "DocTmpSmart.h"
#include "ViewSmart.h"

#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPrintMan dialog

CDlgPrintMan::CDlgPrintMan() : super(CDlgPrintMan::IDD)
{
    AddDU(new TMskDataUnit(_T("Print"), IDC_WIN0, m_Win, (1 << tvwN) - 1, 0, tvwN, IDP_NO_WIN));
    AddDU(new TMskDataUnit(_T("PrintAll"), IDC_ALL0, m_All, 0x0E, 0, tvwN));
}

BEGIN_MESSAGE_MAP(CDlgPrintMan, super)
ON_BN_CLICKED(ID_HELP, OnHelp)
ON_WM_HELPINFO()
ON_CONTROL_RANGE(BN_CLICKED, IDC_WIN0, IDC_WIN5, OnChangeSel)
ON_CONTROL_RANGE(BN_CLICKED, IDC_PAGE_SETUP0, IDC_PAGE_SETUP5, OnPageSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPrintMan message handlers

BOOL CDlgPrintMan::OnInitDialog()
{
    super::OnInitDialog();

    for (int i = 0; i < tvwN; ++i)
    {
        CDocTmpSmart* pTmp = theApp.GetTmp(i);
        SetDlgItemText(IDC_WIN0 + i, pTmp->GetWindowTitle());

        if (pTmp->GetDoc()->IsEmpty())
        {
            const UINT arrIDC[] = {static_cast<UINT>(IDC_WIN0 + i), static_cast<UINT>(IDC_PAGE_SETUP0 + i),
                static_cast<UINT>(IDC_ALL0 + i), 0};
            EnableDlgItems(arrIDC, false);
            continue;
        }
        OnChangeSel(IDC_WIN0 + i);
    }

    //  SetDlgItemText(1154, _T("1"));
    return true;
}

void CDlgPrintMan::OnChangeSel(UINT nIDC)
{
    bool bEna = IsDlgButtonChecked(nIDC) != 0;
    int i = nIDC - IDC_WIN0;
    EnableDlgItem(IDC_PAGE_SETUP0 + i, bEna);
    if (arrSchedule.GetSize() <= 1) bEna = false;
    EnableDlgItem(IDC_ALL0 + i, bEna);
}

void CDlgPrintMan::OnPageSetup(UINT nIDC)
{
    int i = nIDC - IDC_PAGE_SETUP0;
    CDlgPrint* pDlg = theApp.GetTmp(i)->CreateDlgPrint(true);
    pDlg->m_pd.Flags |= PD_HIDEPRINTTOFILE;
    theApp.DoPrintDialog(pDlg);
}

void CDlgPrintMan::DoPrint()
{
    int win = 0, all = 0;

    theApp.BeginWaitCursor();
    for (int i = 0; i < tvwN; ++i)
    {
        CDocSmart* pDoc = theApp.GetDoc(i);
        if (pDoc->IsEmpty()) continue;
        win |= m_Win & (1 << i);
    }

    if (arrSchedule.GetSize() > 1) all = m_All & win;

    if (all != 0)
    {
        TSchedule* schOld = pSchActive;
        TScheduleArray& arrSchTemp = theApp.GetSchList();

        for (int si = 0; si < arrSchTemp.GetSize(); ++si)
        {
            theApp.SetSchActive(arrSchTemp[si], false);
            for (int i = 1; i < 4; ++i)
                if (win & all & (1 << i)) theApp.GetView(i)->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
        }
        theApp.SetSchActive(schOld, false);
    }

    for (int i = 0; i < tvwN; ++i)
        if (win & ~all & (1 << i)) theApp.GetView(i)->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);

    theApp.EndWaitCursor();
}
