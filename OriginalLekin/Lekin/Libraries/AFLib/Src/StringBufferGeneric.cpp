#include "StdAfx.h"

#include "AFLibStringBufferGeneric.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibPointers.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibMath;
using namespace AFLibPrivate;
using namespace AFLibIO;

namespace {
const int nGrowDefault = 1 << 10;
const int nGrowMin = 1 << 4;
}  // namespace

TStringBufferGeneric::TStringBufferGeneric(int charShift)
{
    m_charShift = charShift;
    m_nGrow = m_nAlloc = nGrowDefault;
    m_nLength = 0;
    Allocate(NULL);
}

TStringBufferGeneric::TStringBufferGeneric(const TStringBufferGeneric& sb)
{
    PrivateCopy(sb);
}

TStringBufferGeneric::TStringBufferGeneric(int charShift, int nGrow)
{
    m_charShift = charShift;
    m_nGrow = m_nAlloc = max(nGrow, nGrowMin);
    m_nLength = 0;
    Allocate(NULL);
}

TStringBufferGeneric::~TStringBufferGeneric()
{
    delete[] m_pBuffer;
}

void TStringBufferGeneric::Append(LPCSTR str)
{
    PrivateAppend(str, 0, strlen(str));
}

void TStringBufferGeneric::Append(LPCWSTR str)
{
    PrivateAppend(str, 1, wcslen(str));
}

void TStringBufferGeneric::AppendBstr(BSTR str)
{
    PrivateAppend(str, 1, SysStringLen(str));
}

int TStringBufferGeneric::GetPosition(LPCVOID ss) const
{
    return ss == NULL ? -1 : (reinterpret_cast<LPCBYTE>(ss) - m_pBuffer) >> m_charShift;
}

void TStringBufferGeneric::ComputeAllocSize(int nLength)
{
    m_nAlloc = (nLength / m_nGrow + 1) * m_nGrow;
}

void TStringBufferGeneric::SetChar(int pos, WCHAR ch)
{
    ASSERT(pos >= 0 && pos < m_nAlloc);

    if (IsUnicode())
        *reinterpret_cast<LPWSTR>(GetData(pos)) = ch;
    else
        *reinterpret_cast<LPSTR>(GetData(pos)) = ch & 0xFF;
}

void TStringBufferGeneric::Allocate(LPCVOID str)
{
    m_pBuffer = new BYTE[m_nAlloc << m_charShift];
    if (m_pBuffer == NULL) ThrowMessage(LocalAfxString(IDS_OUT_OF_MEM));

    if (m_nLength > 0) memcpy(m_pBuffer, str, m_nLength << m_charShift);
    Terminate();
}

void TStringBufferGeneric::PrivateCopy(const TStringBufferGeneric& sb)
{
    m_charShift = sb.m_charShift;
    m_nLength = sb.m_nLength;
    m_nAlloc = sb.m_nAlloc;
    m_nGrow = sb.m_nGrow;
    Allocate(sb.m_pBuffer);
}

void TStringBufferGeneric::ProtectedCopy(const TStringBufferGeneric& sb)
{
    if (&sb == this) return;
    delete[] m_pBuffer;
    PrivateCopy(sb);
}

void TStringBufferGeneric::Reallocate(int incLen)
{
    int newLen = m_nLength + incLen;
    if (newLen < m_nAlloc) return;

    ComputeAllocSize(newLen);
    LPBYTE pBuffer = new BYTE[m_nAlloc << m_charShift];
    if (pBuffer == NULL) ThrowMessage(LocalAfxString(IDS_OUT_OF_MEM));

    memcpy(pBuffer, m_pBuffer, (m_nLength + 1) << m_charShift);
    delete[] m_pBuffer;
    m_pBuffer = pBuffer;
}

