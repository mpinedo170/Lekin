#include "StdAfx.h"

#include "AFLibStringParser.h"

#include "AFLibStringBuffer.h"
#include "AFLibStrings.h"
#include "AFLibThreadSafeStatic.h"

using AFLibIO::TStringParser;

TStringParser::TStringParser(LPCTSTR sPtr) : super(sPtr, false)
{}

void TStringParser::SetEater(bool bOn)
{
    ASSERT(!bOn);
}

CString TStringParser::FindFirstDelim(LPCTSTR strDelim)
{
    SAFE_STATIC_SB(buffer);
    ProtectedFindFirstDelim(strDelim, buffer);
    return LPCTSTR(buffer);
}

CString TStringParser::FindFirstNot(LPCTSTR strColl)
{
    SAFE_STATIC_SB(buffer);
    ProtectedFindFirstNot(strColl, buffer);
    return LPCTSTR(buffer);
}

CString TStringParser::FindFirst(bool bSkip, AFLIB_NINE_STRINGS_LOCAL)
{
    LPCTSTR* pStr = &str1;
    int count = 0;

    for (; count < 9; ++count)
        if (AFLib::IsEmptyStr(pStr[count])) break;

    SAFE_STATIC_SB(buffer);
    ProtectedFindFirst(bSkip, count, pStr, buffer);
    return LPCTSTR(buffer);
}
