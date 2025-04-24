#include "StdAfx.h"

#include "AppAsm.h"
#include "DocTmpAlg.h"

#include "TreePlug.h"

IMPLEMENT_DYNCREATE(CDocTmpAlg, super)

CDocTmpAlg::CDocTmpAlg() : CMultiDocTemplate(0, NULL, NULL, NULL)
{}

CDocTmpAlg::CDocTmpAlg(
    UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass) :
    super(nIDResource, pDocClass, pFrameClass, pViewClass)
{}

CDocument* CDocTmpAlg::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible)
{
    CTreePlug tree;
    tree.CreateDummy();
    tree.Init(true);

    if (tree.Import(lpszPathName)) tree.Save();
    return NULL;
}
