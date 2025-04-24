#include "StdAfx.h"

#include "AFLibStringTokenizerStr.h"

#include "AFLibStrings.h"
#include "AFLibThreadSafeStatic.h"

using namespace AFLib;
using namespace AFLibIO;

TStringTokenizerStr::TStringTokenizerStr(LPCTSTR sPtr, LPCTSTR sSep) : super(sPtr), m_sSep(sSep)
{
    m_bCommaSafe = m_sSep.Find(chrComma) < 0;
}

void TStringTokenizerStr::PostRead()
{
    if (IsEof()) return;
    ProtectedTestSymbolHard(m_sSep);
}

void TStringTokenizerStr::ProtectedReadStr(TStringBuffer& sDest)
{
    LPCTSTR sSep = m_sSep;
    ProtectedFindFirst(true, 1, &sSep, sDest);
}

bool TStringTokenizerStr::SkipIfEmpty()
{
    return ProtectedTestSymbol(m_sSep);
}
