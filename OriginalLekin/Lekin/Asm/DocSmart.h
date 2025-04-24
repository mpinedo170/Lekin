#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDocSmart document -- parent file for all document files

class CDocSmart : public CDocument
{
private:
    typedef CDocument super;

protected:
    CDocSmart();

public:
    CDocTmpSmart* GetDocTemplate() const;
    void AddToRecentList(LPCTSTR lpszPathName);

    void Save()
    {
        OnFileSave();
    }
    void SaveAs()
    {
        OnFileSaveAs();
    }

    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual void ReportSaveLoadException(LPCTSTR lpszPathName, CException* e, BOOL bSaving, UINT nIDPDefault);

    // overridables

    virtual CString GetTitle2();
    virtual void SetModifiedFlag2(bool bModified);
    virtual bool IsEmpty();
    virtual bool IsModified2();

    DECLARE_DYNCREATE(CDocSmart)
    DECLARE_MESSAGE_MAP()
};
