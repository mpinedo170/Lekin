#include "StdAfx.h"

#include "AFLibGlobal.h"

#include "AFLib3State.h"
#include "AFLibExcBreak.h"
#include "AFLibExcMessage.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"
#include "AFLibStringTokenizerStr.h"
#include "AFLibStrings.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibMath;
using namespace AFLibThread;
using AFLibPrivate::LocalAfxString;
using AFLibThread::CExcBreak;

namespace AFLibMath {
double Round(double value, int digits)
{
    double coef = 1;

    if (digits != 0) coef = pow(10., digits);

    return floor(value * coef + 0.5) / coef;
}

int Rint(double value)
{
    return static_cast<int>(floor(value + 0.5));
}

bool IsNan(double a)
{
    return _finite(a) == 0;
}

bool IsNan(float a)
{
    return _finite(a) == 0;
}

double NormalDensity(double x)
{
    static const double p = 1 / sqrt(2 * M_PI);
    return p * exp(-x * x / 2);
}

double NormalDistribution(double x)
{
    static const double b1 = 0.319381530;
    static const double b2 = -0.356563782;
    static const double b3 = 1.781477937;
    static const double b4 = -1.821255978;
    static const double b5 = 1.330274429;
    static const double p = 0.2316419;
    static const double c = 0.39894228;

    double t = 1 / (1 + p * fabs(x));
    double n = c * exp(-x * x / 2) * t * (t * (t * (t * (t * b5 + b4) + b3) + b2) + b1);

    return x >= 0 ? 1 - n : n;
}
}  // namespace AFLibMath

namespace {
__declspec(thread) TStringBuffer* pSbToStr;

// helper for AllocBlock and AllocBlockFixed
HGLOBAL PrivateAllocBlock(LPCVOID pData, int length, bool bMoveable)
{
    HGLOBAL hMem = GlobalAlloc(bMoveable ? GMEM_MOVEABLE : GMEM_FIXED, length);
    LPVOID pTemp = GlobalLock(hMem);

    if (pData != NULL)
        memcpy(pTemp, pData, length);
    else
        memset(pTemp, 0, length);

    return hMem;
}
}  // namespace

namespace AFLib {
DWORD GetBitField(DWORD number, DWORD mask)
{
    if (mask == 0) return 0;
    DWORD mult = (mask ^ (mask << 1)) & mask;
    return (number & mask) / mult;
}

DWORD UpdateBitField(DWORD number, DWORD mask, DWORD field)
{
    if (mask == 0) return number;
    // extract the rightmost nonzero bit in mask
    DWORD mult = (mask ^ (mask << 1)) & mask;
    return (number & ~mask) | (field * mult);
}

double StrToDouble(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        double d = sp.ReadDoubleNan();
        sp.TestEofHard();
        return d;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return Nan;
}

CString DoubleToStr(double d, int precision, bool bForceSign)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendDouble(d, precision, bForceSign);
    return LPCTSTR(*pSbToStr);
}

CString DoubleToStrStrict(double d, int precision, bool bForceSign)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendDoubleStrict(d, precision, bForceSign);
    return LPCTSTR(*pSbToStr);
}

CString PercentToStr(double d, int precision, bool bForceSign)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendPercent(d, precision, bForceSign);
    return LPCTSTR(*pSbToStr);
}

CString PercentToStrStrict(double d, int precision, bool bForceSign)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendPercentStrict(d, precision, bForceSign);
    return LPCTSTR(*pSbToStr);
}

int StrToInt(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        int a = sp.ReadInt();
        sp.TestEofHard();
        return a;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return NanI;
}

INT64 StrToInt64(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        INT64 a = sp.ReadInt64();
        sp.TestEofHard();
        return a;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return NanI64;
}

CString IntToStr(INT64 a)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendInt(a);
    return LPCTSTR(*pSbToStr);
}

UINT StrToUint(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        UINT a = sp.ReadUint();
        sp.TestEofHard();
        return a;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return NanUI;
}

UINT64 StrToUint64(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        UINT64 a = sp.ReadUint64();
        sp.TestEofHard();
        return a;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return NanUI64;
}

CString UintToStr(UINT64 a)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendUint(a);
    return LPCTSTR(*pSbToStr);
}

UINT StrToHex(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        UINT a = sp.ReadHex();
        sp.TestEofHard();
        return a;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return NanUI;
}

