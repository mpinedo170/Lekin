#pragma once

#include "DocSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocJob document

class CDocJob : public CDocSmart
{
private:
    typedef CDocSmart super;

protected:
    CDocJob();

public:
    virtual void Serialize(CArchive& ar);
    virtual void DeleteContents();

    virtual CString GetTitle2();
    virtual void SetModifiedFlag2(bool bModified);
    virtual bool IsEmpty();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CDocJob)
};
