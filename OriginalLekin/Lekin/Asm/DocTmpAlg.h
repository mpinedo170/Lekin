#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDocTmpSmart document -- Document Template for "Algorithm" documents

class CDocTmpAlg : public CMultiDocTemplate
{
private:
    typedef CMultiDocTemplate super;

protected:
    CDocTmpAlg();

public:
    CDocTmpAlg(
        UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = true);

    DECLARE_DYNCREATE(CDocTmpAlg)
};