UINT64 StrToHex64(LPCTSTR s)
{
    TStringSpacer sp(s);

    try
    {
        UINT64 a = sp.ReadHex64();
        sp.TestEofHard();
        return a;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return NanUI64;
}

CString HexToStr(UINT64 a, int digits)
{
    SAFE_STATIC_PTR(StaticStringBufferStorage, TStringBuffer, pSbToStr);
    pSbToStr->Empty();
    pSbToStr->AppendHex(a, digits);
    return LPCTSTR(*pSbToStr);
}

const CString& BoolToPM(bool b)
{
    return b ? strPlus : strDash;
}

COleDateTime StrToTime(LPCTSTR s, bool bStrict)
{
    struct tm atm = {0};
    int mon = 0;
    int year = 0;

    int nNum = _stscanf_s(
        s, _T("%d/%d/%d %d:%d:%d"), &mon, &atm.tm_mday, &year, &atm.tm_hour, &atm.tm_min, &atm.tm_sec);

    if (nNum < 3 || nNum == 4 || year < 0) ThrowMessage(LocalAfxString(IDS_BAD_DATE_FORMAT));

    if (year >= 0 && year < 50)
        atm.tm_year = year + 100;
    else if (year >= 50 && year < 99)
        atm.tm_year = year;
    else
        atm.tm_year = year - 1900;

    atm.tm_mon = mon - 1;
    atm.tm_isdst = -1;
    struct tm atm2 = atm;
    __time64_t t64 = _mktime64(&atm);

    if (t64 == -1) ThrowMessage(LocalAfxString(IDS_BAD_DATE_FORMAT));

    if (bStrict)
    {
        if (atm.tm_year != atm2.tm_year || atm.tm_mon != atm2.tm_mon || atm.tm_mday != atm2.tm_mday ||
            atm.tm_hour != atm2.tm_hour || atm.tm_min != atm2.tm_min || atm.tm_sec != atm2.tm_sec)
            ThrowMessage(LocalAfxString(IDS_BAD_DATE_FORMAT));
    }

    return COleDateTime(t64);
}

CString TimeToStr(const COleDateTime& odt)
{
    SAFE_CONST_STR(strTimeFormat, _T("%m/%d/%Y %H:%M:%S"));
    return odt == odtZero ? strEmpty : odt.Format(strTimeFormat);
}

int CountTokens(LPCTSTR line, LPCTSTR delimiter)
{
    TStringTokenizerStr sp(line, delimiter);
    int i = 0;
    while (!sp.IsEof())
    {
        sp.ReadStr();
        ++i;
    }
    return i;
}

CString GetTokenAt(LPCTSTR line, LPCTSTR delimiter, int n)
{
    TStringTokenizerStr sp(line, delimiter);
    CString s;
    while (--n >= -1)
    {
        if (sp.IsEof()) return strEmpty;
        s = sp.ReadStr();
    }
    return s;
}

bool IsKeyHotInString(LPCTSTR str, TCHAR ch)
{
    LPCTSTR ss = str;
    ch = ToUpper(ch);

    while (true)
    {
        ss = _tcschr(ss, _T('&'));
        if (ss == NULL) break;
        ++ss;
        if (*ss == 0) break;

        TCHAR ch2 = ToUpper(*ss);
        if (ch2 == _T('&'))
        {
            ++ss;
            continue;
        }

        if (ch2 == ch) return true;
    }

    return false;
}

BSTR AFSysAllocStringLen(LPCSTR str, int len)
{
    if (len < 0) len = strlen(str);

    int len2 = ::MultiByteToWideChar(_AtlGetConversionACP(), 0, str, len, NULL, 0);
    BSTR bstr = ::SysAllocStringLen(NULL, len2);

    if (bstr == NULL) ThrowMessage(LocalAfxString(IDS_OUT_OF_MEM));

    ::MultiByteToWideChar(_AtlGetConversionACP(), 0, str, len, bstr, len2);
    bstr[len2] = 0;
    return bstr;
}

BSTR AFSysAllocStringLen(LPCWSTR str, int len)
{
    if (len < 0)
    {
        ASSERT(str != NULL);
        len = wcslen(str);
    }

    BSTR bstr = ::SysAllocStringLen(str, len);
    if (bstr == NULL) ThrowMessage(LocalAfxString(IDS_OUT_OF_MEM));
    return bstr;
}

BSTR AFSysAllocStringBstr(BSTR src)
{
    return AFSysAllocStringLen(src, ::SysStringLen(src));
}

void AFSysReAllocStringLen(BSTR& bstr, LPCSTR str, int len)
{
    BSTR bstrOld = bstr;
    bstr = AFSysAllocStringLen(str, len);
    if (bstrOld != NULL) ::SysFreeString(bstrOld);
}

void AFSysReAllocStringLen(BSTR& bstr, LPCWSTR str, int len)
{
    BSTR bstrOld = bstr;
    bstr = AFSysAllocStringLen(str, len);
    if (bstrOld != NULL) ::SysFreeString(bstrOld);
}

void AFSysReAllocStringBstr(BSTR& bstr, BSTR src)
{
    AFSysReAllocStringLen(bstr, src, ::SysStringLen(src));
}

HGLOBAL AllocBlock(LPCVOID pData, int length)
{
    return PrivateAllocBlock(pData, length, false);
}

HGLOBAL AllocBlockFixed(LPCVOID pData, int length)
{
    return PrivateAllocBlock(pData, length, true);
}

HGLOBAL AllocString(LPCSTR s, bool bUnicode)
{
    int length = strlen(s) + 1;

    if (bUnicode)
    {
        CStringW s2(s);
        return AllocBlock(LPCWSTR(s2), length * 2);
    }
    else
        return AllocBlock(s, length);
}

HGLOBAL AllocString(LPCWSTR s, bool bUnicode)
{
    int length = wcslen(s) + 1;

    if (bUnicode)
        return AllocBlock(s, length * 2);
    else
    {
        CStringA s2(s);
        return AllocBlock(s2, length);
    }
}

INT64 GetFreeVirtualMem()
{
    MEMORYSTATUSEX statex = {sizeof(MEMORYSTATUSEX)};

    GlobalMemoryStatusEx(&statex);
    return statex.ullAvailVirtual;
}

INT64 GetTotalHeapMem(HANDLE hHeap, bool bUsedOnly)
{
    INT64 total = 0;
    PROCESS_HEAP_ENTRY phe = {0};
    HeapLock(hHeap);

    while (HeapWalk(hHeap, &phe))
        if (!bUsedOnly || (phe.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) total += phe.cbOverhead + phe.cbData;

    int err = GetLastError();

    HeapUnlock(hHeap);
    if (err != ERROR_NO_MORE_ITEMS) ThrowMessage(LocalAfxString(IDS_ERROR_IN_HEAP, IntToStr(err)));
    return total;
}

bool EnableLowFragHeap()
{
    ULONG HeapFragValue = 2;
    bool bResult = HeapSetInformation(HANDLE(_get_heap_handle()), HeapCompatibilityInformation,
                       &HeapFragValue, sizeof(HeapFragValue)) != 0;

    bResult |= HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &HeapFragValue,
                   sizeof(HeapFragValue)) != 0;
    return bResult;
}
}  // namespace AFLib

