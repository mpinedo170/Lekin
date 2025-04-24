#pragma once

#include "AFLibDefine.h"

namespace ATL {
class CRegKey;
}

class CInternetSession;

/////////////////////////////////////////////////////////////////////////////
// CAppAF
// Application class with extended registry get/set functions,
//   as well as some others

namespace AFLib {
class AFLIB CAppAF : public CWinApp
{
private:
    typedef CWinApp super;
    DEFINE_COPY_AND_ASSIGN(CAppAF);

    LPVOID m_pVersion;               // complete version info
    VS_FIXEDFILEINFO m_versionInfo;  // fixed version info
    CInternetSession* m_pIntSess;    // internet session used by the application
    HWND m_hWndCon;                  // console window

    // fill m_pVersion and m_versionInfo
    void LoadVersionData();

    // for the following private functions,
    // hKeyRoot is either HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER

    // create a key for the section; if section is NULL or "", get parent key
    ATL::CRegKey PrivateGetKey(HKEY hKeyRoot, LPCTSTR section, bool bWriteAccess);

    // delete entry; if entry is NULL or "", delete entire section
    void PrivateDeleteEntry(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry);

    // get string value from registry
    CString PrivateGetString(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, LPCTSTR def);

    // write string value into registry
    bool PrivateWriteString(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, LPCTSTR value);

    // get integer value from registry
    int PrivateGetInt(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, int def);

    // write integer value into registry
    bool PrivateWriteInt(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, int value);

    // get double value from registry
    double PrivateGetDouble(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, double def);

    // write double value into registry
    bool PrivateWriteDouble(HKEY hKeyRoot, LPCTSTR section, LPCTSTR entry, double value);

    // helper for MigrateSettings()
    void PrivateMigrateSettings(HKEY hKeyRoot, LPCTSTR oldAppName, bool bKeepOld);

    // helper for OpenConsole()
    bool PrivateOpenConsole(LPCTSTR conTitle);

public:
    explicit CAppAF(LPCTSTR lpszAppName = NULL);
    ~CAppAF();

    // get registry path for storing application-related settings
    virtual CString GetProfilePath();

    // get registry key under HKEY_LOCAL_MACHINE for the section
    virtual ATL::CRegKey GetMachineKey(LPCTSTR section, bool bWriteAccess);

    // get registry key under HKEY_CURRENT_USER for the section
    virtual ATL::CRegKey GetUserKey(LPCTSTR section, bool bWriteAccess);

    // delete entry under HKEY_LOCAL_MACHINE
    virtual void DeleteMachineEntry(LPCTSTR section, LPCTSTR entry)
    {
        PrivateDeleteEntry(HKEY_LOCAL_MACHINE, section, entry);
    }

    // delete entry under HKEY_CURRENT_USER
    virtual void DeleteUserEntry(LPCTSTR section, LPCTSTR entry)
    {
        PrivateDeleteEntry(HKEY_CURRENT_USER, section, entry);
    }

    // get string value from registry under HKEY_LOCAL_MACHINE
    virtual CString GetMachineString(LPCTSTR section, LPCTSTR entry, LPCTSTR def = NULL)
    {
        return PrivateGetString(HKEY_LOCAL_MACHINE, section, entry, def);
    }

    // get string value from registry under HKEY_CURRENT_USER
    virtual CString GetUserString(LPCTSTR section, LPCTSTR entry, LPCTSTR def = NULL)
    {
        return PrivateGetString(HKEY_CURRENT_USER, section, entry, def);
    }

    // set string value in registry under HKEY_LOCAL_MACHINE
    virtual bool WriteMachineString(LPCTSTR section, LPCTSTR entry, LPCTSTR value)
    {
        return PrivateWriteString(HKEY_LOCAL_MACHINE, section, entry, value);
    }

    // set string value in registry under HKEY_CURRENT_USER
    virtual bool WriteUserString(LPCTSTR section, LPCTSTR entry, LPCTSTR value)
    {
        return PrivateWriteString(HKEY_CURRENT_USER, section, entry, value);
    }

    // get integer value from registry under HKEY_LOCAL_MACHINE
    virtual int GetMachineInt(LPCTSTR section, LPCTSTR entry, int def)
    {
        return PrivateGetInt(HKEY_LOCAL_MACHINE, section, entry, def);
    }

    // get integer value from registry under HKEY_CURRENT_USER
    virtual int GetUserInt(LPCTSTR section, LPCTSTR entry, int def)
    {
        return PrivateGetInt(HKEY_CURRENT_USER, section, entry, def);
    }

    // set integer value in registry under HKEY_LOCAL_MACHINE
    virtual bool WriteMachineInt(LPCTSTR section, LPCTSTR entry, int value)
    {
        return PrivateWriteInt(HKEY_LOCAL_MACHINE, section, entry, value);
    }

    // set integer value in registry under HKEY_CURRENT_USER
    virtual bool WriteUserInt(LPCTSTR section, LPCTSTR entry, int value)
    {
        return PrivateWriteInt(HKEY_CURRENT_USER, section, entry, value);
    }

    // get boolean value from registry under HKEY_LOCAL_MACHINE
    virtual bool GetMachineBool(LPCTSTR section, LPCTSTR entry, bool def)
    {
        return GetMachineInt(section, entry, def) != 0;
    }

    // get boolean value from registry under HKEY_CURRENT_USER
    virtual bool GetUserBool(LPCTSTR section, LPCTSTR entry, bool def)
    {
        return GetUserInt(section, entry, def) != 0;
    }

    // get integer value from registry under HKEY_LOCAL_MACHINE
    virtual double GetMachineDouble(LPCTSTR section, LPCTSTR entry, double def)
    {
        return PrivateGetDouble(HKEY_LOCAL_MACHINE, section, entry, def);
    }

    // get integer value from registry under HKEY_CURRENT_USER
    virtual double GetUserDouble(LPCTSTR section, LPCTSTR entry, double def)
    {
        return PrivateGetDouble(HKEY_CURRENT_USER, section, entry, def);
    }

    // set double value in registry under HKEY_LOCAL_MACHINE
    virtual bool WriteMachineDouble(LPCTSTR section, LPCTSTR entry, double value)
    {
        return PrivateWriteDouble(HKEY_LOCAL_MACHINE, section, entry, value);
    }

    // set double value in registry under HKEY_CURRENT_USER
    virtual bool WriteUserDouble(LPCTSTR section, LPCTSTR entry, double value)
    {
        return PrivateWriteDouble(HKEY_CURRENT_USER, section, entry, value);
    }

    // check if the key exists for current app name; if not, rename oldAppName key
    void MigrateSettings(LPCTSTR oldAppName, bool bKeepOld);

    // get rectangle for cenering
    CRect GetMainRect();

    // center a rectangle
    void CenterRect(CRect& rect);

    // get version info for this application
    const VS_FIXEDFILEINFO& GetVersionInfo();

    // get version data for a given tag (unicode)
    CStringW GetVersionData(LPCWSTR tag);

    // get version data for a given tag (ASCII)
    CStringA GetVersionData(LPCSTR tag);

    // get version info for this application (in string format)
    CString GetStrVersionInfo(int count);

    // init and get m_pIntSess
    CInternetSession* GetInternetSession();

    // allocate a console to the process
    bool OpenConsole(LPCTSTR conTitle);

    // close the allocated console
    bool CloseConsole();

    virtual int ExitInstance();
    virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CAppAF)
};
}  // namespace AFLib
