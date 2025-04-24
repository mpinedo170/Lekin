#include "StdAfx.h"

#include "AFLibCalcGlobal.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStrings.h"
#include "AFLibThreadSafeConst.h"

using AFLibIO::TStringBuffer;
using namespace AFLib;

namespace {
CString PrivateGeneratePrint(LPCTSTR format, LPCTSTR* pStrings)
{
    SAFE_CONST_STR(strPrint, _T("Print("));
    SAFE_CONST_STR(strPrintQ, strPrint + strDQuote);
    SAFE_CONST_STR(strFinal, _T("\");") + strEol);
    TStringBuffer buffer;

    for (LPCTSTR ss = format; *ss != 0; ++ss)
    {
        TCHAR ch = *ss;

        switch (ch)
        {
            case chrBSlash:
                if (buffer.IsEmpty()) buffer += strPrintQ;
                buffer += _T("\\\\");
                break;

            case chrDQuote:
                if (buffer.IsEmpty()) buffer += strPrintQ;
                buffer += _T("\\\"");
                break;

            case chrPercent:
                if (ss[1] >= _T('1') && ss[1] <= _T('9'))
                {
                    ++ss;
                    int n = *ss - _T('1');
                    LPCTSTR s = pStrings[n];

                    if (IsEmptyStr(s)) break;

                    if (StringStartsWith(s, strDQuote) && StringEndsWith(s, strDQuote))
                    {
                        int len = _tcslen(s) - 1;

                        if (!buffer.IsEmpty())
                        {
                            ++s;
                            --len;
                        }

                        buffer.Append(s, len);
                    }
                    else
                    {
                        if (!buffer.IsEmpty()) buffer += _T("\", ");
                        buffer += pStrings[n];
                        buffer += _T(", \"");
                    }
                    break;
                }

                // fall through to default
            default:
                // ignore special and Non-Ascii characters
                if (ch < chrSpace || ch >= 127) break;

                if (buffer.IsEmpty()) buffer += strPrintQ;
                buffer += ch;
                break;
        }
    }

    if (!buffer.IsEmpty()) buffer += strFinal;

    return LPCTSTR(buffer);
}
}  // namespace

namespace AFLibCalc {
AFLIB CString GeneratePrint(UINT ids, AFLIB_NINE_STRINGS_LOCAL)
{
    CString format;
    format.LoadString(ids);
    return PrivateGeneratePrint(format, &str1);
}

AFLIB CString GeneratePrint(LPCTSTR format, AFLIB_NINE_STRINGS_LOCAL)
{
    return PrivateGeneratePrint(format, &str1);
}
}  // namespace AFLibCalc
