#include "StdAfx.h"

#include "AppAsm.h"

bool equals(LPCTSTR str1, LPCTSTR str2)
{
    return _tcsicmp(str1, str2) == 0;
}

void CheckInt(int val, int mn, int mx)
{
    if (val >= mn && val <= mx) return;
    switch (mn)
    {
        case 0:
            ThrowMessage(IDP_NONNEGATIVE);
            break;
        case 1:
            ThrowMessage(IDP_POSITIVE);
            break;
        default:
            ThrowMessage(IDP_BETWEEN, IntToStr(mn), IntToStr(mx));
    }
}

void CheckID(LPCTSTR s, UINT ids2)
{
    UINT ids = 0;
    int len = _tcslen(s);

    if (len == 0)
        ids = IDP_EMPTY_ID;
    else if (len > MAX_ID || IsSpace(s[0]) || IsSpace(s[len - 1]) ||
             _tcspbrk(s, _T(";") /*_tcspbrk(s, _T(";.")*/) != NULL)
        ids = IDP_BAD_ID;
    else
        return;

    CString s2(MAKEINTRESOURCE(ids2));
    ThrowMessage(ids, s2);
}

void CheckStatus(LPCTSTR s)
{
    if (_tcslen(s) != 1 || !IsAlpha(s[0])) ThrowMessage(IDP_BAD_STATUS);
}

//**********************************

void ThrowNotExist(LPCTSTR name, UINT ids2)
{
    CString s(MAKEINTRESOURCE(ids2));
    ThrowMessage(IDP_NOT_EXIST, s, name);
}

void ThrowBadOp(LPCTSTR name, UINT ids2)
{
    CString s(MAKEINTRESOURCE(ids2));
    ThrowMessage(IDP_BAD_OP, s, name);
}

void ThrowDupID(LPCTSTR s, UINT ids2)
{
    CString s2(MAKEINTRESOURCE(ids2));
    ThrowMessage(IDP_DUP_ID, s, s2);
}

void ThrowTooMany(int mx, UINT ids2)
{
    CString s2(MAKEINTRESOURCE(ids2));
    ThrowMessage(IDP_TOO_MANY, IntToStr(mx), s2);
}

//**********************************

void ReportDupID(CDataExchange* pDX, UINT idc, UINT ids2)
{
    CString s;
    pDX->m_pDlgWnd->GetDlgItemText(idc, s);

    try
    {
        ThrowDupID(s, ids2);
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
    }

    pDX->PrepareEditCtrl(idc);
    pDX->Fail();
}

void ReportTooMany(int mx, UINT ids2)
{
    try
    {
        ThrowTooMany(mx, ids2);
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
    }
}
