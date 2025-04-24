#pragma once

#include "DocSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocSeq document

class CDocSeq : public CDocSmart
{
private:
    typedef CDocSmart super;

protected:
    CDocSeq();

public:
    virtual void Serialize(CArchive& ar);
    virtual void DeleteContents();
    virtual void SetTitle(LPCTSTR lpszTitle);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

    virtual CString GetTitle2();
    virtual void SetModifiedFlag2(bool bModified);
    virtual bool IsEmpty();

    DECLARE_DYNCREATE(CDocSeq)
    DECLARE_MESSAGE_MAP()
};
