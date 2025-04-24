#include "StdAfx.h"

#include "AppAsm.h"
#include "DocJob.h"

#include "Job.h"

/////////////////////////////////////////////////////////////////////////////
// CDocJob

IMPLEMENT_DYNCREATE(CDocJob, super)

CDocJob::CDocJob()
{}

CString CDocJob::GetTitle2()
{
    return TJob::GetSettingName();
}

bool CDocJob::IsEmpty()
{
    return arrJob.GetSize() == 0;
}

void CDocJob::SetModifiedFlag2(bool bModified)
{
    super::SetModifiedFlag2(bModified);
    theApp.m_pArrJobSorted->RemoveAll();
    theApp.ScheduleMenu();
}

BEGIN_MESSAGE_MAP(CDocJob, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocJob serialization

void CDocJob::Serialize(CArchive& ar)
{
    SerializeJobs(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CDocJob commands

void CDocJob::DeleteContents()
{
    theApp.CheckSequences(false);
    arrJob.DestroyAll();
    UpdateAllViews(NULL);
    super::DeleteContents();
}
