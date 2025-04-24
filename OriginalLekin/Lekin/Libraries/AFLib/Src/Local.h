#pragma once

#include "AFLibIOGlobal.h"
#include "AFLibStrings.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// AFLib "private" global functions and variables.
// These are NOT exported and can NOT be used directly.

namespace AFLibPrivate {  // DLL definition data
extern AFX_EXTENSION_MODULE AFLibDLL;

// "no data" string; used when an empty string is a legitimate value
extern const CString strNoData;

// a string to test font's width and height
extern const CString strFontTest;

// max length of a text file prefix
const int lenTextPrefixMax = 3;

// file prefixes for different text files
extern const BYTE arrStrTextPrefix[AFLibIO::ttfN][lenTextPrefixMax];

// lengths of the file prefixes
extern const int arrLenTextPrefix[AFLibIO::ttfN];

// load string from resources, fill in the blanks
CString FormatStrings(HINSTANCE hInst, UINT ids, LPCTSTR* pStrings);

// throw message "expected ..."
void LocalThrowMessageExpected(UINT ids);

// load string from DLL resource, fill in the blanks
CString LocalAfxString(UINT ids, AFLIB_NINE_STRINGS);

// load string from DLL resource, show message
UINT LocalAfxMB(UINT ids, AFLIB_NINE_STRINGS);

// check is string starts with pattern; may ignore case
bool LocalStringStartsWith(LPCTSTR s, LPCTSTR pattern, bool bIgnoreCase);

// check is string ends with pattern; may ignore case
bool LocalStringEndsWith(LPCTSTR s, LPCTSTR pattern, bool bIgnoreCase);

// get version info from a buffer
VS_FIXEDFILEINFO LocalGetVersion(LPVOID buffer, LPCTSTR name);

// get tagged version info string from a buffer
CStringW LocalGetVersionData(LPVOID buffer, LPCWSTR tag, LPCTSTR name);

// timer IDs used through WM_TIMER; sometimes, no timer -- WM_TIMER is sent or posted
enum
{
    timerUpdate = 10001,
    timerStartEdit,
    timerCommitEdit,
    timerMouseUpdate,
    timerLoad,
    timerReallyCancel
};
}  // namespace AFLibPrivate
