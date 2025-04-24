#include "StdAfx.h"

#include "AppAsm.h"
#include "DocSmart.h"

#include "DocTmpSmart.h"
#include "FrmSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocSmart

IMPLEMENT_DYNCREATE(CDocSmart, super)

CDocSmart::CDocSmart()
{}

BEGIN_MESSAGE_MAP(CDocSmart, super)
END_MESSAGE_MAP()

CString CDocSmart::GetTitle2()
{
    return strEmpty;
};

void CDocSmart::SetModifiedFlag2(bool bModified)
{
    if (bModified) SetModifiedFlag();
    GetDocTemplate()->GetFrm()->UpdateTitle();
    UpdateAllViews(NULL);
}

CDocTmpSmart* CDocSmart::GetDocTemplate() const
{
    CDocTemplate* pTmp = super::GetDocTemplate();
    ASSERT_KINDOF(CDocTmpSmart, pTmp);
    return (CDocTmpSmart*)pTmp;
}

void CDocSmart::AddToRecentList(LPCTSTR lpszPathName)
{
    CDocTmpSmart* pTmp = GetDocTemplate();
    ASSERT(pTmp->m_pRecent);
    pTmp->m_pRecent->Add(lpszPathName);
}

bool CDocSmart::IsEmpty()
{
    return true;
}

bool CDocSmart::IsModified2()
{
    return IsModified() != 0;
}

BOOL CDocSmart::OnSaveDocument(LPCTSTR lpszPathName)
{
    if (!super::OnSaveDocument(lpszPathName)) return false;
    AddToRecentList(lpszPathName);
    CDocTmpSmart* pTmp = GetDocTemplate();
    pTmp->GetFrm()->UpdateTitle();
    if (pTmp->GetID() == tvwSeq) theApp.m_pTmpLog->GetFrm()->UpdateTitle();
    return true;
}

BOOL CDocSmart::OnOpenDocument(LPCTSTR lpszPathName)
{
    if (!super::OnOpenDocument(lpszPathName)) return false;
    AddToRecentList(lpszPathName);
    return true;
}

void CDocSmart::ReportSaveLoadException(
    LPCTSTR lpszPathName, CException* pExc, BOOL bSaving, UINT nIDPDefault)
{
    if (pExc->IsKindOf(RUNTIME_CLASS(CExcMessage)))
    {
        dynamic_cast<CExcMessage*>(pExc)->PrefixLine(IDS_ERROR_LOADING);
        pExc->ReportError();
        return;
    }
    super::ReportSaveLoadException(lpszPathName, pExc, bSaving, nIDPDefault);
}
