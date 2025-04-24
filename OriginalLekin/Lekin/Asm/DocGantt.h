#pragma once

#include "DocSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocGantt document

class CDocGantt : public CDocSmart
{
private:
    typedef CDocSmart super;

protected:
    CDocGantt();
    virtual CString GetTitle2();
    virtual bool IsEmpty();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CDocGantt)
};