namespace {
template <class TString, class TPtr>
void PrivateReadClipboardString2(int format, TString& str, TPtr)
{
    if (!OpenClipboard(NULL)) return;

    HGLOBAL hMem = GetClipboardData(format);
    if (hMem == NULL)
    {
        CloseClipboard();
        return;
    }

    str = TPtr(GlobalLock(hMem));
    GlobalUnlock(hMem);
    CloseClipboard();
}

template <class TString, class TPtr>
void PrivateReadClipboardString(int format, TString& str, TPtr p)
{
    PrivateReadClipboardString2(format, str, p);
    if (str.IsEmpty())
    {
        Sleep(10);
        PrivateReadClipboardString2(format, str, p);
    }
}
}  // namespace

namespace AFLib {
CString ReadClipboardString(int format)
{
    CString s;
    PrivateReadClipboardString(format, s, LPCSTR(NULL));
    return s;
}

CStringW ReadClipboardStringW(int format)
{
    CStringW s;
    PrivateReadClipboardString(format, s, LPCWSTR(NULL));
    return s;
}

void WriteClipboardBlock(HGLOBAL hMem, bool bEmpty, int format)
{
    AfxGetMainWnd()->OpenClipboard();
    if (bEmpty) EmptyClipboard();
    SetClipboardData(format, hMem);
    CloseClipboard();
}

void WriteClipboardString(LPCTSTR s, bool bEmpty, int format)
{
    WriteClipboardBlock(AllocString(s, false), bEmpty, format);
}

void WriteClipboardStringW(LPCWSTR s, bool bEmpty, int format)
{
    WriteClipboardBlock(AllocString(s, true), bEmpty, format);
}

bool IsWin95()
{
    static T3State bWin95 = t3Undef;
    if (bWin95 != t3Undef) return bWin95;

    SetErrorMode(SEM_FAILCRITICALERRORS);
    OSVERSIONINFO ver = {sizeof(OSVERSIONINFO)};

#pragma warning(suppress : 4996)
    if (!GetVersionEx(&ver)) return false;

    switch (ver.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_WINDOWS:
            bWin95 = true;
            break;
        case VER_PLATFORM_WIN32_NT:
            bWin95 = false;
            break;
        default:
            ThrowMessage(LocalAfxString(IDS_WRONG_WIN));
    }

    return bWin95;
}

HANDLE AfxStartProcess(LPCTSTR command, UINT uCmdShow, LPCTSTR startDir)
{
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = uCmdShow;

    CString buffer;
    int bufLen = _tcslen(command) + 16;
    LPTSTR command2 = buffer.GetBuffer(bufLen);
    _tcscpy_s(command2, bufLen, command);

    PROCESS_INFORMATION pi = {0};
    bool bOk = CreateProcess(
                   NULL, command2, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, startDir, &si, &pi) != 0;
    buffer.ReleaseBuffer();

    if (!bOk) return NULL;

    CloseHandle(pi.hThread);
    return pi.hProcess;
}

void InitConsoleApp()
{
    if (AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) return;

    _cputs("Error initializing MFC.\n");
    exit(1);
}

void HandleConsoleExc(CException* pExc)
{
    ConsoleOut(GetExceptionMsg(pExc) + strEol + strEol);
    pExc->Delete();
    exit(1);
}

void ConsoleOut(LPCTSTR s)
{
    _cputts(s);
}

void StandardOut(LPCTSTR s)
{
    for (; *s != 0; ++s) _puttchar(*s);
}
}  // namespace AFLib
