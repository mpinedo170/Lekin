#include "StdAfx.h"

#include "AppAsm.h"
#include "DocSeq.h"

#include "DocTmpSmart.h"
#include "Schedule.h"

IMPLEMENT_DYNCREATE(CDocSeq, super)

CDocSeq::CDocSeq()
{}

CString CDocSeq::GetTitle2()
{
    return pSchActive != NULL ? pSchActive->m_id : strEmpty;
}

void CDocSeq::SetModifiedFlag2(bool bModified)
{
    super::SetModifiedFlag2(bModified);
    theApp.m_pArrSchSorted->RemoveAll();

    for (int i = 1; i < tvwN; ++i)
    {
        CDocTmpSmart* pTmp = theApp.GetTmp(i);
        if (pTmp != GetDocTemplate()) pTmp->Modify(false);
    }
    theApp.UpdateFloaters();
}

bool CDocSeq::IsEmpty()
{
    return pSchActive == NULL;
}

BEGIN_MESSAGE_MAP(CDocSeq, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocSeq serialization

void CDocSeq::Serialize(CArchive& ar)
{
    SerializeSch(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CDocSeq commands

void CDocSeq::DeleteContents()
{
    theApp.CloseFloaters();
    theApp.SetSchActive(NULL, false);
    arrSchedule.DestroyAll();
    TSchedule::ClearTiming(TSchedule::Completely);
    super::DeleteContents();
}

void CDocSeq::SetTitle(LPCTSTR lpszTitle)
{
    if (lpszTitle == GetTitle()) return;
    super::SetTitle(lpszTitle);

    for (int i = 3; i < tvwN; ++i)
    {
        CDocSmart* pDoc = theApp.GetDoc(i);
        pDoc->SetTitle(lpszTitle);
        pDoc->UpdateAllViews(NULL);
    }
}

BOOL CDocSeq::OnOpenDocument(LPCTSTR lpszPathName)
{
    if (!super::OnOpenDocument(lpszPathName)) return false;
    if (arrSchedule.GetSize() > 0) theApp.SetSchActive(arrSchedule[0], false);
    return true;
}
