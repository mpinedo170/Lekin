#include "StdAfx.h"

#include "LibEnum.h"

using namespace AFLib;
using AFLibPrivate::TLibEnum;

TLibEnum TLibEnum::m_a;

namespace {  // process enumeration structure
struct TEnumInfoStruct
{
    DWORD dwPID;
    PROCENUMPROC lpProc;
    LPVOID lpParam;
    bool bEnd;
};

BOOL WINAPI Enum16(
    DWORD dwThreadId, WORD hMod16, WORD hTask16, LPSTR pszModName, LPSTR pszFileName, LPVOID lpUserDefined)
{
    BOOL bRet = false;
    TEnumInfoStruct* psInfo = reinterpret_cast<TEnumInfoStruct*>(lpUserDefined);

#ifdef _UNICODE
    CString fileName(pszFileName);
    bRet = psInfo->lpProc(psInfo->dwPID, hTask16, fileName, psInfo->lpParam);
#else
    bRet = psInfo->lpProc(psInfo->dwPID, hTask16, pszFileName, psInfo->lpParam);
#endif

    if (!bRet) psInfo->bEnd = true;
    return !bRet;
}
}  // namespace

TLibEnum::TLibEnum()
{
    m_hInstLib = m_hInstLib2 = NULL;
    m_bWin95 = IsWin95();

    m_pfCreateToolhelp32Snapshot = NULL;
    m_pfProcess32First = NULL;
    m_pfProcess32Next = NULL;

    m_pfEnumProcesses = NULL;
    m_pfEnumProcessModules = NULL;
    m_pfGetModuleFileNameEx = NULL;

    m_pfVDMEnumTaskWOWEx = NULL;
}

TLibEnum::~TLibEnum()
{
    if (m_hInstLib != NULL) FreeLibrary(m_hInstLib);
    if (m_hInstLib2 != NULL) FreeLibrary(m_hInstLib2);
}

bool TLibEnum::InitLibrary()
{
    if (m_hInstLib != NULL) return true;

    if (m_bWin95)
    {
        m_hInstLib = LoadLibraryA("Kernel32.DLL");
        if (m_hInstLib == NULL) return false;

        // Get procedure addresses.
        // We are linking to these functions of Kernel32
        // explicitly, because otherwise a module using
        // this code would fail to load under Windows NT,
        // which does not have the Toolhelp32
        // functions in the Kernel32.

        m_pfCreateToolhelp32Snapshot = reinterpret_cast<HANDLE(WINAPI*)(DWORD, DWORD)>(
            GetProcAddress(m_hInstLib, "CreateToolhelp32Snapshot"));
        m_pfProcess32First = reinterpret_cast<BOOL(WINAPI*)(HANDLE, LPPROCESSENTRY32)>(
            GetProcAddress(m_hInstLib, "Process32First"));
        m_pfProcess32Next = reinterpret_cast<BOOL(WINAPI*)(HANDLE, LPPROCESSENTRY32)>(
            GetProcAddress(m_hInstLib, "Process32Next"));

        if (m_pfProcess32Next == NULL || m_pfProcess32First == NULL || m_pfCreateToolhelp32Snapshot == NULL)
        {
            FreeLibrary(m_hInstLib);
            m_hInstLib = NULL;
            return false;
        }
    }
    else
    {  // Load library and get the procedures explicitly. We do
        // this so that we don't have to worry about modules using
        // this code failing to load under Windows 95, because
        // it can't resolve references to the PSAPI.DLL.
        m_hInstLib = LoadLibrary(_T("PSAPI.DLL"));
        if (m_hInstLib == NULL) return false;

        m_hInstLib2 = LoadLibrary(_T("VDMDBG.DLL"));
        if (m_hInstLib2 == NULL)
        {
            FreeLibrary(m_hInstLib);
            m_hInstLib = NULL;
            return false;
        }

        // Get procedure addresses.
        m_pfEnumProcesses = reinterpret_cast<BOOL(WINAPI*)(LPDWORD, DWORD, LPDWORD)>(
            GetProcAddress(m_hInstLib, "EnumProcesses"));
        m_pfEnumProcessModules = reinterpret_cast<BOOL(WINAPI*)(HANDLE, HMODULE*, DWORD, LPDWORD)>(
            GetProcAddress(m_hInstLib, "EnumProcessModules"));
        m_pfVDMEnumTaskWOWEx = reinterpret_cast<INT(WINAPI*)(DWORD, TASKENUMPROCEX, LPARAM)>(
            GetProcAddress(m_hInstLib2, "VDMEnumTaskWOWEx"));

#ifdef _UNICODE
        static const LPCSTR procName = "GetModuleFileNameExW";
#else
        static const LPCSTR procName = "GetModuleFileNameExA";
#endif

        m_pfGetModuleFileNameEx =
            (DWORD(WINAPI*)(HANDLE, HMODULE, LPTSTR, DWORD))GetProcAddress(m_hInstLib, procName);

        if (m_pfEnumProcesses == NULL || m_pfEnumProcessModules == NULL || m_pfGetModuleFileNameEx == NULL ||
            m_pfVDMEnumTaskWOWEx == NULL)
        {
            FreeLibrary(m_hInstLib);
            m_hInstLib = NULL;
            FreeLibrary(m_hInstLib2);
            m_hInstLib2 = NULL;
            return false;
        }
    }

    return true;
}