void TStringBufferGeneric::PrivateAppend(LPCVOID str, int charShift, int len)
{
    if (len <= 0) return;
    int len2 = len;

    if (IsUnicode() && charShift == 0)
    {  // this is Unicode, adding Ascii
        len2 = ::MultiByteToWideChar(_AtlGetConversionACP(), 0, reinterpret_cast<LPCSTR>(str), len, NULL, 0);

        Reallocate(len2);
        ::MultiByteToWideChar(_AtlGetConversionACP(), 0, reinterpret_cast<LPCSTR>(str), len,
            reinterpret_cast<LPWSTR>(GetData(m_nLength)), len2);
    }
    else if (!IsUnicode() && charShift == 1)
    {  // this is Ascii, adding Unicode
        len2 = ::WideCharToMultiByte(
            _AtlGetConversionACP(), 0, reinterpret_cast<LPCWSTR>(str), len, NULL, 0, NULL, NULL);

        Reallocate(len2);
        ::WideCharToMultiByte(_AtlGetConversionACP(), 0, reinterpret_cast<LPCWSTR>(str), len,
            reinterpret_cast<LPSTR>(GetData(m_nLength)), len2, NULL, NULL);
    }
    else
    {
        Reallocate(len2);
        memcpy(GetData(m_nLength), str, len << m_charShift);
    }

    m_nLength += len2;
    Terminate();
}

void TStringBufferGeneric::AppendUint(UINT64 a, int digits)
{
    ASSERT(digits >= 0);
    Reallocate(max(24, digits));

    UINT64 mul = 1;
    while (a / mul >= 10)
    {
        mul *= 10;
        --digits;
    }

    ProtectedFill('0', digits - 1);

    while (mul > 0)
    {
        UINT digit = UINT(a / mul);
        WCHAR ch = digit + '0';
        PutChar(ch);
        a %= mul;
        mul /= 10;
    }

    Terminate();
}

void TStringBufferGeneric::AppendInt(INT64 a, int digits)
{
    Reallocate(1);

    if (a < 0)
    {
        PutChar(chrDashW);
        a = -a;
    }

    AppendUint(UINT64(a), digits);
}

void TStringBufferGeneric::AppendHex(UINT64 a, int digits)
{
    Reallocate(digits);

    for (int nDig = digits; --nDig >= 0;)
    {
        UINT d = UINT((a >> (nDig * 4)) & 0xF);
        WCHAR ch = d < 10 ? d + '0' : d - 10 + 'A';
        PutChar(ch);
    }

    Terminate();
}

void TStringBufferGeneric::PrivateAppendDouble(
    double d, int precision, bool bForceSign, bool bStrict, bool bPercent)
{
    precision = max(precision, 0);

    double eps = pow(10., -precision);
    WCHAR sg = chrPlusW;
    int mainDigits = 1;

    d = (floor(d / eps + 0.5)) * eps;

    if (IsNan(d))
    {
        Reallocate(1);
        PutChar(chrQuestionW);
        Terminate();
        return;
    }

    // positive, negative, or zero?
    eps *= 0.5;

    if (d < -eps)
    {
        sg = chrDashW;
        d = -d;
    }
    else if (d < eps)
    {
        sg = chrSpaceW;
        d = 0;
    }

    if (sg != chrSpaceW) d += eps * 0.2;

    // number of digits before decimal?
    if (d >= 10) mainDigits = int(floor(log10(d))) + 1;

    // take into account sign, decimal dot, and percent
    Reallocate(mainDigits + precision + 3);

    if (bForceSign || sg == chrDashW) PutChar(sg);

    double mul = pow(10., mainDigits - 1);

    for (int i = mainDigits; --i >= 0;)
    {
        int digit = min(int(d / mul), 9);
        PutChar(digit + '0');
        d = max(d - digit * mul, 0.);
        mul *= 0.1;
    }

    if (precision > 0 && (bStrict || d > eps))
    {
        PutChar(chrDotW);
        mul = 0.1;

        for (int i = 0; i < precision; ++i)
        {
            int digit = min(int(floor(d / mul) + 0.1), 9);
            PutChar(digit + '0');
            d = max(d - digit * mul, 0.);
            mul *= 0.1;

            if (!bStrict && d < eps) break;
        }
    }

    if (bPercent) PutChar(chrPercentW);
    Terminate();
}

void TStringBufferGeneric::ProtectedFill(int ch, int count)
{
    if (count <= 0) return;

    Reallocate(count);

    if (IsUnicode())
        wmemset(LPWSTR(GetData(m_nLength)), ch, count);
    else
        memset(GetData(m_nLength), ch, count);

    m_nLength += count;
    Terminate();
}

