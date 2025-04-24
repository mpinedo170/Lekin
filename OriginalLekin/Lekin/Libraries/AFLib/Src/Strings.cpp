#include "StdAfx.h"

#include "AFLibArrays.h"
#include "AFLibExcMessage.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibModalCounter.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;

namespace AFLibThread {
TThreadSafeConst<CStringA> StaticStringStorageA;
TThreadSafeConst<CStringW> StaticStringStorageW;
}  // namespace AFLibThread

namespace {
UINT PrivateMB(HINSTANCE hInst, UINT ids, UINT nType, LPCTSTR* pStrings)
{
    CString msg = FormatStrings(hInst, ids, pStrings);
    Capitalize(msg);
    AFLibGui::TModalCounter modalCounter;
    return AfxMessageBox(msg, nType, ids);
}

// types of quotings supported
enum TQuoting
{
    qDouble,
    qSingle,
    qLikeAcc,
    qLikeStd
};

CString PrivateQuoteString(LPCTSTR s, TQuoting q)
{
    SAFE_STATIC_SB(strSpecial);
    SAFE_STATIC_SB(buffer);
    TCHAR chrQ = q == qDouble ? chrDQuote : chrSQuote;

    buffer += chrQ;

    switch (q)
    {
        case qLikeAcc:
            strSpecial += _T("*?#[");
            break;
        case qLikeStd:
            strSpecial += _T("%_[");
            break;
    }

    while (true)
    {
        TCHAR ch = *s;

        if (ch == 0)
            break;
        else if (ch == chrQ)
        {
            buffer += chrQ;
            buffer += chrQ;
        }
        else if (strSpecial.Find(ch) >= 0)
        {
            buffer += _T('[');
            buffer += ch;
            buffer += _T(']');
        }
        else
            buffer += ch;

        ++s;
    }

    buffer += chrQ;
    return LPCTSTR(buffer);
}

class TExcMsgFunc
{
private:
    CException* m_pExc;
    LPUINT m_pIdsHelp;

public:
    TExcMsgFunc(CException* pExc, LPUINT pIdsHelp) : m_pExc(pExc), m_pIdsHelp(pIdsHelp)
    {}
    void operator()(LPTSTR buffer, int length)
    {
        m_pExc->GetErrorMessage(buffer, length, m_pIdsHelp);
    }
};
}  // namespace

namespace AFLibPrivate {
const CString strNoData = LocalAfxString(IDS_NO_DATA);
const CString strFontTest = _T("Wg");

const BYTE arrStrTextPrefix[ttfN][lenTextPrefixMax] = {{0xEF, 0xBB, 0xBF}, {0xFF, 0xFE}, {0xFE, 0xFF}};

extern const int arrLenTextPrefix[AFLibIO::ttfN] = {3, 2, 2};

CString FormatStrings(HINSTANCE hInst, UINT ids, LPCTSTR* pStrings)
{
    CString format;
    format.LoadString(hInst, ids);
    CString s;
    AfxFormatStrings(s, format, pStrings, 9);
    return s;
}

CString LocalAfxString(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    return FormatStrings(AFLibDLL.hModule, ids, &str1);
}

void LocalThrowMessageExpected(UINT ids)
{
    CString s;
    s.LoadString(AFLibDLL.hModule, ids);
    ThrowMessageExpected(s);
}

UINT LocalAfxMB(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    return PrivateMB(AFLibDLL.hModule, ids, MB_OK, &str1);
}

bool LocalStringStartsWith(LPCTSTR s, LPCTSTR pattern, bool bIgnoreCase)
{
    return (bIgnoreCase ? _tcsnicmp(s, pattern, _tcslen(pattern)) : _tcsncmp(s, pattern, _tcslen(pattern))) ==
           0;
}

bool LocalStringEndsWith(LPCTSTR s, LPCTSTR pattern, bool bIgnoreCase)
{
    int lenS = _tcslen(s);
    int lenP = _tcslen(pattern);

    return lenS >= lenP &&
           (bIgnoreCase ? _tcsicmp(s + lenS - lenP, pattern) : _tcscmp(s + lenS - lenP, pattern)) == 0;
}
}  // namespace AFLibPrivate

