#pragma once

#include "DocSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocMch document

class CDocMch : public CDocSmart
{
private:
    typedef CDocSmart super;

protected:
    CDocMch();

public:
    virtual void Serialize(CArchive& ar);
    virtual void DeleteContents();

    virtual CString GetTitle2();
    virtual void SetModifiedFlag2(bool bModified);
    virtual bool IsEmpty();

    DECLARE_DYNCREATE(CDocMch)
    DECLARE_MESSAGE_MAP()
};
