#include "StdAfx.h"

#include "AFLibStringTokenizerText.h"

#include "AFLibGlobal.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using namespace AFLibIO;

TStringTokenizerText::TStringTokenizerText(LPCTSTR sPtr) : super(sPtr), m_sLastSep(2)
{
    m_bCommaSafe = true;
}

void TStringTokenizerText::PostRead()
{
    m_sLastSep.Empty();
    if (IsEof()) return;

    TCHAR ch = GetCurrent();
    if (strEol.Find(ch) < 0) ThrowMessage(LocalAfxString(IDS_NO_SEPARATOR));
    m_sLastSep += ch;
    ProtectedNext();
    if (IsEof()) return;

    TCHAR ch2 = GetCurrent();
    if (ch == ch2 || strEol.Find(ch2) < 0) return;
    m_sLastSep += ch2;
    ProtectedNext();
}

void TStringTokenizerText::ProtectedReadStr(TStringBuffer& sDest)
{
    ProtectedFindFirstDelim(strEol, sDest);
    PostRead();
}

bool TStringTokenizerText::SkipIfEmpty()
{
    if (IsEof()) return false;
    if (strEol.Find(GetCurrent()) < 0) return false;
    PostRead();
    return true;
}
