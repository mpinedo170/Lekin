#pragma once

#define INCLUDE_AFLIB
#ifndef INCLUDE_AFLIB_GUI
#define NO_AFLIB_ATL
#endif
#define USE_AFLIB_NAMESPACES
#include "AFLib.h"

const int MAX_ID = 40;

int StrTailToInt(LPCTSTR str);

CString NextToken(LPCTSTR& s, TCHAR delim);
CString GetToken(LPCTSTR s, int pos, TCHAR delim);

void AlterName(CString& name, bool bReset = false);

class TTokenSet : public CS2SMap
{
public:
    CString m_name;

    TTokenSet(CArchive& ar);
};

bool ReadNext(CArchive& ar, CString& token, CString& data);

void WriteToken(CArchive& ar, int indent, LPCTSTR token, LPCTSTR data);
void WriteToken(CArchive& ar, int indent, LPCTSTR token, int data);
void NextLn(CArchive& ar);
