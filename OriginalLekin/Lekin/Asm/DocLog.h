#pragma once

#include "DocSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CDocLog document

class CDocLog : public CDocSmart
{
private:
    typedef CDocSmart super;

protected:
    CDocLog();
    virtual bool IsEmpty();
    virtual bool IsModified2();

    DECLARE_DYNCREATE(CDocLog)
    DECLARE_MESSAGE_MAP()
};
