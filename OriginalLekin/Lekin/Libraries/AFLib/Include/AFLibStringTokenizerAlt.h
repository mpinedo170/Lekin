#pragma once

#include "AFLibStringTokenizerAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TStringTokenizerAlt
// Tokenizes the input string.
// Delimiter is ANY character from the separator string.

namespace AFLibIO {
class AFLIB TStringTokenizerAlt : public TStringTokenizerAbstract
{
private:
    typedef TStringTokenizerAbstract super;
    DEFINE_COPY_AND_ASSIGN(TStringTokenizerAlt);

    // set of token delimiters
    CString m_sSep;

    // last separator encountered
    TCHAR m_cLastSep;

protected:
    virtual void ProtectedReadStr(TStringBuffer& sDest);
    virtual void PostRead();

public:
    TStringTokenizerAlt(LPCTSTR sPtr, LPCTSTR sSep);

    virtual bool SkipIfEmpty();

    // get the last separator
    TCHAR GetLastSeparator()
    {
        return m_cLastSep;
    }
};
}  // namespace AFLibIO
