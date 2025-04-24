#include "StdAfx.h"

#include "AppAsm.h"

#include "../ControlSum.h"
#include "Job.h"
#include "Misc.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////
// file/path

/////////////////////////////////////////////////
// multiprocessing/multithreading

static HWND hWndFound;
static CString mainframe;

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM)
{
    CWnd* pWnd = CWnd::FromHandle(hwnd);
    CString title;
    pWnd->GetWindowText(title);
    int index = title.Find(_T(" - ["));
    if (index != -1) title = title.Left(index);
    if (title == mainframe)
    {
        hWndFound = hwnd;
        return false;
    }
    return true;
}

bool CheckOtherInstance()
{
    mainframe.LoadString(IDR_MAINFRAME);
    hWndFound = NULL;
    EnumWindows(EnumWindowsProc, 0);
    if (hWndFound)
    {
        ::SetForegroundWindow(hWndFound);
        return true;
    }
    return false;
}

bool MakeProcess(LPCTSTR command, LPCTSTR dir, int mpFlags, HANDLE& hProcess, HANDLE& hThread)
{
    STARTUPINFO strInfo = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION proInfo;

    DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS;
    GetStartupInfo(&strInfo);
    strInfo.dwFlags = 0;

    if (mpFlags & mpSpecial)
    {
        strInfo.dwX = 0xABFE;
        strInfo.dwY = arrJob.GetSize();
        strInfo.dwXSize = arrWorkcenter.GetSize();
        strInfo.dwYSize = GetTotalMchMax();
    }

    if (mpFlags & mpDetached) dwCreationFlags |= CREATE_NEW_CONSOLE;

    if (mpFlags & mpHide)
    {
        strInfo.dwFlags |= STARTF_USESHOWWINDOW;
        strInfo.wShowWindow = SW_HIDE;
    }

    hProcess = hThread = NULL;
    if (!CreateProcess(
            NULL, LPTSTR(command), NULL, NULL, true, dwCreationFlags, NULL, dir, &strInfo, &proInfo))
    {
        AfxMB(IDP_CANT_START, command);
        return false;
    }

    hProcess = proInfo.hProcess;
    hThread = proInfo.hThread;
    return true;
}

/////////////////////////////////////////////////
// misc

bool CheckHack()
{
    bool bOk = true;

#ifndef _DEBUG
    CString exe = GetExeFullPath();
    LPWORD pBuffer = NULL;

    try
    {
        CFile F(exe, CFile::modeRead | CFile::shareDenyNone);
        ControlSum(F, pBuffer, NULL, false);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        bOk = false;
    }
    delete pBuffer;
#endif

    return bOk;
}

CString GetVersionInfo(int count)
{
    HRSRC hRes = FindResource(NULL, LPCTSTR(DWORD(VS_VERSION_INFO)), RT_VERSION);
    if (hRes == NULL) exit(255);

    HGLOBAL hMem = LoadResource(NULL, hRes);
    if (hMem == NULL) exit(255);

    VS_FIXEDFILEINFO* buf = NULL;
    UINT bufSize = 0;
    char lpSubBlock[] = {chrBSlash, 0};

    if (!VerQueryValue(LPVOID(hMem), lpSubBlock, (LPVOID*)(&buf), &bufSize)) exit(255);

    CString format;
    for (int i = 0; i < count; ++i)
    {
        if (i > 0) format += strDot;
        format += _T("%d");
    }

    CString s;
    s.Format(format, int(HIWORD(buf->dwProductVersionMS)), int(LOWORD(buf->dwProductVersionMS)),
        int(HIWORD(buf->dwProductVersionLS)), int(LOWORD(buf->dwProductVersionLS)));
    return s;
}

void LekinBeep(bool bOk)
{
    MessageBeep(bOk ? MB_OK : MB_ICONEXCLAMATION);
}
