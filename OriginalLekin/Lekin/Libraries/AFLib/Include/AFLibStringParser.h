#pragma once

#include "AFLibStringParserGeneral.h"
#include "AFLibStrings.h"

/////////////////////////////////////////////////////////////////////////////
// TStringParser
// Parses string.  Does not skip whitespace automatically.

namespace AFLibIO {
class AFLIB TStringParser : public TStringParserGeneral
{
private:
    typedef TStringParserGeneral super;
    DEFINE_COPY_AND_ASSIGN(TStringParser);

public:
    explicit TStringParser(LPCTSTR sPtr);

    // throws an ecxeption on an attempt to turn space eater on
    virtual void SetEater(bool bOn);

    // read until a char from strDelim
    CString FindFirstDelim(LPCTSTR strDelim);

    // find first symbol NOT from strColl
    CString FindFirstNot(LPCTSTR strColl);

    // find the first one of several substrings
    CString FindFirst(bool bSkip, AFLIB_NINE_STRINGS);
};
}  // namespace AFLibIO
