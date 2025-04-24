#pragma once

/////////////////////////////////////////////////////////////////////////////
// Various strings and string-related functions

// For use in a function declaration (prototype).
// 9 string parameters with default values

#define AFLIB_NINE_STRINGS                                                                                  \
    LPCTSTR str1 = AFLib::strEmpty, LPCTSTR str2 = AFLib::strEmpty, LPCTSTR str3 = AFLib::strEmpty,         \
            LPCTSTR str4 = AFLib::strEmpty, LPCTSTR str5 = AFLib::strEmpty, LPCTSTR str6 = AFLib::strEmpty, \
            LPCTSTR str7 = AFLib::strEmpty, LPCTSTR str8 = AFLib::strEmpty, LPCTSTR str9 = AFLib::strEmpty

// For use in a function definition.
// 9 string parameters

#define AFLIB_NINE_STRINGS_LOCAL \
    LPCTSTR str1, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR

// For template classes where the base class if a template parameter.
// Helps to "inherit" constructors from the base class.
// Requires the base class to have a default constructor.
// Requires the new class to have the PrivateInit() function.

#define AFLIB_TEMP_CONSTRUCTORS(THIS)                                                                      \
    THIS() : super()                                                                                       \
    {                                                                                                      \
        PrivateInit();                                                                                     \
    }                                                                                                      \
    template <class T1>                                                                                    \
    explicit THIS(const T1& t1) : super(t1)                                                                \
    {                                                                                                      \
        PrivateInit();                                                                                     \
    }                                                                                                      \
    template <class T1, class T2>                                                                          \
    THIS(const T1& t1, const T2& t2) : super(t1, t2)                                                       \
    {                                                                                                      \
        PrivateInit();                                                                                     \
    }                                                                                                      \
    template <class T1, class T2, class T3>                                                                \
    THIS(const T1& t1, const T2& t2, const T3& t3) : super(t1, t2, t3)                                     \
    {                                                                                                      \
        PrivateInit();                                                                                     \
    }                                                                                                      \
    template <class T1, class T2, class T3, class T4>                                                      \
    THIS(const T1& t1, const T2& t2, const T3& t3, const T4& t4) : super(t1, t2, t3, t4)                   \
    {                                                                                                      \
        PrivateInit();                                                                                     \
    }                                                                                                      \
    template <class T1, class T2, class T3, class T4, class T5>                                            \
    THIS(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) : super(t1, t2, t3, t4, t5) \
    {                                                                                                      \
        PrivateInit();                                                                                     \
    }

namespace ATL {
class COleDateTimeSpan;
}

#define DEFINE_STR(id)                 \
    AFLIB extern const CString id;     \
    AFLIB extern const CStringA id##A; \
    AFLIB extern const CStringW id##W;

#define DEFINE_CHR(id, chr)   \
    const TCHAR id = _T(chr); \
    const CHAR id##A = chr;   \
    const WCHAR id##W = L##chr;

// Frequently used strings and characters (both Ascii and Unicode definitions)

