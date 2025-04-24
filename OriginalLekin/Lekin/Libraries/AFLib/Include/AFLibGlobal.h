#pragma once

#include "AFLibPointers.h"
#include "AFLibStrings.h"

/////////////////////////////////////////////////////////////////////////////
// Various global functions and constants

#ifdef StrToInt
#undef StrToInt
#endif

namespace ATL {
class COleDateTime;
class COleDateTimeSpan;
class CRegKey;
}  // namespace ATL

namespace AFLib {  // large integer, but still far from MAXINT
const int MXI = 100000000;

// zero (invalid) time
AFLIB extern const ATL::COleDateTime odtZero;

// zero time span
AFLIB extern const ATL::COleDateTimeSpan odtsZero;

// logical function operator
enum TLogicalFunction
{
    lfnAnd,
    lfnAndN,
    lfnOr,
    lfnXor
};

//*** Functions for int64 ***

// make 64-bit int from two 32-bit
inline UINT64 MAKEUINT64(int hi32, int lo32)
{
    return (UINT64(hi32) << 32) | UINT(lo32);
}

// extract higher 32 bits from a 64-bit int
inline UINT HIGH32(UINT64 i64)
{
    return UINT(i64 >> 32);
}

// extract lower 32 bits from a 64-bit int
inline UINT LOW32(UINT64 i64)
{
    return UINT(i64);
}

//*** Security functions ***

// allow access to dir for everyone; create dir if necessary
AFLIB void CreateUnsecureDir(LPCTSTR dir);

// allow access to a registry key for everyone; create the key if necessary
AFLIB void CreateUnsecureRegKey(HKEY hKeyRoot, LPCTSTR path);

// true for Windows-95/98, false for NT/2000
AFLIB bool IsWin95();

//*** Registry functions ***

// types of owner info supported by GetOwnerInfo()
enum TGetOwnerInfo
{
    goiUser,
    goiCompany,
    goiProductID,
    goiN
};

// "software\microsoft\windows"
AFLIB CString GetWindowsRegKeyPath(bool bNT);

// get the name of the predefined key
AFLIB CString GetRootRegKeyName(HKEY hKeyRoot);

// open a sub-key; will create if allowed and needed
AFLIB ATL::CRegKey OpenRegSubKey(HKEY hKeyRoot, LPCTSTR path, bool bWriteAccess, int accessRights = 0);

// copy key from hKeyRootSrc\pathSrc to hKeyRootDst\pathDst
AFLIB void CopyRegKey(HKEY hKeyRootSrc, LPCTSTR pathSrc, HKEY hKeyRootDst, LPCTSTR pathDst);

// get a string value from registry
AFLIB CString GetRegValueStr(HKEY hKey, LPCTSTR sec, LPCTSTR def);

// get an integer value from registry
AFLIB int GetRegValueInt(HKEY hKey, LPCTSTR sec, int def);

// set a string value in the registry
AFLIB bool SetRegValueStr(HKEY hKey, LPCTSTR sec, LPCTSTR value);

// set an integer value in the registry
AFLIB bool SetRegValueInt(HKEY hKey, LPCTSTR sec, int value);

// get the owner or the company name
AFLIB CString GetOwnerInfo(TGetOwnerInfo goi);

// get the ID of the CPU (from the CPU itself, not from registry)
AFLIB CString GetComputerID();

// get a profile string from an INI file
AFLIB CString GetPrivateProfileValueStr(LPCTSTR section, LPCTSTR key, LPCTSTR def, LPCTSTR file);

//*** Registry read/write functions.
//*** If main application class if CAppAF, call its virtual functions.
//*** Otherwise call general CWinApp functions.

// get string from the registry (through the app)
AFLIB CString DynamicGetProfileString(LPCTSTR section, LPCTSTR entry, LPCTSTR def);

// write a string into the registry (through the app)
AFLIB bool DynamicWriteProfileString(LPCTSTR section, LPCTSTR entry, LPCTSTR value);

// get integer from the registry (through the app)
AFLIB int DynamicGetProfileInt(LPCTSTR section, LPCTSTR entry, int def);

// write an integer into the registry (through the app)
AFLIB bool DynamicWriteProfileInt(LPCTSTR section, LPCTSTR entry, int value);

//*** Bit field functions ***

// (number & mask), shift right by an appropriate count
AFLIB DWORD GetBitField(DWORD number, DWORD mask);

// update bit field in 'number' specified by 'mask' to 'field'
AFLIB DWORD UpdateBitField(DWORD number, DWORD mask, DWORD field);

//*** int/double/string conversion functions ***

// convert string to double
AFLIB double StrToDouble(LPCTSTR s);

// convert double to string; cut trailing zeros
AFLIB CString DoubleToStr(double d, int precision = 6, bool bForceSign = false);

// convert double to string; do NOT cut trailing zeros
AFLIB CString DoubleToStrStrict(double d, int precision = 6, bool bForceSign = false);

// convert percentage to string; cut trailing zeros
AFLIB CString PercentToStr(double d, int precision = 2, bool bForceSign = false);

// convert percentage to string; do NOT cut trailing zeros
AFLIB CString PercentToStrStrict(double d, int precision = 2, bool bForceSign = false);

// convert string to integer
AFLIB int StrToInt(LPCTSTR s);

// convert string to 64-bit integer
AFLIB INT64 StrToInt64(LPCTSTR s);

// convert integer to string
AFLIB CString IntToStr(INT64 a);

// convert string to unsigned integer
AFLIB UINT StrToUint(LPCTSTR s);

// convert string to 64-bit unsigned integer
AFLIB UINT64 StrToUint64(LPCTSTR s);

// convert unsigned integer to string
AFLIB CString UintToStr(UINT64 a);

// convert hexadecimal representation to unsigned integer
AFLIB UINT StrToHex(LPCTSTR s);

// convert hexadecimal representation to 64-bit unsigned integer
AFLIB UINT64 StrToHex64(LPCTSTR s);

// get hexadecimal representation
AFLIB CString HexToStr(UINT64 a, int digits);

// convert bool to plus or minus
AFLIB const CString& BoolToPM(bool b);

// convert string into Time structure
AFLIB COleDateTime StrToTime(LPCTSTR s, bool bStrict = true);

// convert time to string using the most common format
AFLIB CString TimeToStr(const COleDateTime& odt);

//*** easy string tokenizer ***

// count the number of delimited substrings
AFLIB int CountTokens(LPCTSTR line, LPCTSTR delimiter);

// get n-th substring
AFLIB CString GetTokenAt(LPCTSTR line, LPCTSTR delimiter, int n);

// check if ampersand-ch is a part of str
AFLIB bool IsKeyHotInString(LPCTSTR str, TCHAR ch);

//*** conversion from TSTR to BSTR ***

// allocate a BSTR, initialize with ascii string
AFLIB BSTR AFSysAllocStringLen(LPCSTR str, int len = -1);

// allocate a BSTR, initialize with Unicode string
AFLIB BSTR AFSysAllocStringLen(LPCWSTR str, int len = -1);

// allocate a BSTR, initialize with BSTR
AFLIB BSTR AFSysAllocStringBstr(BSTR src);

// allocate a BSTR, do not initialize
AFLIB inline BSTR AFSysAllocString(int len)
{
    return AFSysAllocStringLen(LPCWSTR(NULL), len);
}

// allocate a BSTR, initialize with CStringA
AFLIB inline BSTR AFSysAllocString(const CStringA& str)
{
    return AFSysAllocStringLen(str, str.GetLength());
}

// allocate a BSTR, initialize with CStringW
AFLIB inline BSTR AFSysAllocString(const CStringW& str)
{
    return AFSysAllocStringLen(str, str.GetLength());
}

// allocate a BSTR, initialize with ascii string, free old bstr
AFLIB void AFSysReAllocStringLen(BSTR& bstr, LPCSTR str, int len = -1);

// allocate a BSTR, initialize with Unicode string, free old bstr
AFLIB void AFSysReAllocStringLen(BSTR& bstr, LPCWSTR str, int len = -1);

// allocate a BSTR, initialize with BSTR, free old bstr
AFLIB void AFSysReAllocStringBstr(BSTR& bstr, BSTR src);

// allocate a BSTR, initialize with CStringA, free old bstr
AFLIB inline void AFSysReAllocString(BSTR& bstr, const CStringA& str)
{
    AFSysReAllocStringLen(bstr, str, str.GetLength());
}

// allocate a BSTR, initialize with CStringW, free old bstr
AFLIB inline void AFSysReAllocString(BSTR& bstr, const CStringW& str)
{
    AFSysReAllocStringLen(bstr, str, str.GetLength());
}

//*** memory allocation ***

// put a block in global memory (for clipboard copy)
AFLIB HGLOBAL AllocBlock(LPCVOID pData, int length);

// allocate a fixed (not moveable) global memory block
AFLIB HGLOBAL AllocBlockFixed(LPCVOID pData, int length);

// put string in global memory (for clipboard copy)
AFLIB HGLOBAL AllocString(LPCSTR s, bool bUnicode);
AFLIB HGLOBAL AllocString(LPCWSTR s, bool bUnicode);

// get the amount of available (virtual) memory
AFLIB INT64 GetFreeVirtualMem();

// get the total amount of blocks in the heap
AFLIB INT64 GetTotalHeapMem(HANDLE hHeap, bool bUsedOnly = false);

// enable low-fragmentation heap
AFLIB bool EnableLowFragHeap();

//*** clipboard functions ***

// read string from clipboard
AFLIB CString ReadClipboardString(int format = CF_TEXT);
AFLIB CStringW ReadClipboardStringW(int format = CF_UNICODETEXT);

// write a block of data into clipboard
AFLIB void WriteClipboardBlock(HGLOBAL hMem, bool bEmpty, int format);

// write string into clipboard
AFLIB void WriteClipboardString(LPCTSTR s, bool bEmpty, int format = CF_TEXT);
AFLIB void WriteClipboardStringW(LPCWSTR s, bool bEmpty, int format = CF_UNICODETEXT);

// process enumeration -- works on all Win32 platforms
typedef BOOL(CALLBACK* PROCENUMPROC)(DWORD processID, WORD task16ID, LPCTSTR sExe, LPVOID param);
AFLIB BOOL WINAPI EnumerateProcesses(PROCENUMPROC lpProc, LPVOID param);

// slightly enhanced WinExec
AFLIB HANDLE AfxStartProcess(LPCTSTR command, UINT uCmdShow, LPCTSTR startDir = NULL);

//*** String functions ***

// helper function -- GetErrorMessage is not awfully convenient
AFLIB CString GetExceptionMsg(CException* pExc, LPUINT pIdsHelp = NULL);

// throw CExcMessage
AFLIB void ThrowMessage(LPCTSTR message);
AFLIB void ThrowMessage(UINT ids, AFLIB_NINE_STRINGS);

// throw message "something expected"
AFLIB void ThrowMessageExpected(LPCTSTR s);
AFLIB void ThrowMessageExpected(UINT ids);

// throw message "Cannot open DB"
AFLIB void ThrowMessageNoDB();

// check if the string starts with a pattern
AFLIB bool StringStartsWith(LPCTSTR s, LPCTSTR pattern);
AFLIB bool StringStartsWithIC(LPCTSTR s, LPCTSTR pattern);

// check if the string ends with a pattern
AFLIB bool StringEndsWith(LPCTSTR s, LPCTSTR pattern);
AFLIB bool StringEndsWithIC(LPCTSTR s, LPCTSTR pattern);

// put string into quotes
AFLIB CString QuoteString(LPCTSTR s);

// put string into quotes for SQL query
AFLIB CString QuoteStringSQL(LPCTSTR s);

// same, but be careful with standard "like" wildcard symbols
AFLIB CString QuoteStringLike(LPCTSTR s);

// same, but be careful with MS Access "like" wildcard symbols
AFLIB CString QuoteStringLikeAcc(LPCTSTR s);

// take Access LIKE pattern, convert into standard SQL, put in quotes
AFLIB CString PrepareAccLike(LPCTSTR s, bool bMulti);

// modify a title so that it doesn't appear in arrRestrict
AFLIB void CreateUniqueTitle(CString& title, const CStringArray& arrRestrict);

// modify an ID so that it doesn't appear in arrRestrict
AFLIB void CreateUniqueID(CString& ID, const CStringArray& arrRestrict);

// call in the beginning of _tmain() in a console app
AFLIB void InitConsoleApp();

// handle an exception in a console app (print/exit)
AFLIB void HandleConsoleExc(CException* pExc);

// output a string to the console
AFLIB void ConsoleOut(LPCTSTR s);

// output a string to the standard output
AFLIB void StandardOut(LPCTSTR s);

// get a system string in a nice way: get buffer length, then string itself
template <class F>
inline CString GetNiceString(F f)
{
    CString s;
    int length = f(NULL, 0);
    if (length <= 0) return s;

    LPTSTR buffer = s.GetBuffer(length + 2);
    f(buffer, length + 1);
    buffer[length] = 0;
    s.ReleaseBuffer();
    return s;
}

// get a system string in an ugly way: double the buffer until it fits
template <class F>
inline CString GetUglyString(F f)
{
    int length = 1024;

    while (true)
    {
        CString s;
        LPTSTR buffer = s.GetBuffer(length + 2);
        f(buffer, length + 1);
        buffer[length] = 0;
        s.ReleaseBuffer();
        if (s.GetLength() < length - 1) return s;
        length *= 2;
    }
}

// Is..., To... character functions
// Defined for simple or wide characters depending on _UNICODE

#undef _istalnum
#undef _istalpha
#undef _istascii
#undef _istcntrl
#undef _istdigit
#undef _istgraph
#undef _istlower
#undef _istprint
#undef _istpunct
#undef _istspace
#undef _istupper
#undef _istxdigit

#undef _totupper
#undef _totlower

#ifdef _UNICODE
#define DEFINE_IS(Is, is, isw)     \
    AFLIB bool Is(LPCTSTR s);      \
    inline bool Is(WCHAR ch)       \
    {                              \
        return isw(WORD(ch)) != 0; \
    }
#else
#define DEFINE_IS(Is, is, isw)    \
    AFLIB bool Is(LPCTSTR s);     \
    inline bool Is(CHAR ch)       \
    {                             \
        return is(BYTE(ch)) != 0; \
    }
#endif

DEFINE_IS(IsAlnum, isalnum, iswalnum)
DEFINE_IS(IsAlpha, isalpha, iswalpha)
DEFINE_IS(IsAscii, isascii, iswascii)
DEFINE_IS(IsCntrl, iscntrl, iswcntrl)
DEFINE_IS(IsDigit, isdigit, iswdigit)
DEFINE_IS(IsLower, islower, iswlower)
DEFINE_IS(IsPunct, ispunct, iswpunct)
DEFINE_IS(IsSpace, isspace, iswspace)
DEFINE_IS(IsUpper, isupper, iswupper)
DEFINE_IS(IsXDigit, isxdigit, iswxdigit)

#undef DEFINE_IS

AFLIB bool IsAlnumU(LPCTSTR s);
AFLIB bool IsAlphaU(LPCTSTR s);
AFLIB bool IsStrictID(LPCTSTR s);

#ifdef _UNICODE
inline bool IsAlnumU(WCHAR ch)
{
    return iswalnum(ch) || ch == L'_';
}
inline bool IsAlphaU(WCHAR ch)
{
    return iswalpha(ch) || ch == L'_';
}
inline WCHAR ToUpper(WCHAR ch)
{
    return (WCHAR)towupper(WORD(ch));
}
inline WCHAR ToLower(WCHAR ch)
{
    return (WCHAR)towlower(WORD(ch));
}
#else
inline bool IsAlnumU(CHAR ch)
{
    return isalnum(ch) || ch == '_';
}
inline bool IsAlphaU(CHAR ch)
{
    return isalpha(ch) || ch == '_';
}
inline CHAR ToUpper(CHAR ch)
{
    return toupper(BYTE(ch));
}
inline CHAR ToLower(CHAR ch)
{
    return tolower(BYTE(ch));
}
#endif
}  // namespace AFLib
