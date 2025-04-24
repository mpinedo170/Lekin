#include "StdAfx.h"

#include "AFLibAppAF.h"

#include "AFLibBlockMBCounter.h"
#include "AFLibExcBreak.h"
#include "AFLibExcMsgBox.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibResource.h"
#include "AFLibStringBuffer.h"
#include "Local.h"

using AFLib::CAppAF;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibGui;
using namespace AFLibPrivate;

namespace {
const CString keySoftware = _T("Software");
}

IMPLEMENT_DYNAMIC(CAppAF, super)

CAppAF::CAppAF(LPCTSTR lpszAppName) : super(lpszAppName)
{
    m_pVersion = NULL;
    memset(&m_versionInfo, 0, sizeof(VS_FIXEDFILEINFO));
    m_pIntSess = NULL;
    m_hWndCon = NULL;
}

CAppAF::~CAppAF()
{  // m_pIntSess should be deleted in ExitInstance(); if it was not, we delete it here
    delete m_pIntSess;
    free(m_pVersion);
}

BEGIN_MESSAGE_MAP(CAppAF, super)
END_MESSAGE_MAP()

int CAppAF::ExitInstance()
{
    delete m_pIntSess;
    m_pIntSess = NULL;
    return super::ExitInstance();
}

CString CAppAF::GetProfilePath()
{
    ASSERT(!IsEmptyStr(m_pszRegistryKey));
    ASSERT(!IsEmptyStr(m_pszProfileName));

    return keySoftware + strBSlash + m_pszRegistryKey + strBSlash + m_pszProfileName;
}

CRegKey CAppAF::PrivateGetKey(HKEY hKeyRoot, LPCTSTR section, bool bWriteAccess)
{
    CString path = GetProfilePath();
    if (!IsEmptyStr(section)) path += strBSlash + section;
    return OpenRegSubKey(hKeyRoot, path, bWriteAccess);
}

void CAppAF::PrivateDeleteEntry(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry)
{
    try
    {
        if (IsEmptyStr(entry))
        {
            CRegKey key = PrivateGetKey(hKeyRoot, NULL, true);
            key.RecurseDeleteKey(section);
        }
        else
        {
            CRegKey key = PrivateGetKey(hKeyRoot, section, true);
            key.DeleteValue(entry);
        }
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }
}

CString CAppAF::PrivateGetString(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, LPCTSTR def)
{
    try
    {
        CRegKey key = PrivateGetKey(hKeyRoot, section, false);
        return GetRegValueStr(key, entry, def);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        return def;
    }
}

bool CAppAF::PrivateWriteString(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, LPCTSTR value)
{
    try
    {
        CRegKey key = PrivateGetKey(hKeyRoot, section, true);
        key.SetStringValue(entry, value);
        return true;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        return false;
    }
}

int CAppAF::PrivateGetInt(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, int def)
{
    try
    {
        CRegKey key = PrivateGetKey(hKeyRoot, section, false);
        return GetRegValueInt(key, entry, def);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        return def;
    }
}

bool CAppAF::PrivateWriteInt(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, int value)
{
    try
    {
        CRegKey key = PrivateGetKey(hKeyRoot, section, true);
        key.SetDWORDValue(entry, value);
        return true;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        return false;
    }
}

double CAppAF::PrivateGetDouble(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, double def)
{
    CString s = PrivateGetString(hKeyRoot, section, entry, strEmpty);
    double r = StrToDouble(s);
    return AFLibMath::IsNan(r) ? def : r;
}

bool CAppAF::PrivateWriteDouble(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, double value)
{
    return PrivateWriteString(hKeyRoot, section, entry, DoubleToStr(value, 12));
}

