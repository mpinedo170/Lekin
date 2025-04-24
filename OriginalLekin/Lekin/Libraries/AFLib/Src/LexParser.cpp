#include "StdAfx.h"

#include "AFLibLexParser.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStrings.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using AFLibCalc::TLexParser;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;

TLexParser::TLexParser(LPCTSTR sPtr) : super(sPtr)
{
    m_iLine = 1;
    m_sLineStart = sPtr;
    InitNextLineStart();
    RollLineCount();
}

void TLexParser::InitNextLineStart()
{
    m_sNextLineStart = NULL;
    if (m_sLineStart == NULL) return;

    LPCTSTR ss = _tcspbrk(m_sLineStart, strEol);
    if (ss == NULL) return;

    m_sNextLineStart = ss + 1;
    if (strEol.Find(*m_sNextLineStart) && *m_sNextLineStart != *ss) ++m_sNextLineStart;
}

void TLexParser::RollLineCount()
{
    while (true)
    {
        if (m_sNextLineStart == NULL) break;
        if (!IsEof() && m_sNextLineStart > ProtectedGetPtr()) break;
        ++m_iLine;
        m_sLineStart = m_sNextLineStart;
        InitNextLineStart();
    }
}

void TLexParser::EatSpace()
{
    super::EatSpace();
    RollLineCount();
}

bool TLexParser::LexReadID(TStringBuffer& str)
{
    str.Empty();

    TCHAR ch = GetCurrent();
    if (!IsAlphaU(ch)) return false;

    while (true)
    {
        ch = GetCurrent();
        if (!IsAlnumU(ch)) break;
        str += ch;
        ProtectedNext();
    }

    PostRead();
    return true;
}

bool TLexParser::LexReadNumber(TStringBuffer& str, double& number)
{
    int n = 0;

    str.Empty();
    number = 0;
    if (IsEof() || _stscanf_s(ProtectedGetPtr(), _T("%lf%n"), &number, &n) < 1) return false;

    TCHAR ch = ProtectedGetPtr()[n];
    if (IsAlnumU(ch)) return false;

    ProtectedReadSegment(n, str);
    PostRead();
    return true;
}

void TLexParser::SetPos(int pos)
{
    if (pos < GetPos())
    {
        m_iLine = 1;
        m_sLineStart = GetInitPtr();
    }

    super::SetPos(pos);
    RollLineCount();
}

void TLexParser::SkipToEol()
{
    SetPos(m_sNextLineStart == NULL ? GetLength() : m_sNextLineStart - GetInitPtr());
}

void TLexParser::SkipTo(LPCTSTR strEnd)
{
    SAFE_STATIC_SB(buffer);

    ProtectedFindFirst(false, 1, &strEnd, buffer);
    if (IsEof()) ThrowMessage(LocalAfxString(IDS_EOF_IN_COMMENT));
    Skip(_tcslen(strEnd));
}
