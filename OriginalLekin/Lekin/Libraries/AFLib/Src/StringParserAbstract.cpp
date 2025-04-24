#include "StdAfx.h"

#include "AFLibStringParserAbstract.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using namespace AFLibIO;
using namespace AFLibMath;

TStringParserAbstract::TStringParserAbstract(LPCTSTR sPtr)
{
    m_sPtr = m_sInitial = sPtr;
    m_length = _tcslen(sPtr);
    m_bIgnoreCase = false;
    TestEof();
}

TStringParserAbstract::~TStringParserAbstract()
{}

void TStringParserAbstract::TestEof()
{
    if (!IsEof() && *m_sPtr == 0) ProtectedSetEof();
}

void TStringParserAbstract::BuildCaseInsensitive(LPCTSTR sSrc, TStringBuffer& sDest)
{
    for (; *sSrc; ++sSrc)
    {
        if (sDest.Find(ToLower(*sSrc)) < 0) sDest += TCHAR(ToLower(*sSrc));
        if (sDest.Find(ToUpper(*sSrc)) < 0) sDest += TCHAR(ToUpper(*sSrc));
    }
}

void TStringParserAbstract::ProtectedReadSegment(int count, TStringBuffer& sDest)
{
    sDest.Empty();
    if (IsEof()) return;

    ProtectedReadSegment(GetPos() + count >= m_length ? NULL : m_sPtr + count, 0, sDest);
}

void TStringParserAbstract::ProtectedReadSegment(LPCTSTR ssEnd, int nSkip, TStringBuffer& sDest)
{
    sDest.Empty();

    if (ssEnd == NULL)
    {
        sDest += m_sPtr;
        ProtectedSetEof();
        return;
    }

    sDest.Append(m_sPtr, ssEnd - m_sPtr);
    m_sPtr = ssEnd + nSkip;
    TestEof();
}

void TStringParserAbstract::ProtectedFindFirstCh(bool bSkip, TCHAR ch, TStringBuffer& sDest)
{
    sDest.Empty();
    if (IsEof()) return;

    LPCTSTR ss = _tcschr(m_sPtr, ch);
    ProtectedReadSegment(ss, bSkip ? 1 : 0, sDest);
}

void TStringParserAbstract::ProtectedFindFirst(
    bool bSkip, int count, const LPCTSTR arrStr[], TStringBuffer& sDest)
{
    sDest.Empty();
    if (IsEof()) return;

    LPCTSTR ss = NULL;
    int len = 0;

    if (m_bIgnoreCase)
    {
        for (ss = m_sPtr; *ss; ++ss)
            for (int i = 0; i < count; ++i)
                if (StringStartsWithIC(ss, arrStr[i]))
                {
                    len = _tcslen(arrStr[i]);
                    goto Ret;
                }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            LPCTSTR ss2 = _tcsstr(m_sPtr, arrStr[i]);
            if (ss2 != NULL && (ss == NULL || ss2 < ss))
            {
                ss = ss2;
                len = _tcslen(arrStr[i]);
            }
        }
    }
Ret:
    ProtectedReadSegment(ss, bSkip ? len : 0, sDest);
}

void TStringParserAbstract::ProtectedFindFirstDelim(LPCTSTR strDelim, TStringBuffer& sDest)
{
    SAFE_STATIC_SB(strDelimCI);

    sDest.Empty();
    if (IsEof()) return;

    if (m_bIgnoreCase)
    {
        BuildCaseInsensitive(strDelim, strDelimCI);
        strDelim = strDelimCI;
    }

    LPCTSTR ss = _tcspbrk(m_sPtr, strDelim);
    ProtectedReadSegment(ss, 0, sDest);
}

void TStringParserAbstract::ProtectedFindFirstNot(LPCTSTR strColl, TStringBuffer& sDest)
{
    SAFE_STATIC_SB(strCollCI);

    sDest.Empty();
    if (IsEof()) return;

    if (m_bIgnoreCase)
    {
        BuildCaseInsensitive(strColl, strCollCI);
        strColl = strCollCI;
    }

    int n = _tcsspn(m_sPtr, strColl);
    if (n > 0) ProtectedReadSegment(m_sPtr + n, 0, sDest);
}

bool TStringParserAbstract::ProtectedTestSymbolNoMove(LPCTSTR str)
{
    return IsEof() ? false : LocalStringStartsWith(m_sPtr, str, m_bIgnoreCase);
}

bool TStringParserAbstract::ProtectedTestSymbol(LPCTSTR str)
{
    if (!ProtectedTestSymbolNoMove(str)) return false;
    m_sPtr += _tcslen(str);
    TestEof();
    return true;
}

