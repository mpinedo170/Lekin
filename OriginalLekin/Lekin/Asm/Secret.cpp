#include "StdAfx.h"
#include APP_H
#include "DlgKey.h"
#include "Registry.h"
#include "..\Scramble.h"

enum { pDay, pMonth, pYear, pJob, pWkc, pMch, pMul, pBad1, pBad2 };

#define keyKey _T("Key")

const int SCRAMBLE_MASK = 0xAF;

static char S(TCHAR c)
{ return c==0 || c==SCRAMBLE_MASK ? c : c ^ SCRAMBLE_MASK; }

static char keySecret[] =
{ S('J'),S('a'),S('v'),S('a'),S(' '),S('V'),S('M'),0 };

static char fileSecret1[] =
{ S('\\'),S('H'),S('e'),S('l'),S('p'),S('\\'),
  S('Y'),S('u'),S('r'),S('i'),S('e'),S('.'),
  S('h'),S('l'),S('p'),0
};

static char fileSecret2[] =
{ S('\\'),S('s'),S('y'),S('s'),S('t'),S('e'),S('m'),S('\\'),
  S('T'),S('H'),S('R'),S('E'),S('E'),S('.'),
  S('V'),S('B'),S('X'),0
};

static char filePass[] =
{ S('A'),S(':'),S('\\'),
  S('P'),S('a'),S('s'),S('s'),S('w'),S('o'),S('r'),S('d'),0
};

static char math[] =
{ S('5'),S('0'),S(' '),S('2'),S('0'),S(' '),S('1'),S('0'),S('0'),0 };

static char myself[] =
{ S('A'),S('s'),S('m'),S('.'),S('e'),S('x'),S('e'),0 };

static char ipPolice[] =
{ S('1'),S('2'),S('8'),S('.'),S('5'),S('9'),S('.'),
  S('5'),S('3'),S('.'),S('2'),S('4'),S('6'), 0
};

static CString Unscramble(LPCSTR key)
{ CString s;
  for (; *key != 0; key++)
    s += TCHAR(S(*key));
  return s;
}

static HKEY OpenSecretKey()
{ CString s = GetWinString(true);

  for (int i=0; i<2; i++)
  { int k = s.ReverseFind('\\');
    if (k == -1) return NULL;
    s = s.Left(k);
  }

  return OpenKey(s + _T("\\") + Unscramble(keySecret));
}

static CString GetSecretFile(bool bSecond)
{ TCHAR s[256];
  if (!GetWindowsDirectory(s, 256)) return strEmpty;
  return CString(s) + Unscramble(bSecond ? fileSecret2 : fileSecret1);
}

static CString digits(int* pass, int count=4)
{ CString s;
  for (int i=0; ; i++)
  { s += IntToStr(pass[i]);
    if (i >= count-1) break;
    s += ",";
  }
  return s;
}

static bool undigits(LPCTSTR str, int* pass, int count=4)
{ for (int i=0; i<count; i++)
  { CString s = NextToken(str, ',');
    pass[i] = StrToInt(s);
  }
  return true;
}

//**************************

static bool BullshitCheck(int* pass)
{ for (int i=0; i<MatSize; i++)
    if (pass[i] <= 0) return false;

  return
    pass[pDay]<=31 && pass[pMonth]<=12 && pass[pYear]<=2003 &&
    pass[pJob]<=1000 && pass[pWkc]<=1000 && pass[pMch]<=1000 &&
    pass[pMul]<=2 && pass[pBad1]<2880 && pass[pBad2]<2880 && pass[9]<=1;
}

//**************************

static bool WriteSecret(int* pass)
{ int scram[MatSize];
  Scramble(pass, scram, true);

  HKEY hKey = OpenSecretKey();
  if (!hKey) return false;
  bool bOk = SetRegValue(hKey, NULL, digits(scram));
  RegCloseKey(hKey);
  if (!bOk) return false;

  TRY
  { CString name = GetSecretFile(false);
    if (!CreateHiddenFile(name)) return false;
    CFile F1(name, CFile::modeReadWrite | CFile::typeBinary);
    F1.Write(scram+4, 3 * sizeof(int));

    name = GetSecretFile(true);
    if (!CreateHiddenFile(name)) return false;
    CFile F2(name, CFile::modeReadWrite | CFile::typeBinary);
    F2.Write(scram+7, (MatSize-7) * sizeof(int));
  }
  CATCH(CException, e)
  { bOk = false; }
  END_CATCH

  return bOk;
}

static bool ReadSecret(int* pass)
{ int scram[MatSize];
  CString s;

  HKEY hKey = OpenSecretKey();
  if (!hKey) return false;
  s = GetRegValue(hKey, NULL);
  RegCloseKey(hKey);
  if (!undigits(s, scram)) return false;

  TRY
  { CString name = GetSecretFile(false);
    CFile F1(name, CFile::modeRead | CFile::typeBinary);
    F1.Read(scram+4, 3 * sizeof(int));

    name = GetSecretFile(true);
    CFile F2(name, CFile::modeRead | CFile::typeBinary);
    F2.Read(scram+7, (MatSize-7) * sizeof(int));
  }
  CATCH(CException, e)
  { return false; }
  END_CATCH

  if (!Unscramble(pass, scram, true)) return false;
  return BullshitCheck(pass);
}

//**************************

static bool TestSector(int sector, bool bOk)
{ char buf[512];
  if (sector > 2880) return false;
  return ReadSector(buf, sector) == bOk;
}