namespace AFLib {
DEFINE_STR(strEmpty)
DEFINE_STR(strPrefixer)
DEFINE_STR(strEol)
DEFINE_STR(str2Space)
DEFINE_STR(str4Space)
DEFINE_STR(str2Quote)
DEFINE_STR(strCommaSpace)
DEFINE_STR(strEllipsis)
DEFINE_STR(strPathSeparators)

DEFINE_STR(strNewLine)
DEFINE_STR(strCarRet);
DEFINE_STR(strFormFeed);
DEFINE_STR(strTab);
DEFINE_STR(strQuestion);
DEFINE_STR(strDot);
DEFINE_STR(strSQuote);
DEFINE_STR(strDQuote);
DEFINE_STR(strBSlash);
DEFINE_STR(strComma);
DEFINE_STR(strSemicolon);
DEFINE_STR(strSlash);
DEFINE_STR(strSpace);
DEFINE_STR(strStar);
DEFINE_STR(strDash);
DEFINE_STR(strPlus);
DEFINE_STR(strEqual);
DEFINE_STR(strColon);
DEFINE_STR(strExclam);
DEFINE_STR(strPound);
DEFINE_STR(strPercent);
DEFINE_STR(strUnderscore);
DEFINE_STR(strHat);
DEFINE_STR(strPipe);
DEFINE_STR(strTilda);
DEFINE_STR(strDollar);
DEFINE_STR(strAt);
DEFINE_STR(strCenterDot);

DEFINE_STR(strWhitespace);

DEFINE_STR(strArial);
DEFINE_STR(strCourier);
DEFINE_STR(strTahoma);
DEFINE_STR(strRoman);
DEFINE_STR(strSymbol);
DEFINE_STR(strVerdana);

DEFINE_STR(strNew);
DEFINE_STR(strOK);

DEFINE_STR(extExe);
DEFINE_STR(extLog);
DEFINE_STR(extTxt);
DEFINE_STR(extCsv);
DEFINE_STR(extZip);

DEFINE_CHR(chrNewLine, '\n');
DEFINE_CHR(chrCarRet, '\r');
DEFINE_CHR(chrFormFeed, '\f');
DEFINE_CHR(chrTab, '\t');
DEFINE_CHR(chrQuestion, '?');
DEFINE_CHR(chrDot, '.');
DEFINE_CHR(chrSQuote, '\'');
DEFINE_CHR(chrDQuote, '"');
DEFINE_CHR(chrBSlash, '\\');
DEFINE_CHR(chrComma, ',');
DEFINE_CHR(chrSemicolon, ';');
DEFINE_CHR(chrSlash, '/');
DEFINE_CHR(chrSpace, ' ');
DEFINE_CHR(chrStar, '*');
DEFINE_CHR(chrDash, '-');
DEFINE_CHR(chrPlus, '+');
DEFINE_CHR(chrEqual, '=');
DEFINE_CHR(chrColon, ':');
DEFINE_CHR(chrExclam, '!');
DEFINE_CHR(chrPound, '#');
DEFINE_CHR(chrPercent, '%');
DEFINE_CHR(chrUnderscore, '_');
DEFINE_CHR(chrHat, '^');
DEFINE_CHR(chrPipe, '|');
DEFINE_CHR(chrTilda, '~');
DEFINE_CHR(chrDollar, '$');
DEFINE_CHR(chrAt, '@');
DEFINE_CHR(chrCenterDot, '\xB7');

const WCHAR chrBulletW = 0x2022;
const WCHAR chrDiamondW = 0x25CA;
const WCHAR chrGreekAlphaW = 0x03B1;
const WCHAR chrGreekBetaW = 0x03B2;
const WCHAR chrGreekMuW = 0x03BC;
const WCHAR chrGreekSigmaW = 0x03C3;
const WCHAR chrDiagonalW = 0x2044;
const WCHAR chrOverlineW = 0x203E;

const WCHAR chrSquaredW = 0x00B2;
const WCHAR chrSquareRootW = 0x221A;
const WCHAR chrInfinityW = 0x221E;
const WCHAR chrLeqW = 0x2264;
const WCHAR chrGeqW = 0x2265;

const WCHAR chrLargeMacronW = 0x00AF;
const WCHAR chrSmallMacronW = 0x02C9;

const WCHAR chrTriangleUpW = 0x25B2;
const WCHAR chrTriangleDownW = 0x25BC;
const WCHAR chrTriangleLeftW = 0x25C4;
const WCHAR chrTriangleRightW = 0x25BA;

// is string null or empty?
inline bool IsEmptyStr(LPCSTR s)
{
    return s == NULL || s[0] == 0;
}

// is string null or empty?
inline bool IsEmptyStr(LPCWSTR s)
{
    return s == NULL || s[0] == 0;
}

// capitalize a string
AFLIB void Capitalize(CString& s);
AFLIB void Capitalize(LPTSTR s);

// replace whitespace with single space
AFLIB CString NormalizeWhitespace(LPCTSTR s);

// load the string from the resource and insert up to 9 parameters
AFLIB CString AfxString(UINT ids, AFLIB_NINE_STRINGS);
AFLIB CString AfxString(HINSTANCE hInst, UINT ids, AFLIB_NINE_STRINGS);
AFLIB CString AfxString(LPCTSTR format, AFLIB_NINE_STRINGS);

// "time spent"
AFLIB CString FormatTimeSpent(const ATL::COleDateTimeSpan& odts);

// convert string into Unicode
AFLIB CStringW EncodeString(int encoding, LPCSTR str, int length = -1);

// this function is provided for compatibility between Unicode & non-Unicode builds:
// ignores encoding
AFLIB CStringW EncodeString(int encoding, LPCWSTR str, int length = -1);
}  // namespace AFLib

namespace AFLibDB {
DEFINE_STR(strUse);
DEFINE_STR(strSelect);
DEFINE_STR(strDistinct);
DEFINE_STR(strInsertInto);
DEFINE_STR(strUpdate);
DEFINE_STR(strDelete);
DEFINE_STR(strSet);
DEFINE_STR(strTop);
DEFINE_STR(strFrom);
DEFINE_STR(strWhere);
DEFINE_STR(strOrderBy);
DEFINE_STR(strGroupBy);
DEFINE_STR(strHaving);
DEFINE_STR(strDesc);
DEFINE_STR(strValues);
DEFINE_STR(strAs);
DEFINE_STR(strTrue);
DEFINE_STR(strFalse);
DEFINE_STR(strNot)
DEFINE_STR(strAnd)
DEFINE_STR(strOr)
DEFINE_STR(strIn)
DEFINE_STR(strBetween)
DEFINE_STR(strLike)
DEFINE_STR(strIs)
DEFINE_STR(strNull)
}  // namespace AFLibDB

#undef DEFINE_CHR
#undef DEFINE_STR