#define DEFINE_IS(Is)                  \
    bool Is(LPCTSTR s)                 \
    {                                  \
        for (; *s != 0; ++s)           \
            if (!Is(*s)) return false; \
        return true;                   \
    }

#define DEFINE_STR(id, str)     \
    const CString id = _T(str); \
    const CStringA id##A = str; \
    const CStringW id##W = L##str;

namespace AFLib {
DEFINE_IS(IsAlnum)
DEFINE_IS(IsAlnumU)
DEFINE_IS(IsAlpha)
DEFINE_IS(IsAlphaU)
DEFINE_IS(IsAscii)
DEFINE_IS(IsCntrl)
DEFINE_IS(IsDigit)
DEFINE_IS(IsLower)
DEFINE_IS(IsPunct)
DEFINE_IS(IsSpace)
DEFINE_IS(IsUpper)
DEFINE_IS(IsXDigit)

DEFINE_STR(strEmpty, "")
DEFINE_STR(strPrefixer, ": ")
DEFINE_STR(strEol, "\r\n")
DEFINE_STR(str2Space, "  ");
DEFINE_STR(str4Space, "    ");
DEFINE_STR(str2Quote, "\"\"")
DEFINE_STR(strCommaSpace, ", ")
DEFINE_STR(strEllipsis, "...")
DEFINE_STR(strPathSeparators, ":/\\")

DEFINE_STR(strNewLine, "\n")
DEFINE_STR(strCarRet, "\r");
DEFINE_STR(strFormFeed, "\f");
DEFINE_STR(strTab, "\t");
DEFINE_STR(strQuestion, "?");
DEFINE_STR(strDot, ".");
DEFINE_STR(strSQuote, "'");
DEFINE_STR(strDQuote, "\"");
DEFINE_STR(strBSlash, "\\");
DEFINE_STR(strComma, ",");
DEFINE_STR(strSemicolon, ";");
DEFINE_STR(strSlash, "/");
DEFINE_STR(strSpace, " ");
DEFINE_STR(strStar, "*");
DEFINE_STR(strDash, "-");
DEFINE_STR(strPlus, "+");
DEFINE_STR(strEqual, "=");
DEFINE_STR(strColon, ":");
DEFINE_STR(strExclam, "!");
DEFINE_STR(strPound, "#");
DEFINE_STR(strPercent, "%");
DEFINE_STR(strUnderscore, "_")
DEFINE_STR(strHat, "^");
DEFINE_STR(strPipe, "|");
DEFINE_STR(strTilda, "~");
DEFINE_STR(strDollar, "$");
DEFINE_STR(strAt, "@");
DEFINE_STR(strCenterDot, "\xB7");

DEFINE_STR(strWhitespace, "\t\f\n\r\v ");

DEFINE_STR(strArial, "Arial");
DEFINE_STR(strCourier, "Courier New");
DEFINE_STR(strTahoma, "Tahoma");
DEFINE_STR(strRoman, "Times New Roman");
DEFINE_STR(strSymbol, "Symbol");
DEFINE_STR(strVerdana, "Verdana");

DEFINE_STR(strNew, "New");
DEFINE_STR(strOK, "OK");

DEFINE_STR(extExe, ".exe");
DEFINE_STR(extLog, ".log");
DEFINE_STR(extTxt, ".txt");
DEFINE_STR(extCsv, ".csv");
DEFINE_STR(extZip, ".zip");

bool IsStrictID(LPCTSTR s)
{
    return !IsEmptyStr(s) && IsAlpha(s[0]) && IsAlnum(s);
}

void Capitalize(CString& s)
{
    if (s.GetLength() == 0) return;
    s.SetAt(0, ToUpper(s[0]));
}

void Capitalize(LPTSTR s)
{
    s[0] = ToUpper(s[0]);
}

CString NormalizeWhitespace(LPCTSTR s)
{
    SAFE_STATIC_SB(buffer);
    TStringSpacer sp(s);

    while (!sp.IsEof())
    {
        if (!buffer.IsEmpty()) buffer += chrSpace;
        buffer += sp.ReadToSpace();
    }

    return LPCTSTR(buffer);
}

CString GetExceptionMsg(CException* pExc, LPUINT pIdsHelp)
{
    CString msg = GetUglyString(TExcMsgFunc(pExc, pIdsHelp));
    if (!msg.IsEmpty()) return msg;

#ifdef _UNICODE
    return LocalAfxString(IDS_UNCLASSIFIED, CString(pExc->GetRuntimeClass()->m_lpszClassName));
#else
    return LocalAfxString(IDS_UNCLASSIFIED, pExc->GetRuntimeClass()->m_lpszClassName);
#endif
}

CString AfxString(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    return FormatStrings(AfxGetApp()->m_hInstance, ids, &str1);
}

CString AfxString(HINSTANCE hInst, UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    return FormatStrings(hInst, ids, &str1);
}

CString AfxString(LPCTSTR format, AFLIB_NINE_STRINGS_LOCAL)
{
    CString s;
    AfxFormatStrings(s, format, &str1, 9);
    return s;
}

CString FormatTimeSpent(const COleDateTimeSpan& odts)
{
    CString format;
    format.LoadString(AFLibDLL.hModule, odts.GetTotalDays() >= 1 ? IDS_TIME_SPENT : IDS_TIME_SPENT_NO_DAY);
    return odts.Format(format);
}

CStringW EncodeString(int encoding, LPCSTR str, int length)
{
    if (length == -1) length = strlen(str);

    CStringW s;
    LPWSTR dst = s.GetBuffer(length + 1);

    ::MultiByteToWideChar(encoding, 0, str, length, dst, length * 2);
    dst[length] = 0;
    s.ReleaseBuffer();
    return s;
}

CStringW EncodeString(int, LPCWSTR str, int length)
{
    return length < 0 ? CStringW(str) : CStringW(str, length);
}

void ThrowMessage(LPCTSTR message)
{
    throw new CExcMessage(message);
}

void ThrowMessage(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    throw new CExcMessage(ids, FormatStrings(AfxGetApp()->m_hInstance, ids, &str1));
}

void ThrowMessageExpected(LPCTSTR s)
{
    ThrowMessage(LocalAfxString(IDS_EXPECTED, s));
}

void ThrowMessageExpected(UINT ids)
{
    CString message(MAKEINTRESOURCE(ids));
    ThrowMessageExpected(message);
}

void ThrowMessageNoDB()
{
    ThrowMessage(IDS_CANT_OPEN_DB);
}

bool StringStartsWith(LPCTSTR s, LPCTSTR pattern)
{
    return LocalStringStartsWith(s, pattern, false);
}

bool StringStartsWithIC(LPCTSTR s, LPCTSTR pattern)
{
    return LocalStringStartsWith(s, pattern, true);
}

bool StringEndsWith(LPCTSTR s, LPCTSTR pattern)
{
    return LocalStringEndsWith(s, pattern, false);
}

bool StringEndsWithIC(LPCTSTR s, LPCTSTR pattern)
{
    return LocalStringEndsWith(s, pattern, true);
}

CString QuoteString(LPCTSTR s)
{
    return PrivateQuoteString(s, qDouble);
}

CString QuoteStringSQL(LPCTSTR s)
{
    return PrivateQuoteString(s, qSingle);
}

CString QuoteStringLike(LPCTSTR s)
{
    return PrivateQuoteString(s, qLikeStd);
}

CString QuoteStringLikeAcc(LPCTSTR s)
{
    return PrivateQuoteString(s, qLikeAcc);
}

CString PrepareAccLike(LPCTSTR s, bool bMulti)
{
    SAFE_STATIC_SB(buffer);
    bool bBracket = false;

    buffer += chrSQuote;

    while (true)
    {
        TCHAR ch = *s;

        if (ch == 0) break;

        if (ch >= 0 && ch < chrSpace) ch = chrSpace;

        switch (ch)
        {
            case chrPercent:
            case chrUnderscore:
                if (bBracket)
                    buffer += ch;
                else
                {
                    buffer += _T('[');
                    buffer += ch;
                    buffer += _T(']');
                }
                break;

            case chrStar:
                buffer += bBracket ? ch : chrPercent;
                break;

            case chrQuestion:
                buffer += bBracket ? ch : chrUnderscore;
                break;

            case chrPound:
                if (bBracket)
                    buffer += ch;
                else
                    buffer += _T("[0-9]");
                break;

            case chrExclam:
                buffer += bBracket ? chrHat : ch;
                break;

            case chrSemicolon:
                // Will deliberately insert TAB into a string.
                // Works OK with the SQL Server.
                buffer += bBracket || !bMulti ? ch : chrTab;
                break;

            case chrSQuote:
                buffer.AppendCh(ch, 2);
                break;

            case _T('['):
                buffer += ch;
                bBracket = true;
                break;

            case _T(']'):
                buffer += ch;
                bBracket = false;
                break;

            default:
                buffer += ch;
                break;
        }

        ++s;
    }

    if (bBracket) buffer += _T(']');
    buffer += chrSQuote;
    return LPCTSTR(buffer);
}

void CreateUniqueTitle(CString& title, const CStringArray& arrRestrict)
{
    ASSERT(!title.IsEmpty());

    int i = title.GetLength() - 1;
    int n = 1;
    bool bFound = false;
    CString titlePrefix;

    if (title[i] == _T(')'))
    {
        while (i > 0 && IsDigit(title[i - 1])) --i;

        if (i > 0 && title[i - 1] == _T('('))
        {
            if (i < title.GetLength() - 1) n = StrToInt(title.Mid(i, title.GetLength() - 1 - i)) + 1;
            titlePrefix = title.Left(i);
            bFound = true;
        }
    }

    if (!bFound) titlePrefix = title + _T(" (");

    for (;; ++n)
    {
        bFound = false;
        BSearchStrings(title, arrRestrict, bFound);
        if (!bFound) break;
        title = titlePrefix + IntToStr(n) + _T(")");
    }
}

void CreateUniqueID(CString& ID, const CStringArray& arrRestrict)
{
    ASSERT(!ID.IsEmpty());

    int i = ID.GetLength();
    int n = 1;
    bool bFound = false;
    CString idPrefix;

    while (i > 0 && IsDigit(ID[i - 1]))
    {
        bFound = true;
        --i;
    }

    idPrefix = ID.Left(i);
    if (bFound) n = StrToInt(LPCTSTR(ID) + i) + 1;

    for (;; ++n)
    {
        bFound = false;
        BSearchStrings(ID, arrRestrict, bFound);
        if (!bFound) break;
        ID = idPrefix + IntToStr(n);
    }
}
}  // namespace AFLib

