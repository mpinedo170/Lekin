#include "StdAfx.h"

#include "AFLibStringTokenizer.h"

#include "AFLibStrings.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;

TStringTokenizer::TStringTokenizer(LPCTSTR sPtr, TCHAR chSep) :
    super(sPtr),
    m_chSep(chSep),
    m_bSkipSpace(false)
{
    m_bCommaSafe = m_chSep != chrComma;
}

void TStringTokenizer::SetSkipSpace(bool bSkipSpace)
{
    if (bSkipSpace && m_chSep == chrSpace) ThrowMessage(LocalAfxString(IDS_CANT_SKIP_SPACE));
    m_bSkipSpace = bSkipSpace;
}

void TStringTokenizer::PostRead()
{
    if (m_bSkipSpace)
    {
        while (GetCurrent() == chrSpace) ProtectedNext();
    }
    if (!IsEof()) ProtectedTestSymbolHard(m_chSep);
}

void TStringTokenizer::ProtectedReadStr(TStringBuffer& sDest)
{
    ProtectedFindFirstCh(true, m_chSep, sDest);
}

bool TStringTokenizer::SkipIfEmpty()
{
    return ProtectedTestSymbol(m_chSep);
}
