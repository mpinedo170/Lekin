#include "StdAfx.h"

#include "AFLibStringTokenizerAlt.h"

#include "AFLibGlobal.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using namespace AFLibIO;

TStringTokenizerAlt::TStringTokenizerAlt(LPCTSTR sPtr, LPCTSTR sSep) :
    super(sPtr),
    m_sSep(sSep),
    m_cLastSep(0)
{
    m_bCommaSafe = m_sSep.Find(chrComma) < 0;
}

void TStringTokenizerAlt::PostRead()
{
    m_cLastSep = GetCurrent();
    if (IsEof()) return;

    if (m_sSep.Find(m_cLastSep) < 0) ThrowMessage(LocalAfxString(IDS_NO_SEPARATOR));
    ProtectedNext();
}

void TStringTokenizerAlt::ProtectedReadStr(TStringBuffer& sDest)
{
    ProtectedFindFirstDelim(m_sSep, sDest);
    PostRead();
}

bool TStringTokenizerAlt::SkipIfEmpty()
{
    if (IsEof()) return false;
    if (m_sSep.Find(GetCurrent()) < 0) return false;
    PostRead();
    return true;
}