void TStringParserAbstract::ProtectedTestSymbolHard(LPCTSTR str)
{
    if (ProtectedTestSymbol(str)) return;

    CString s;
    if (_tcschr(str, chrNewLine) != NULL)
        LocalThrowMessageExpected(IDS_EOL);
    else if (_tcscmp(str, strTab) == 0)
        LocalThrowMessageExpected(IDS_TAB);
    else if (_tcscmp(str, strDQuote) == 0)
        LocalThrowMessageExpected(IDS_QUOTE);
    else
        ThrowMessageExpected(QuoteString(str));
}

bool TStringParserAbstract::ProtectedTestSymbol(TCHAR ch)
{
    if (GetCurrent() != ch) return false;
    ProtectedNext();
    return true;
}

bool TStringParserAbstract::ProtectedTestSymbolNoMove(TCHAR ch)
{
    return GetCurrent() == ch;
}

void TStringParserAbstract::ProtectedTestSymbolHard(TCHAR ch)
{
    if (GetCurrent() == ch)
    {
        ProtectedNext();
        return;
    }

    // call the LPCTSTR version to throw the exception
    TCHAR str[2] = {ch, 0};
    ProtectedTestSymbolHard(str);
}

void TStringParserAbstract::ProtectedNext()
{
    if (IsEof()) return;
    ++m_sPtr;
    TestEof();
}

void TStringParserAbstract::ProtectedSetPos(int pos)
{
    m_sPtr = pos < 0 || pos >= m_length ? NULL : m_sInitial + pos;
}

void TStringParserAbstract::TestEofHard()
{
    if (!IsEof()) ThrowMessage(LocalAfxString(IDS_EXTRA_CHAR));
}

void TStringParserAbstract::SetIgnoreCase(bool bIgnoreCase)
{
    m_bIgnoreCase = bIgnoreCase;
}

void TStringParserAbstract::SetPos(int pos)
{
    ProtectedSetPos(pos);
}

CString TStringParserAbstract::ReadFinal()
{
    if (IsEof()) return strEmpty;

    SAFE_STATIC_SB(buffer);
    ProtectedReadSegment(NULL, 0, buffer);
    return LPCTSTR(buffer);
}

void TStringParserAbstract::ProtectedReadQuotedStr(TStringBuffer& sDest)
{
    sDest.Empty();
    if (IsEof()) return;

    ProtectedTestSymbolHard(chrDQuote);
    while (true)
    {
        TCHAR ch = GetCurrent();
        if (ch == 0)
        {
            ProtectedSetEof();
            ThrowMessage(LocalAfxString(IDS_NO_QUOTE));
        }

        ProtectedNext();

        if (ch == chrDQuote)
        {
            ch = GetCurrent();
            if (ch != chrDQuote) break;
            ProtectedNext();
        }

        sDest += ch;
    }
}

CString TStringParserAbstract::ReadQuotedStr()
{
    SAFE_STATIC_SB(buffer);
    ProtectedReadQuotedStr(buffer);
    PostRead();
    return LPCTSTR(buffer);
}

bool TStringParserAbstract::ProtectedReadNegative()
{
    bool bNeg = false;

    switch (GetCurrent())
    {
        case chrDash:
            bNeg = true;
        case chrPlus:
        case chrSpace:
            ProtectedNext();
    }

    return bNeg;
}

INT64 TStringParserAbstract::ProtectedReadInt64()
{
    bool bNeg = ProtectedReadNegative();
    INT64 a = PrivateReadUint(IDS_INT, 10, bNeg ? UINT64(_I64_MAX) + 1 : UINT64(_I64_MAX));
    return bNeg ? -a : a;
}

UINT64 TStringParserAbstract::ProtectedReadUint64()
{
    return PrivateReadUint(IDS_UINT64, 10, _UI64_MAX);
}

UINT64 TStringParserAbstract::ProtectedReadHex64()
{
    return PrivateReadUint(IDS_HEX64, 16, _UI64_MAX);
}

