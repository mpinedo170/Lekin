#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgInvalidate.h"

#include "DocTmpSmart.h"

#include "Job.h"
#include "Misc.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgInvalidate dialog

CDlgInvalidate::CDlgInvalidate() : super(CDlgInvalidate::IDD)
{}

void CDlgInvalidate::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_PROMPT, m_Prompt);
}

BEGIN_MESSAGE_MAP(CDlgInvalidate, super)
ON_BN_CLICKED(IDC_OKALL, OnOkAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInvalidate message handlers

void CDlgInvalidate::OnOkAll()
{
    EndDialog(IDC_OKALL);
}

/////////////////////////////////////////////////////////////////////////////
// Global Functions

bool InvalidateJobs(TJobArray& arrJobsToInvalidate, TOperationArray& arrOpOrphan)
{
    bool bConfirmed = false;
    bool bModify = false;

    for (int i = 0; i < arrJobsToInvalidate.GetSize(); ++i)
    {
        TJob* pJob = arrJobsToInvalidate[i];
        if (!pJob->IsValid())
        {
            if (!bConfirmed)
            {
                if (bModify) theApp.m_pTmpJob->Modify();
                bModify = false;

                CDlgInvalidate dlg;
                AfxFormatString1(dlg.m_Prompt, IDS_BAD_ROUTE, pJob->m_id);
                switch (dlg.DoModal())
                {
                    case IDCANCEL:
                        return false;
                    case IDC_OKALL:
                        bConfirmed = true;
                }
            }
            pJob->Invalidate(arrOpOrphan);
            bModify = true;
        }
    }

    if (bModify) theApp.m_pTmpJob->Modify();
    return true;
}

void DeleteEmptyJobs()
{
    if (TJob::DeleteEmpty()) theApp.m_pTmpJob->Modify();
}

bool InvalidateAll()
{
    if (!GotWkc()) return false;
    TJob::InvalidateShop();

    if (TJob::GetShop() == OneShop && arrWorkcenter.GetSize() > 1)
    {
        if (AfxMB2(MB_OKCANCEL, IDP_EXTRA_WKC, WktGetLabel(WktGet())) != IDOK) return false;
        arrWorkcenter.DestroyAt(1, 9999);
        theApp.m_pTmpMch->Modify();
    }

    TOperationArray arrOpOrphan;
    bool bOk = InvalidateJobs(arrJob, arrOpOrphan);
    DeleteEmptyJobs();
    arrOpOrphan.DestroyAll();

    return bOk;
}
