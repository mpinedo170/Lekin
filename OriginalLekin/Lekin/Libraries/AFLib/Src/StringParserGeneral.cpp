#include "StdAfx.h"

#include "AFLibStringParserGeneral.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibIO::TStringParserGeneral;

TStringParserGeneral::TStringParserGeneral(LPCTSTR sPtr, bool bEaterOn) : super(sPtr), m_bEaterOn(bEaterOn)
{
    ES();
}

void TStringParserGeneral::ES()
{
    if (m_bEaterOn) EatSpace();
}

void TStringParserGeneral::PostRead()
{
    ES();
}

void TStringParserGeneral::SetEater(bool bOn)
{
    m_bEaterOn = bOn;
    ES();
}

bool TStringParserGeneral::TestSymbolNoMove(LPCTSTR str)
{
    return ProtectedTestSymbolNoMove(str);
}

bool TStringParserGeneral::TestSymbol(LPCTSTR str)
{
    if (!ProtectedTestSymbol(str)) return false;
    ES();
    return true;
}

void TStringParserGeneral::TestSymbolHard(LPCTSTR str)
{
    ProtectedTestSymbolHard(str);
    ES();
}

int TStringParserGeneral::TestSymbolArray(const LPCTSTR arrStr[])
{
    for (int i = 0; arrStr[i] != NULL; ++i)
        if (TestSymbol(arrStr[i])) return i;
    return -1;
}

int TStringParserGeneral::TestSymbolArray(const CStringArray& arrStr)
{
    for (int i = 0; i < arrStr.GetSize(); ++i)
        if (TestSymbol(arrStr[i])) return i;
    return -1;
}

int TStringParserGeneral::TestSymbolArrayHard(const LPCTSTR arrStr[])
{
    int key = TestSymbolArray(arrStr);
    if (key >= 0) return key;

    TStringBuffer buffer;
    for (int i = 0; arrStr[i] != NULL; ++i)
    {
        if (!buffer.IsEmpty()) buffer += chrSpace;
        buffer += QuoteString(arrStr[i]);
    }
    ThrowMessage(LocalAfxString(IDS_EXPECTED_ONE_OF, buffer));
    return -1;
}

int TStringParserGeneral::TestSymbolArrayHard(const CStringArray& arrStr)
{
    int key = TestSymbolArray(arrStr);
    if (key >= 0) return key;

    TStringBuffer buffer;
    for (int i = 0; i < arrStr.GetSize(); ++i)
    {
        if (!buffer.IsEmpty()) buffer += chrSpace;
        buffer += QuoteString(arrStr[i]);
    }
    ThrowMessage(LocalAfxString(IDS_EXPECTED_ONE_OF, buffer));
    return -1;
}

void TStringParserGeneral::SetPos(int pos)
{
    super::SetPos(pos);
    ES();
}

void TStringParserGeneral::Skip(int n)
{
    SetPos(GetPos() + n);
}

void TStringParserGeneral::Next()
{
    ProtectedNext();
    ES();
}

void TStringParserGeneral::EatSpace()
{
    SAFE_STATIC_SB(buffer);
    ProtectedFindFirstNot(strWhitespace, buffer);
}

double TStringParserGeneral::NoCommasDouble()
{
    double d = ProtectedReadDouble(true);
    PostRead();
    return d;
}

CString TStringParserGeneral::ReadToSpace()
{
    SAFE_STATIC_SB(buffer);
    ProtectedFindFirstDelim(strWhitespace, buffer);
    ES();
    return LPCTSTR(buffer);
}

CString TStringParserGeneral::FindFirst(LPCTSTR strWhat)
{
    SAFE_STATIC_SB(buffer);
    ProtectedFindFirst(true, 1, &strWhat, buffer);
    ES();
    return LPCTSTR(buffer);
}

CString TStringParserGeneral::ReadExact(int count)
{
    SAFE_STATIC_SB(buffer);
    ProtectedReadSegment(count, buffer);
    ES();
    return LPCTSTR(buffer);
}

//***************** TEaterManipulator *****************

TStringParserGeneral::TEaterManipulator::TEaterManipulator(TStringParserGeneral& host, bool bEaterOn) :
    m_host(host)
{
    m_bEaterOnSaved = m_host.GetEater();
    m_host.SetEater(bEaterOn);
}

TStringParserGeneral::TEaterManipulator::~TEaterManipulator()
{
    m_host.SetEater(m_bEaterOnSaved);
}