double TStringParserAbstract::ProtectedReadDouble(bool bAllowCommas)
{
    bool bOk = false;
    double d = 0;
    bool bNeg = ProtectedReadNegative();

    for (;; ProtectedNext())
    {
        TCHAR ch = GetCurrent();

        if (bAllowCommas && ch == chrComma) continue;
        if (!IsDigit(ch)) break;

        bOk = true;
        d *= 10;
        d += ch - _T('0');
    }

    if (ProtectedTestSymbol(chrDot))
    {
        double mul = 1;

        for (;; ProtectedNext())
        {
            TCHAR ch = GetCurrent();
            if (!IsDigit(ch)) break;

            bOk = true;
            mul *= 0.1;
            d += (ch - _T('0')) * mul;
        }
    }

    if (!bOk) LocalThrowMessageExpected(IDS_NUM);
    if (bNeg) d = -d;

    if (ToUpper(GetCurrent()) != _T('E')) return d;

    LPCTSTR sPtr = m_sPtr;

    try
    {
        ProtectedNext();
        INT64 e = ProtectedReadInt64();
        double de = d * pow(10., double(e));
        if (!IsNan(de)) return de;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    // wrong exponent; return the number without the exponent
    m_sPtr = sPtr;
    return d;
}

double TStringParserAbstract::ReadDouble()
{
    double d = ProtectedReadDouble(false);
    PostRead();
    return d;
}

double TStringParserAbstract::ReadDoubleNan()
{
    if (!ProtectedTestSymbol(chrQuestion)) return ReadDouble();

    PostRead();
    return Nan;
}

double TStringParserAbstract::ReadPercentage(bool bForcePrecent)
{
    double d = ReadDouble();
    bool bPercent = ProtectedTestSymbol(chrPercent);

    if (bPercent)
        PostRead();
    else if (bForcePrecent)
        ThrowMessageExpected(_T("\"%\""));

    return d / Const100;
}

double TStringParserAbstract::ReadPercentageNan(bool bForcePrecent)
{
    if (!ProtectedTestSymbol(chrQuestion)) return ReadPercentage(bForcePrecent);

    PostRead();
    return Nan;
}

namespace {
void ThrowTooLarge(UINT idsType)
{
    CString s;
    s.LoadString(AFLibDLL.hModule, idsType);
    ThrowMessage(LocalAfxString(IDS_NUM_TOO_LARGE, s));
}
}  // namespace

UINT64 TStringParserAbstract::PrivateReadUint(UINT idsType, int mul, UINT64 iMax)
{
    bool bOk = false;
    UINT64 a = 0;
    UINT64 maxBeforeMul = iMax / mul;

    while (true)
    {
        int n = -1;
        TCHAR ch = GetCurrent();

        if (IsDigit(ch))
            n = ch - _T('0');
        else if (IsUpper(ch))
            n = ch - 'A' + 10;
        else if (IsLower(ch))
            n = ch - 'a' + 10;
        else
            break;

        // check that the digit is valid
        if (n >= mul) break;

        if (a > maxBeforeMul) ThrowTooLarge(idsType);
        a *= mul;

        if (a > iMax - n) ThrowTooLarge(idsType);
        a += n;
        bOk = true;
        ProtectedNext();
    }

    if (!bOk) LocalThrowMessageExpected(idsType);

    return a;
}

int TStringParserAbstract::ReadInt()
{
    bool bNeg = ProtectedReadNegative();
    int a = int(PrivateReadUint(IDS_INT, 10, bNeg ? -INT64(_I32_MIN) : INT64(_I32_MAX)));

    PostRead();
    return bNeg ? -a : a;
}

UINT TStringParserAbstract::ReadUint()
{
    UINT a = UINT(PrivateReadUint(IDS_UINT, 10, _UI32_MAX));
    PostRead();
    return a;
}

UINT TStringParserAbstract::ReadHex()
{
    UINT a = UINT(PrivateReadUint(IDS_HEX, 16, _UI32_MAX));
    PostRead();
    return a;
}

INT64 TStringParserAbstract::ReadInt64()
{
    bool bNeg = ProtectedReadNegative();
    INT64 a = PrivateReadUint(IDS_INT, 10, bNeg ? UINT64(_I64_MAX) + 1 : UINT64(_I64_MAX));

    PostRead();
    return bNeg ? -a : a;
}

UINT64 TStringParserAbstract::ReadUint64()
{
    UINT64 a = ProtectedReadUint64();
    PostRead();
    return a;
}

UINT64 TStringParserAbstract::ReadHex64()
{
    UINT64 a = ProtectedReadHex64();
    PostRead();
    return a;
}

bool TStringParserAbstract::ReadPM()
{
    bool bPlus = false;

    switch (GetCurrent())
    {
        case chrPlus:
            bPlus = true;
        case chrDash:
            break;
        default:
            LocalThrowMessageExpected(IDS_PM);
    }

    ProtectedNext();
    PostRead();
    return bPlus;
}

CString TStringParserAbstract::ReadAlnum()
{
    if (!IsAlnum(GetCurrent())) LocalThrowMessageExpected(IDS_ALNUM);

    SAFE_STATIC_SB(buffer);

    while (true)
    {
        TCHAR ch = GetCurrent();
        if (!IsAlnum(ch)) break;
        buffer += ch;
        ProtectedNext();
    }

    PostRead();
    return LPCTSTR(buffer);
}

//***************** TCaseManipulator *****************

TStringParserAbstract::TCaseManipulator::TCaseManipulator(TStringParserAbstract& host, bool bIgnoreCase) :
    m_host(host)
{
    m_bIgnoreCaseSaved = m_host.GetIgnoreCase();
    m_host.SetIgnoreCase(bIgnoreCase);
}

TStringParserAbstract::TCaseManipulator::~TCaseManipulator()
{
    m_host.SetIgnoreCase(m_bIgnoreCaseSaved);
}
