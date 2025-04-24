#pragma once

#include "AFLibDefine.h"
#include "AFLibGlobal.h"

/////////////////////////////////////////////////////////////////////////////
// TLibEnum
// Helper class for loading Process Enumeration functions.
// The functions are different in Win95 and WinNT.

namespace AFLibPrivate {
class TLibEnum
{
private:
    DEFINE_COPY_AND_ASSIGN(TLibEnum);

    // instance handle for the main library
    HINSTANCE m_hInstLib;

    // instance handle for the secondary library
    HINSTANCE m_hInstLib2;

    // are we working under Win95?
    bool m_bWin95;

    // ToolHelp Function Pointers
    HANDLE(WINAPI* m_pfCreateToolhelp32Snapshot)(DWORD, DWORD);
    BOOL(WINAPI* m_pfProcess32First)(HANDLE, LPPROCESSENTRY32);
    BOOL(WINAPI* m_pfProcess32Next)(HANDLE, LPPROCESSENTRY32);

    // PSAPI Function Pointers
    BOOL(WINAPI* m_pfEnumProcesses)(LPDWORD, DWORD cb, LPDWORD);
    BOOL(WINAPI* m_pfEnumProcessModules)(HANDLE, HMODULE*, DWORD, LPDWORD);
    DWORD(WINAPI* m_pfGetModuleFileNameEx)(HANDLE, HMODULE, LPTSTR, DWORD);

    // VDMDBG Function Pointers
    INT(WINAPI* m_pfVDMEnumTaskWOWEx)(DWORD, TASKENUMPROCEX fp, LPARAM);

public:
    TLibEnum();
    ~TLibEnum();

    // open the library handle and fill function pointers
    bool InitLibrary();

    // main function
    bool EnumerateProcesses(AFLib::PROCENUMPROC lpProc, LPVOID lpParam);

    // one and only object
    static TLibEnum m_a;
};
}  // namespace AFLibPrivate
