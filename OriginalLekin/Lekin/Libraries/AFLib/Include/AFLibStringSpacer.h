#pragma once

#include "AFLibConcatHandler.h"
#include "AFLibStringParserGeneral.h"

/////////////////////////////////////////////////////////////////////////////
// TStringSpacer
// Parses string.  May or may not skip whitespace automatically.

namespace AFLibIO {
class AFLIB TStringSpacer : public TStringParserGeneral, public AFLibPrivate::TConcatHandler
{
private:
    typedef TStringParserGeneral super;
    DEFINE_COPY_AND_ASSIGN(TStringSpacer);

public:
    explicit TStringSpacer(LPCTSTR sPtr);

    // read a C string (possibly Unicode)
    CStringW ReadCStrW();

    // read a simple or C string
    CString ReadSafeStr();

// read a C string
#ifdef _UNICODE
    CString ReadCStr()
    {
        return ReadCStrW();
    }
#else
    CString ReadCStr();
#endif
};
}  // namespace AFLibIO
