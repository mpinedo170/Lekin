#pragma once

#include "AFLibStringBuffer.h"
#include "AFLibStringTokenizerAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TStringTokenizerText
// Tokenizes the input string.
// Token delimiters are end-of-line characters (may be UNIX, MAC or DOS/WIN).

namespace AFLibIO {
class AFLIB TStringTokenizerText : public TStringTokenizerAbstract
{
private:
    typedef TStringTokenizerAbstract super;
    DEFINE_COPY_AND_ASSIGN(TStringTokenizerText);

    TStringBuffer m_sLastSep;  // last encountered separator

protected:
    virtual void ProtectedReadStr(TStringBuffer& sDest);
    virtual void PostRead();

public:
    explicit TStringTokenizerText(LPCTSTR sPtr);

    virtual bool SkipIfEmpty();

    // get the last separator (some combination of CR and LF)
    const TStringBuffer& GetLastSeparator()
    {
        return m_sLastSep;
    }
};
}  // namespace AFLibIO
