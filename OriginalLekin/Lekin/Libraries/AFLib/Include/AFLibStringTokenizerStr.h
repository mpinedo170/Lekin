#pragma once

#include "AFLibStringTokenizerAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TStringTokenizerStr
// Tokenizes the input string.
// Token delimiter is the whole separator string.

namespace AFLibIO {
class AFLIB TStringTokenizerStr : public TStringTokenizerAbstract
{
private:
    typedef TStringTokenizerAbstract super;
    DEFINE_COPY_AND_ASSIGN(TStringTokenizerStr);

    CString m_sSep;  // separator string

protected:
    virtual void ProtectedReadStr(TStringBuffer& sDest);
    virtual void PostRead();

public:
    TStringTokenizerStr(LPCTSTR sPtr, LPCTSTR sSep);

    virtual bool SkipIfEmpty();
};
}  // namespace AFLibIO