static int GetMilli()
{ SYSTEMTIME time; GetSystemTime(&time);
  return time.wMilliseconds + time.wSecond*1000 +
    time.wMinute*60*1000 + time.wHour*60*60*1000;
}


static bool CheckDiskette(bool bEmptyOk, int* pass)
{ int scram[MatSize];
  UINT sz = MatSize * sizeof(int);
  char lic[65];

  TRY
  { CFile F(Unscramble(filePass), CFile::modeRead | CFile::typeBinary);
    if (F.Read(scram, sz) < sz)
      THROW(new CFileException());
    UINT r = F.Read(lic,64);
    lic[r] = 0;
  }
  CATCH(CException, e)
  { memset(pass, 0, sz); return bEmptyOk; }
  END_CATCH

  if (!Unscramble(pass, scram, false)) return false;
  if (!BullshitCheck(pass)) return false;
  if (!OpenFloppy()) return false;

  int sect1 = pass[pBad1];
  int sect2 = 0;
  int sect3 = pass[pBad2];

  bool bOk =
    TestSector(sect1, false) &&
    TestSector(sect2, true) &&
    TestSector(sect3, false);

  CloseFloppy();
  if (bOk) License = lic;
  return bOk;
}

//**************************

static CString SnitchInfo, SnitchInfo2;
static CWinThread* pThreadSnitch;
static CSocket* pSock;

static UINT GetSnitchInfo(LPVOID)
{ static char s[256];
  if (gethostname(s, 256)) return thrError;
  HOSTENT* host = gethostbyname(s);
  if (host == NULL) return thrError;

  HKEY hKey = OpenWinKey();
  if (hKey == NULL) return thrError;
  SnitchInfo = CString(strNewLine + CString(host->h_name) + strNewLine +
    CString(inet_ntoa(*(struct in_addr*)(host->h_addr_list[0]))) + strNewLine +
    CTime::GetCurrentTime().Format(_T("%B %d, %Y\n")) +
    GetOwnerInfo(false) + strNewLine +
    GetOwnerInfo(true) + strNewLine);
  RegCloseKey(hKey);
  return thrOK;
}

static UINT PutSnitchInfo(LPVOID)
{ if (SnitchInfo2.IsEmpty()) return thrError;
  pSock->SendTo(LPCTSTR(SnitchInfo2), SnitchInfo2.GetLength(),
    0x1ABF, Unscramble(ipPolice));
  return thrOK;
}

static void StartSnitch()
{ if (!AfxSocketInit()) return;
  pThreadSnitch = BeginThread(GetSnitchInfo, NULL);
}

static void SendSnitch(LPCTSTR version)
{ EndThread(pThreadSnitch, 5000);
  if (pSock)
  { delete pSock; pSock = NULL; return; }

  if (SnitchInfo.IsEmpty()) return;
  SnitchInfo2 = version + SnitchInfo;
  SnitchInfo.Empty();

  pSock = new CSocket;
  if (!pSock->Create(0, SOCK_DGRAM))
  { delete pSock; pSock = NULL; return; }
  pThreadSnitch = BeginThread(PutSnitchInfo, NULL);
}

//**************************

bool CheckSecret()
{ StartSnitch();
  CString s = Unscramble(math);
  _stscanf(s, _T("%d %d %d"), &MaxJob, &MaxWkc, &MaxMch);

  int pass[MatSize];
  if (!ReadSecret(pass)) return false;

  CTime expiry(pass[pYear], pass[pMonth], pass[pDay], 0, 0, 0);
  CTime current = CTime::GetCurrentTime();
  if (current > expiry)
  { DeleteSecret();
    AfxMB(IDP_EXPIRED);
    return false;
  }

  bool bMulti = pass[pMul]==2;
  if (bMulti)
    License = theApp.GetMachineString(secVit, keyLicense, _T("Demo"));
  else if (!CheckDiskette(false, pass))
  { int key = theApp.GetProfileInt(secSettings, keyKey, 0);
    if (key == 0)
    { CDlgKey dlg; dlg.DoModal();
      switch (dlg.m_Key)
      { case 2:
          DeleteSecret();
          return false;
        case 1:
          theApp.WriteProfileInt(secSettings, keyKey, 1);
          return false;
      }
    }
    if (!CheckDiskette(false, pass)) return false;
  }

  MaxJob = pass[pJob]; MaxWkc = pass[pWkc]; MaxMch = pass[pMch];
  return true;
}

bool DeleteSecret()
{ bool bOk = true;

  HKEY hKey = OpenSecretKey();
  if (hKey)
  { bOk &= (RegDeleteValue(hKey, NULL) == ERROR_SUCCESS);
    RegCloseKey(hKey);
  }
  else bOk = false;

  bOk &= (DeleteFile(GetSecretFile(false)) != 0);
  bOk &= (DeleteFile(GetSecretFile(true)) != 0);
  return bOk;
}

int CreateSecret()
{ StartSnitch();

  int okLevel = 0;
  int pass[MatSize];

  if (!CheckDiskette(true, pass))
  { AfxMB(IDP_PIRATE);
    License = "Pirate";
    okLevel = 0;
  }
  else
  { if (pass[0] != 0)
    { if (WriteSecret(pass))
        okLevel = 2 + pass[pMul];
      else
      { AfxMB(IDP_NO_ADMIN);
        License = "Error";
        okLevel = 1;
      }
    }
    else okLevel = 2;
  }

  SendSnitch(License);
  StopSnitch(); return okLevel;
}

void StopSnitch()
{ EndThread(pThreadSnitch, 5000); }