// The EnumerateProcesses function takes a pointer to a callback function
// that will be called once per process in the system providing
// process EXE filename and process ID.
// Callback function definition:
// BOOL CALLBACK Proc(DWORD dw, LPCSTR lpstr, LPARAM lParam);
//
// lpProc -- Address of callback routine.
//
// lParam -- A user-defined LPARAM value to be passed to
//           the callback routine.

bool TLibEnum::EnumerateProcesses(PROCENUMPROC lpProc, LPVOID lpParam)
{
    if (!InitLibrary()) return false;

    if (m_bWin95)
    {  // If Windows 95:
        // Get a handle to a Toolhelp snapshot of the systems
        // processes.
        HANDLE hSnapShot = m_pfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapShot == INVALID_HANDLE_VALUE) return false;

        // Get the first process' information.
        PROCESSENTRY32 procentry;
        procentry.dwSize = sizeof(PROCESSENTRY32);
        BOOL bFlag = m_pfProcess32First(hSnapShot, &procentry);

        // While there are processes, keep looping.
        while (bFlag)
        {  // Call the enum func with the filename and ProcID.
            if (lpProc(procentry.th32ProcessID, 0, procentry.szExeFile, lpParam))
            {
                procentry.dwSize = sizeof(PROCESSENTRY32);
                bFlag = m_pfProcess32Next(hSnapShot, &procentry);
            }
            else
                bFlag = false;
        }
    }
    else
    {  // Call the PSAPI function EnumProcesses to get all of the
        // ProcID's currently in the system.
        // NOTE: In the documentation, the third parameter of
        // EnumProcesses is named cbNeeded, which implies that you
        // can call the function once to find out how much space to
        // allocate for a buffer and again to fill the buffer.
        // This is not the case. The cbNeeded parameter returns
        // the number of PIDs returned, so if your buffer size is
        // zero cbNeeded returns zero.
        // NOTE: The "HeapAlloc" loop here ensures that we
        // actually allocate a buffer large enough for all the
        // PIDs in the system.

        DWORD dwSize = 0, dwIndex = 0;
        DWORD dwSize2 = 256 * sizeof(DWORD);
        LPDWORD lpdwPIDs = NULL;
        TCHAR szFileName[MAX_PATH];

        while (true)
        {
            lpdwPIDs = reinterpret_cast<LPDWORD>(HeapAlloc(GetProcessHeap(), 0, dwSize2));
            if (lpdwPIDs == NULL) return false;

            if (!m_pfEnumProcesses(lpdwPIDs, dwSize2, &dwSize))
            {
                HeapFree(GetProcessHeap(), 0, lpdwPIDs);
                return false;
            }

            if (dwSize < dwSize2) break;
            HeapFree(GetProcessHeap(), 0, lpdwPIDs);
            dwSize2 *= 2;
        }

        // How many ProcID's did we get?
        dwSize /= sizeof(DWORD);

        // Loop through each ProcID.
        for (dwIndex = 0; dwIndex < dwSize; ++dwIndex)
        {
            szFileName[0] = 0;
            // Open the process (if we can... security does not
            // permit every process in the system).
            HANDLE hProcess =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, lpdwPIDs[dwIndex]);

            if (hProcess != NULL)
            {  // Here we call EnumProcessModules to get only the
                // first module in the process. This is important,
                // because this will be the .EXE module for which we
                // will retrieve the full path name in a second.

                HMODULE hMod = NULL;
                if (m_pfEnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwSize2))
                {  // Get Full pathname:
                    if (!m_pfGetModuleFileNameEx(hProcess, hMod, szFileName, sizeof(szFileName)))
                        szFileName[0] = 0;
                }
                CloseHandle(hProcess);
            }

            // Regardless of OpenProcess success or failure, we
            // still call the enum func with the ProcID.
            if (!lpProc(lpdwPIDs[dwIndex], 0, szFileName, lpParam)) break;

            // Did we just bump into an NTVDM?
            if (StringEndsWithIC(szFileName, _T("NTVDM.EXE")))
            {  // Fill in some info for the 16-bit enum proc.
                TEnumInfoStruct sInfo;
                sInfo.dwPID = lpdwPIDs[dwIndex];
                sInfo.lpProc = lpProc;
                sInfo.lpParam = lpParam;
                sInfo.bEnd = false;

                // Enum the 16-bit stuff.
                m_pfVDMEnumTaskWOWEx(lpdwPIDs[dwIndex], TASKENUMPROCEX(Enum16), LPARAM(&sInfo));

                // Did our main enum func say quit?
                if (sInfo.bEnd) break;
            }
        }

        HeapFree(GetProcessHeap(), 0, lpdwPIDs);
    }

    return true;
}

namespace AFLib {
BOOL WINAPI EnumerateProcesses(PROCENUMPROC lpProc, LPVOID lpParam)
{
    return TLibEnum::m_a.EnumerateProcesses(lpProc, lpParam);
}
}  // namespace AFLib