void CAppAF::PrivateMigrateSettings(HKEY hKeyRoot, LPCTSTR oldAppName, bool bKeepOld)
{
    CString pathNew = GetProfilePath();
    CString pathOldUp = keySoftware + strBSlash + m_pszRegistryKey;
    CString pathOld = pathOldUp + strBSlash + oldAppName;

    try
    {
        CRegKey key = OpenRegSubKey(hKeyRoot, pathNew, false);
        return;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    try
    {
        CRegKey key = OpenRegSubKey(hKeyRoot, pathOld, false);
        CopyRegKey(hKeyRoot, pathOld, hKeyRoot, pathNew);
        if (bKeepOld) return;

        key = OpenRegSubKey(hKeyRoot, pathOldUp, true);
        key.RecurseDeleteKey(oldAppName);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }
}

void CAppAF::MigrateSettings(LPCTSTR oldAppName, bool bKeepOld)
{
    PrivateMigrateSettings(HKEY_LOCAL_MACHINE, oldAppName, bKeepOld);
    PrivateMigrateSettings(HKEY_CURRENT_USER, oldAppName, bKeepOld);
}

CRegKey CAppAF::GetMachineKey(LPCTSTR section, bool bWriteAccess)
{
    return PrivateGetKey(HKEY_LOCAL_MACHINE, section, bWriteAccess);
}

CRegKey CAppAF::GetUserKey(LPCTSTR section, bool bWriteAccess)
{
    return PrivateGetKey(HKEY_CURRENT_USER, section, bWriteAccess);
}

CRect CAppAF::GetMainRect()
{
    if (m_pMainWnd == NULL || m_pMainWnd->IsIconic())
        return CRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

    CRect rect;
    m_pMainWnd->GetWindowRect(rect);
    return rect;
}

void CAppAF::CenterRect(CRect& rect)
{
    CRect rectMain = GetMainRect();
    CenterRectOnRect(rect, rectMain);
}

const VS_FIXEDFILEINFO& CAppAF::GetVersionInfo()
{
    LoadVersionData();
    return m_versionInfo;
}

void CAppAF::LoadVersionData()
{
    if (m_pVersion != NULL) return;

    HRSRC hRes = FindResource(NULL, LPCTSTR(DWORD(VS_VERSION_INFO)), RT_VERSION);
    if (hRes == NULL) ThrowMessage(LocalAfxString(IDS_NO_VERSION, m_pszAppName));

    HGLOBAL hMem = LoadResource(NULL, hRes);
    if (hMem == NULL) ThrowMessage(LocalAfxString(IDS_NO_VERSION, m_pszAppName));

    int length = SizeofResource(NULL, hRes);
    m_pVersion = malloc(length);
    memcpy(m_pVersion, LockResource(hMem), length);
    m_versionInfo = LocalGetVersion(m_pVersion, m_pszAppName);
}

CStringW CAppAF::GetVersionData(LPCWSTR tag)
{
    LoadVersionData();
    return LocalGetVersionData(m_pVersion, tag, m_pszAppName);
}

CStringA CAppAF::GetVersionData(LPCSTR tag)
{
    CStringW tagW(tag);
    return CStringA(GetVersionData(tagW));
}

CString CAppAF::GetStrVersionInfo(int count)
{
    GetVersionInfo();

    TStringBuffer format;
    for (int i = 0; i < count; ++i)
    {
        if (i > 0) format += chrDot;
        format += _T("%d");
    }

    CString s;
    s.Format(format, int(HIWORD(m_versionInfo.dwProductVersionMS)),
        int(LOWORD(m_versionInfo.dwProductVersionMS)), int(HIWORD(m_versionInfo.dwProductVersionLS)),
        int(LOWORD(m_versionInfo.dwProductVersionLS)));
    return s;
}

CInternetSession* CAppAF::GetInternetSession()
{
    if (m_pIntSess == NULL) m_pIntSess = new CInternetSession;
    return m_pIntSess;
}

bool CAppAF::PrivateOpenConsole(LPCTSTR conTitle)
{
    if (m_hWndCon != NULL) return true;
    if (!AllocConsole()) return false;

    TStringBuffer conTitle2;
    conTitle2 += conTitle;
    conTitle2.AppendUint(UINT(m_pMainWnd->m_hWnd));
    if (!SetConsoleTitle(conTitle2)) return false;

    if (!SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT))
        return false;
    if (!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT))
        return false;

    for (int i = 0; m_hWndCon == NULL; ++i)
    {
        if (i == 10) return false;
        Sleep(3);
        m_hWndCon = ::FindWindow(NULL, conTitle2);
    }

    ::ShowWindow(m_hWndCon, SW_HIDE);
#ifdef _DEBUG
    ::ShowWindow(m_hWndCon, SW_MINIMIZE);
#endif
    SetConsoleTitle(conTitle);
    return true;
}

bool CAppAF::OpenConsole(LPCTSTR conTitle)
{
    if (m_hWndCon != NULL) return true;
    FreeConsole();
    m_pMainWnd->SetWindowPos(&CWnd::wndTopMost, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE);
    bool bResult = PrivateOpenConsole(conTitle);

    m_pMainWnd->SetForegroundWindow();
    m_pMainWnd->SetWindowPos(&CWnd::wndNoTopMost, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE);
    return bResult;
}

bool CAppAF::CloseConsole()
{
    if (!FreeConsole())
    {
        m_hWndCon = NULL;
        return true;
    }

    if (!m_hWndCon) return true;
    ::ShowWindow(m_hWndCon, SW_MINIMIZE);

    Sleep(10);
    if (IsWindow(m_hWndCon))
    {
        ::ShowWindow(m_hWndCon, SW_HIDE);
        ::PostMessage(m_hWndCon, WM_CLOSE, 0, 0);
    }

    m_hWndCon = NULL;
    return true;
}

int CAppAF::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt)
{
    if (TBlockMBCounter::IsMsgBoxBlocked()) throw new CExcMsgBox(lpszPrompt, nType, nIDPrompt);
    return super::DoMessageBox(lpszPrompt, nType, nIDPrompt);
}
