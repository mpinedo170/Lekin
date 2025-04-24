#pragma once

#include "DocSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocObj document

class CDocObj : public CDocSmart
{
private:
    typedef CDocSmart super;

protected:
    CDocObj();
    virtual bool IsEmpty();

    DECLARE_DYNCREATE(CDocObj)
    DECLARE_MESSAGE_MAP()
};
