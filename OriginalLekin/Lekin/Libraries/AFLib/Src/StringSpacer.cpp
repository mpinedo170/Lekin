#include "StdAfx.h"

#include "AFLibStringSpacer.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibIO::TStringSpacer;

TStringSpacer::TStringSpacer(LPCTSTR sPtr) : super(sPtr, true)
{}

CStringW TStringSpacer::ReadCStrW()
{
    ProtectedTestSymbolHard(chrDQuote);

    SAFE_STATIC_SB_W(buffer);

    while (true)
    {
        TCHAR ch = GetCurrent();
        ProtectedNext();

        switch (ch)
        {
            case 0:
                ThrowMessage(LocalAfxString(IDS_NO_QUOTE));

            case chrNewLine:
            case chrCarRet:
                ThrowMessage(LocalAfxString(IDS_EOL_IN_STRING));

            case chrDQuote:
                ES();
                if (GetConcat() && ProtectedTestSymbol(chrDQuote)) break;
                return LPCWSTR(buffer);

            case chrBSlash:
                ch = GetCurrent();
                ProtectedNext();

                switch (ch)
                {
                    case 0:
                        ThrowMessage(LocalAfxString(IDS_END_AFTER_BS));

                    case _T('n'):
                        buffer += strEolW;
                        continue;

                    case _T('t'):
                        buffer += chrTabW;
                        continue;

                    case _T('f'):
                        buffer += chrFormFeedW;
                        continue;

                    case _T('x'):
                    case _T('X'):
                    {
                        bool bUnicode = ch == _T('X');
                        SAFE_STATIC_SB(bufHex);

                        ProtectedReadSegment(bUnicode ? 4 : 2, bufHex);
                        int i = 0;
                        if (_stscanf_s(bufHex, _T("%X"), &i) < 1) LocalThrowMessageExpected(IDS_HEX);
                        if (i != 0) buffer += WCHAR(i);
                        continue;
                    }
                }
                // if none of these, fall out to regular char

            default:
#ifdef _UNICODE
                if (ch < ' ' || ch == 127) ThrowMessage(LocalAfxString(IDS_SPEC_IN_STRING));
                buffer += ch;
#else
                if (BYTE(ch) < ' ' || BYTE(ch) == 127) ThrowMessage(LocalAfxString(IDS_SPEC_IN_STRING));
                buffer += WCHAR(BYTE(ch));
#endif
        }
    }
}

#ifndef _UNICODE
CString TStringSpacer::ReadCStr()
{
    return CString(ReadCStrW());
}
#endif

CString TStringSpacer::ReadSafeStr()
{
    if (!TestSymbolNoMove(strDQuote))
    {
        SAFE_STATIC_SB(buffer);

        while (IsChrSafe(GetCurrent()))
        {
            buffer += GetCurrent();
            ProtectedNext();
        }
        ES();
        return LPCTSTR(buffer);
    }

    TConcatManipulator cm(*this, false);
    return ReadCStr();
}