namespace AFLibDB {
DEFINE_STR(strUse, "use ");
DEFINE_STR(strSelect, "select ");
DEFINE_STR(strDistinct, " distinct ");
DEFINE_STR(strInsertInto, "insert into ");
DEFINE_STR(strUpdate, "update ");
DEFINE_STR(strDelete, "delete ");
DEFINE_STR(strSet, " set ");
DEFINE_STR(strTop, " top ");
DEFINE_STR(strFrom, " from ");
DEFINE_STR(strWhere, " where ");
DEFINE_STR(strOrderBy, " order by ");
DEFINE_STR(strGroupBy, " group by ");
DEFINE_STR(strHaving, " having ");
DEFINE_STR(strDesc, " desc ");
DEFINE_STR(strValues, " values ");
DEFINE_STR(strAs, " as ");
DEFINE_STR(strTrue, " (1=1) ");
DEFINE_STR(strFalse, " (1=0) ");
DEFINE_STR(strNot, " not ")
DEFINE_STR(strAnd, " and ")
DEFINE_STR(strOr, " or ")
DEFINE_STR(strIn, " in ")
DEFINE_STR(strBetween, " between ")
DEFINE_STR(strLike, " like ")
DEFINE_STR(strIs, " is ")
DEFINE_STR(strNull, "null")
}  // namespace AFLibDB

namespace AFLibGui {
UINT AfxMB(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    return PrivateMB(AfxGetApp()->m_hInstance, ids, MB_OK, &str1);
}

UINT AfxMB2(UINT nType, UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    return PrivateMB(AfxGetApp()->m_hInstance, ids, nType, &str1);
}

UINT AfxMB(LPCTSTR msg, UINT idHelp)
{
    return AfxMB2(MB_OK, msg, idHelp);
}

UINT AfxMB2(UINT nType, LPCTSTR msg, UINT idHelp)
{
    TModalCounter modalCounter;
    return AfxMessageBox(msg, nType, idHelp);
}

UINT AfxMBDupTitle()
{
    return AfxMB(LocalAfxString(IDS_DUP_TITLE));
}
}  // namespace AFLibGui
