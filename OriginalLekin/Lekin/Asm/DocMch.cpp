#include "StdAfx.h"

#include "AppAsm.h"
#include "DocTmpSmart.h"

#include "DocMch.h"
#include "FrmSmart.h"

#include "Job.h"
#include "Schedule.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDocMch

IMPLEMENT_DYNCREATE(CDocMch, super)

CDocMch::CDocMch()
{}

BEGIN_MESSAGE_MAP(CDocMch, super)
END_MESSAGE_MAP()

CString CDocMch::GetTitle2()
{
    return TWorkcenter::GetSettingName();
}

void CDocMch::SetModifiedFlag2(bool bModified)
{
    super::SetModifiedFlag2(bModified);
    TJob::InvalidateShop();
    theApp.m_pTmpJob->Modify(false);

    EnumerateSch pSch->Recompute(pSch == pSchActive);
    EnumerateEnd theApp.m_pTmpGantt->GetFrm()->RecalcLayout();
    theApp.m_pTmpSeq->Modify(false);
}

bool CDocMch::IsEmpty()
{
    return !GotWkc();
}

/////////////////////////////////////////////////////////////////////////////
// CDocMch serialization

void CDocMch::Serialize(CArchive& ar)
{
    SerializeMch(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CDocMch commands

void CDocMch::DeleteContents()
{
    theApp.CheckSequences(false);
    arrWorkcenter.DestroyAll();
    theApp.m_pTmpJob->Modify(false);
    super::DeleteContents();
}
