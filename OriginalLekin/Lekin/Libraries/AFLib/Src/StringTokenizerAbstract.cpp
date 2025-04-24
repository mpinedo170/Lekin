#include "StdAfx.h"

#include "AFLibStringTokenizerAbstract.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeStatic.h"

using namespace AFLib;
using namespace AFLibIO;

TStringTokenizerAbstract::TStringTokenizerAbstract(LPCTSTR sPtr) : super(sPtr)
{
    m_bCommaSafe = false;
}

CString TStringTokenizerAbstract::ReadStr()
{
    SAFE_STATIC_SB(buffer);
    ProtectedReadStr(buffer);
    return LPCTSTR(buffer);
}

void TStringTokenizerAbstract::SkipStr()
{
    SAFE_STATIC_SB(buffer);
    ProtectedReadStr(buffer);
}

void TStringTokenizerAbstract::ProtectedReadExcelStr(TStringBuffer& sDest)
{
    if (ProtectedTestSymbolNoMove(chrDQuote))
    {
        ProtectedReadQuotedStr(sDest);
        PostRead();
    }
    else
        ProtectedReadStr(sDest);
}

CString TStringTokenizerAbstract::ReadExcelStr()
{
    return ProtectedTestSymbolNoMove(chrDQuote) ? ReadQuotedStr() : ReadStr();
}

double TStringTokenizerAbstract::NoCommasDouble()
{
    double d = 0;

    if (ProtectedTestSymbol(chrDQuote))
    {
        d = ProtectedReadDouble(true);
        ProtectedTestSymbolHard(chrDQuote);
    }
    else
        d = ProtectedReadDouble(m_bCommaSafe);

    PostRead();
    return d;
}
