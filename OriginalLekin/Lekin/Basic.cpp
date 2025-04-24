#define _WIN32_WINNT 0x0502  // Win NT version (XP SP1)
#define _WIN32_IE 0x0601     // Internet Explorer 6.0 SP1

#include <afxtempl.h>
#include <afxwin.h>

#include "Basic.h"

CString NextToken(LPCTSTR& s, TCHAR delim)
{
    if (s == NULL) return strEmpty;
    LPCTSTR ss = _tcschr(s, delim);

    if (!ss)
    {
        CString temp(s);
        s = NULL;
        return temp;
    }
    else
    {
        CString temp(s, ss - s);
        while (true)
        {
            ++ss;
            if (*ss != delim || delim != ' ') break;
        }
        s = ss;
        return temp;
    }
}

CString GetToken(LPCTSTR s, int pos, TCHAR delim)
{
    CString result;
    for (int i = 0; i <= pos; ++i) result = NextToken(s, delim);
    return result;
}

void AlterName(CString& name, bool bReset)
{
    int k;
    int slen = name.GetLength();
    int stop = max(0, slen - 8);
    CString add;

    for (k = slen; k > stop; --k)
        if (!isdigit(name[k - 1])) break;

    if (k == slen)
    {
        if (slen == 0 || !isalpha(name[slen - 1]) || toupper(name[slen - 1]) == 'Z')
            add = _T(" - 01");
        else
        {
            --k;
            add = CString(TCHAR(name[slen - 1] + 1));
        }
    }
    else
    {
        int p = 1;
        if (!bReset)
        {
            _stscanf_s(LPCTSTR(name) + k, _T("%d"), &p);
            ++p;
        }

        add.Format(_T("%0*d"), slen - k, p);
    }

    k = min(k, MAX_ID - add.GetLength());
    CString temp = CString(name, k) + add;
    name = temp;
}

int StrTailToInt(LPCTSTR str)
{
    LPCTSTR ss = str + _tcslen(str) - 1;
    while (ss >= str && isdigit(*ss)) --ss;
    return StrToInt(ss + 1);
}

// Archive functions

static int ReadByte(CArchive& ar)
{
    BYTE c;
    return ar.Read(&c, 1) < 1 ? EOF : int(c);
}

bool ReadNext(CArchive& ar, CString& token, CString& data)
{
    token.Empty();
    data.Empty();

    int c = chrSpace;
    while (IsSpace(c)) c = ReadByte(ar);

    while (!IsSpace(c) && c != chrColon && c != EOF)
    {
        token += TCHAR(c);
        c = ReadByte(ar);
    }
    if (token.IsEmpty()) return true;
    if (c == chrColon) c = chrSpace;

    while (c == chrSpace || c == chrTab) c = ReadByte(ar);

    while (c != chrCarRet && c != chrNewLine && c != EOF)
    {
        data += TCHAR(c);
        c = ReadByte(ar);
    }

    while (!data.IsEmpty() && data[data.GetLength() - 1] <= chrSpace) data = data.Left(data.GetLength() - 1);
    return false;
}

void WriteToken(CArchive& ar, int indent, LPCTSTR token, LPCTSTR data)
{
    static const CStringA space(' ', 20);
    ar.Write(LPCSTR(space), indent);

    CStringA s = token;
    ar.Write(LPCSTR(s), s.GetLength());
    ar << ':';

    if (data && data[0])
    {
        int count = 19 - s.GetLength();
        ar.Write(space, max(0, count));
        s = data;
        ar.Write(LPCSTR(s), s.GetLength());
    }
    NextLn(ar);
}

void WriteToken(CArchive& ar, int indent, LPCTSTR token, int data)
{
    WriteToken(ar, indent, token, IntToStr(data));
}

void NextLn(CArchive& ar)
{
    ar.Write(strEolA, 2);
}

TTokenSet::TTokenSet(CArchive& ar)
{
    m_name = ar.GetFile()->GetFileName();
}
