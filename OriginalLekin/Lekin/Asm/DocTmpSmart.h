#pragma once

class CDocSmart;
class CViewSmart;
class CFrmSmart;

/////////////////////////////////////////////////////////////////////////////
// CDocTmpSmart document -- Document Template used for all documents

class CDocTmpSmart : public CMultiDocTemplate
{
private:
    typedef CMultiDocTemplate super;

    bool DoFile(LPCTSTR lpszPathName, int p1, int p2);

protected:
    CDocTmpSmart();

public:
    CRecentFileList* m_pRecent;
    TMargin* m_pMargin;

    CDocTmpSmart(
        UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
    ~CDocTmpSmart();

    virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = true);
    virtual CFrameWnd* CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther);

    int GetID();
    int GetFileID();
    CDlgPrint* CreateDlgPrint(bool bSetupOnly);

    CDocSmart* GetDoc();
    CViewSmart* GetView();
    CFrmSmart* GetFrm();
    void ActivateFrm();
    void Modify(bool bModified = true);
    void UpdateView();

    CString GetWindowTitle();
    CString GetTitle();
    CString GetExt();
    CString GetUntitled();

    CString Prompt();
    bool PromptOpen();
    bool SaveFile(LPCTSTR lpszPathName);
    bool AppendFile(LPCTSTR lpszPathName);

    DECLARE_DYNCREATE(CDocTmpSmart)
};
