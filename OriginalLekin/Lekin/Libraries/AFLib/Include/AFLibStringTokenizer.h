#pragma once

#include "AFLibStringTokenizerAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TStringTokenizer
// Tokenizes the input string.
// Token delimiter is the separator character;
//   character case is NEVER ignored.
// Works faster than TStringTokenizerStr.

namespace AFLibIO {
class AFLIB TStringTokenizer : public TStringTokenizerAbstract
{
private:
    typedef TStringTokenizerAbstract super;
    DEFINE_COPY_AND_ASSIGN(TStringTokenizer);

    TCHAR m_chSep;      // separator character
    bool m_bSkipSpace;  // skip space before a delimiter

protected:
    virtual void ProtectedReadStr(TStringBuffer& sDest);
    virtual void PostRead();

public:
    TStringTokenizer(LPCTSTR sPtr, TCHAR chSep);

    // set the skip-space flag
    void SetSkipSpace(bool bSkipSpace);

    // get the skip-space flag
    bool GetSkipSpace()
    {
        return m_bSkipSpace;
    }

    virtual bool SkipIfEmpty();
};
}  // namespace AFLibIO
