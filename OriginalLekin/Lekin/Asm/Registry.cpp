#include "StdAfx.h"
#include APP_H

CString GetWinString(bool NT)
{ CString kk;
  if (NT && !IsWin95()) kk = " NT";
  CString s; AfxFormatString1(s, IDS_WINREG, kk);
  return s;
}

HKEY OpenKey(LPCTSTR s)
{ HKEY hKey = NULL;
  return RegOpenKey(HKEY_LOCAL_MACHINE, s, &hKey)==ERROR_SUCCESS ?
    hKey : NULL;
}

HKEY OpenWinKey()
{ return OpenKey(GetWinString(true)); }

CString GetRegValue(HKEY hKey, LPCTSTR sec, LPCTSTR def)
{ DWORD type = 0;
  DWORD size = 0;

  if (RegQueryValueEx(hKey, sec, NULL,
    &type, NULL, &size) != ERROR_SUCCESS)
      return def;
  if (type != REG_SZ) return def;

  CString s;
  RegQueryValueEx(hKey, sec, NULL,
    NULL, LPBYTE(s.GetBuffer(size+1)), &size);
  s.ReleaseBuffer();
  return s;
}

bool SetRegValue(HKEY hKey, LPCTSTR sec, LPCTSTR value)
{ return RegSetValueEx(hKey, sec, 0, REG_SZ,
    PBYTE(value), _tcslen(value)+1) == ERROR_SUCCESS;
}

static LPCTSTR keyUser = _T("RegisteredOwner");
static LPCTSTR keyComp = _T("RegisteredOrganization");

CString GetOwnerInfo(bool bCompany)
{ HKEY hKey = OpenWinKey();
  CString info = GetRegValue(hKey, bCompany ? keyComp : keyUser, _T("Columbia"));
  RegCloseKey(hKey);
  return info;
}