void TStringBufferGeneric::AppendVariant(const COleVariant& var)
{
    switch (var.vt)
    {
        case VT_BSTR:
            AppendBstr(V_BSTR(&var));
            break;

        case VT_BSTR | VT_BYREF:
            AppendBstr(*V_BSTRREF(&var));
            break;

        default:
        {
            COleVariant var2;
            var2.ChangeType(VT_BSTR, const_cast<COleVariant*>(&var));

            ASSERT(var2.vt == VT_BSTR);
            AppendBstr(V_BSTR(&var2));
            break;
        }
    }
}

void TStringBufferGeneric::SetGrowBy(int growBy)
{
    LPBYTE pBuffer = m_pBuffer;

    m_nGrow = max(growBy, nGrowMin);
    ComputeAllocSize(m_nLength);
    Allocate(pBuffer);
    delete[] pBuffer;
}

void TStringBufferGeneric::Clear()
{
    if (m_nAlloc > m_nGrow)
    {
        delete[] m_pBuffer;
        m_nAlloc = m_nGrow;
        m_nLength = 0;
        Allocate(NULL);
    }
    else
        Empty();
}

void TStringBufferGeneric::Empty()
{
    m_nLength = 0;
    Terminate();
}

void TStringBufferGeneric::Delete(int pos, int count)
{
    ASSERT(pos >= 0);
    if (count <= 0 || pos >= m_nLength) return;

    count = min(count, m_nLength - pos);
    memmove(GetData(pos), GetData(pos + count), (m_nLength - pos - count) << m_charShift);
    m_nLength -= count;
    Terminate();
}

int TStringBufferGeneric::ProtectedFind(int ch, int pos) const
{
    ASSERT(pos >= 0);
    if (pos >= m_nLength) return -1;

    LPCVOID ss = IsUnicode() ? wmemchr(reinterpret_cast<const WCHAR*>(GetData(pos)), ch, m_nLength)
                             : memchr(GetData(pos), ch, m_nLength);

    return GetPosition(ss);
}

int TStringBufferGeneric::ProtectedFind(LPCVOID str, int pos) const
{
    if (pos >= m_nLength) return -1;

    LPCVOID ss = NULL;

    if (IsUnicode())
        ss = wcsstr(reinterpret_cast<LPCWSTR>(GetData(pos)), reinterpret_cast<LPCWSTR>(str));
    else
        ss = strstr(reinterpret_cast<LPCSTR>(GetData(pos)), reinterpret_cast<LPCSTR>(str));

    return GetPosition(ss);
}

int TStringBufferGeneric::ProtectedFindOneOf(LPCVOID str, int pos) const
{
    if (pos >= m_nLength) return -1;

    LPCVOID ss = NULL;

    if (IsUnicode())
        ss = wcspbrk(reinterpret_cast<LPCWSTR>(GetData(pos)), reinterpret_cast<LPCWSTR>(str));
    else
        ss = strpbrk(reinterpret_cast<LPCSTR>(GetData(pos)), reinterpret_cast<LPCSTR>(str));

    return GetPosition(ss);
}

int TStringBufferGeneric::ProtectedCompare(LPCVOID str) const
{
    return IsUnicode() ? wcscmp(reinterpret_cast<LPCWSTR>(m_pBuffer), reinterpret_cast<LPCWSTR>(str))
                       : strcmp(reinterpret_cast<LPCSTR>(m_pBuffer), reinterpret_cast<LPCSTR>(str));
}

int TStringBufferGeneric::Read(CFile& Fin, int length, TTypeTextFile ttf)
{
    int lenRead = 0;
    Reallocate(length);

    lenRead = IsUnicode() ? FileRead(Fin, reinterpret_cast<LPWSTR>(GetData(m_nLength)), length, ttf)
                          : FileRead(Fin, reinterpret_cast<LPSTR>(GetData(m_nLength)), length, ttf);

    m_nLength += lenRead;
    Terminate();
    return lenRead;
}

void TStringBufferGeneric::Write(CFile& Fout, TTypeTextFile ttf) const
{
    if (IsEmpty()) return;

    if (IsUnicode())
        FileWrite(Fout, reinterpret_cast<LPCWSTR>(m_pBuffer), ttf);
    else
        FileWrite(Fout, reinterpret_cast<LPCSTR>(m_pBuffer), ttf);
}

void TStringBufferGeneric::WriteAndEmpty(CFile& Fout, TTypeTextFile ttf)
{
    if (m_nLength < m_nGrow / 2) return;
    Write(Fout, ttf);
    Empty();
}
