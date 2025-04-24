#include "StdAfx.h"

#include "AFLibGlobal.h"

#include "AFLibAppAF.h"
#include "Local.h"

namespace {
class TProfileFunc
{
private:
    LPCTSTR m_section;
    LPCTSTR m_key;
    LPCTSTR m_def;
    LPCTSTR m_file;

public:
    TProfileFunc(LPCTSTR section, LPCTSTR key, LPCTSTR def, LPCTSTR file) :
        m_section(section),
        m_key(key),
        m_def(def),
        m_file(file)
    {}
    void operator()(LPTSTR buffer, int length)
    {
        GetPrivateProfileString(m_section, m_key, m_def, buffer, length, m_file);
    }
};
}  // namespace

namespace AFLib {
using namespace AFLibPrivate;

CString GetWindowsRegKeyPath(bool bNT)
{
    CString kk;
    if (bNT && !IsWin95()) kk = _T(" NT");
    return LocalAfxString(IDS_WINREG, kk);
}

CString GetRootRegKeyName(HKEY hKeyRoot)
{
    static const HKEY translateTable[] = {HKEY_CLASSES_ROOT, HKEY_CURRENT_CONFIG, HKEY_CURRENT_USER,
        HKEY_DYN_DATA, HKEY_LOCAL_MACHINE, HKEY_PERFORMANCE_DATA, HKEY_PERFORMANCE_NLSTEXT,
        HKEY_PERFORMANCE_TEXT, HKEY_USERS, NULL};

    for (int i = 0; translateTable[i] != NULL; ++i)
        if (hKeyRoot == translateTable[i]) return LocalAfxString(IDS_HKEY00 + i);

    return strEllipsis;
}

CRegKey OpenRegSubKey(HKEY hKeyRoot, LPCTSTR path, bool bWriteAccess, int accessRights)
{
    ASSERT(!IsEmptyStr(path));
    if (accessRights == 0) accessRights = bWriteAccess ? KEY_READ | KEY_WRITE : KEY_READ;

    CRegKey keyResult;
    int result = bWriteAccess
                     ? keyResult.Create(hKeyRoot, path, REG_NONE, REG_OPTION_NON_VOLATILE, accessRights)
                     : keyResult.Open(hKeyRoot, path, accessRights);

    if (result != ERROR_SUCCESS)
        ThrowMessage(LocalAfxString(IDS_CANNOT_CREATE_KEY, GetRootRegKeyName(hKeyRoot) + strBSlash + path));

    return keyResult;
}

void CopyRegKey(HKEY hKeyRootSrc, LPCTSTR pathSrc, HKEY hKeyRootDst, LPCTSTR pathDst)
{
    TCHAR achClass[MAX_PATH] = {0};  // buffer for class name
    DWORD cchClassName = MAX_PATH;   // size of class string
    DWORD cSubKeys = 0;              // number of subkeys
    DWORD cbMaxSubKey = 0;           // longest subkey size
    DWORD cchMaxClass = 0;           // longest class string
    DWORD cValues = 0;               // number of values for key
    DWORD cchMaxValue = 0;           // longest value name
    DWORD cbMaxValueData = 0;        // longest value data

    CRegKey keySrc;
    keySrc.Open(hKeyRootSrc, pathSrc, KEY_READ);

    DWORD retCode = RegQueryInfoKey(keySrc, achClass, &cchClassName, NULL, &cSubKeys, &cbMaxSubKey,
        &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, NULL, NULL);

    if (retCode != ERROR_SUCCESS) return;

    CRegKey keyDst;
    keyDst.Create(hKeyRootDst, pathDst, achClass);

    if (cSubKeys > 0)
    {
        CAutoVectorPtr<TCHAR> achKey(new TCHAR[cbMaxSubKey + 1]);

        for (DWORD i = 0; i < cSubKeys; ++i)
        {
            DWORD cbName = cbMaxSubKey + 1;

            achKey[0] = 0;
            retCode = RegEnumKeyEx(keySrc, i, achKey, &cbName, NULL, NULL, NULL, NULL);

            if (retCode == ERROR_SUCCESS) CopyRegKey(keySrc, achKey, keyDst, achKey);
        }
    }

    if (cValues > 0)
    {
        CAutoVectorPtr<TCHAR> achValue(new TCHAR[cchMaxValue + 1]);
        CAutoVectorPtr<BYTE> data(new BYTE[cbMaxValueData]);

        for (DWORD i = 0; i < cValues; ++i)
        {
            DWORD cchValue = cchMaxValue + 1;
            DWORD cbData = cbMaxValueData;
            DWORD dwType = 0;

            achValue[0] = 0;
            data[0] = 0;
            retCode = RegEnumValue(keySrc, i, achValue, &cchValue, NULL, &dwType, data, &cbData);

            if (retCode == ERROR_SUCCESS) keyDst.SetValue(achValue, dwType, data, cbData);
        }
    }
}

CString GetRegValueStr(HKEY hKey, LPCTSTR sec, LPCTSTR def)
{
    DWORD type = 0;
    DWORD size = 0;

    if (RegQueryValueEx(hKey, sec, NULL, &type, NULL, &size) != ERROR_SUCCESS) return def;
    if (type != REG_SZ) return def;

    CString s;
    RegQueryValueEx(hKey, sec, NULL, NULL, LPBYTE(s.GetBuffer(size + 1)), &size);
    s.ReleaseBuffer();
    return s;
}

int GetRegValueInt(HKEY hKey, LPCTSTR sec, int def)
{
    DWORD type = 0;
    DWORD size = 0;
    DWORD value = 0;

    if (RegQueryValueEx(hKey, sec, NULL, &type, NULL, &size) != ERROR_SUCCESS) return def;
    if (type != REG_DWORD) return def;

    RegQueryValueEx(hKey, sec, NULL, NULL, LPBYTE(&value), &size);
    return value;
}

bool SetRegValueStr(HKEY hKey, LPCTSTR sec, LPCTSTR value)
{
    return RegSetValueEx(hKey, sec, 0, REG_SZ, LPBYTE(value), _tcslen(value) + 1) == ERROR_SUCCESS;
}

bool SetRegValueInt(HKEY hKey, LPCTSTR sec, int value)
{
    return RegSetValueEx(hKey, sec, 0, REG_DWORD, LPBYTE(&value), 4) == ERROR_SUCCESS;
}

CString GetOwnerInfo(TGetOwnerInfo goi)
{
    static const LPCTSTR arrKey[] = {_T("RegisteredOwner"), _T("RegisteredOrganization"), _T("ProductId")};

    CString keyPath = GetWindowsRegKeyPath(true);
    CRegKey key;
    if (key.Open(HKEY_LOCAL_MACHINE, keyPath, KEY_QUERY_VALUE) != ERROR_SUCCESS)
        ThrowMessage(LocalAfxString(IDS_NO_WIN_KEY));
    CString info = GetRegValueStr(key, arrKey[goi], strEmpty);
    return info;
}

CString GetComputerID()
{  // The serial number is a 96-bit value so we have to break up the
    // number into three unsigned int's.
    UINT upper = 0;
    UINT middle = 0;
    UINT lower = 0;

    __asm {
        // To use CPUID, fill EAX with a number corresponding to the operation
        // you wish to perform. In this case we want the serial number, which
        // is operation 3, however; some CPU's (like AMD) don't support it. So,
        // we need to call CPUID with operation 0, which will get the vendor ID
        // and replace EAX with the highest supported operation.
      mov eax, 0
      cpuid

            // If EAX < 3, the processor doesn't support getting the serial number
            // with CPUID. We're hosed, so jump to the end.
      cmp eax, 3
      jl done

            // If we get here, the processor does support getting the serial number.
            // We need to call CPUID again with EAX set to 3. The value is a 96 bit
            // number returned in EBX:EDX:ECX, so we need to clear those registers.
      mov eax, 3
      mov ebx, 0
      mov edx, 0
      mov ecx, 0
      cpuid

            // store the values in our C variables
      mov upper, ebx
      mov middle, edx
      mov lower, ecx

    done:
    }

    // If upper, middle, and lower are all still 0 it means we failed because the
    // processor doesn't support CPUID -- return XXX
    // Otherwise convert to hex
    if (upper == 0 && middle == 0 && lower == 0) return CString(_T('X'), 24);

    return HexToStr(upper, 8) + HexToStr(middle, 8) + HexToStr(lower, 8);
}

CString GetPrivateProfileValueStr(LPCTSTR section, LPCTSTR key, LPCTSTR def, LPCTSTR file)
{
    return GetUglyString(TProfileFunc(section, key, def, file));
}

CString DynamicGetProfileString(LPCTSTR section, LPCTSTR entry, LPCTSTR def)
{
    CAppAF* pApp = dynamic_cast<CAppAF*>(AfxGetApp());
    return pApp == NULL ? AfxGetApp()->GetProfileString(section, entry, def)
                        : pApp->GetUserString(section, entry, def);
}

bool DynamicWriteProfileString(LPCTSTR section, LPCTSTR entry, LPCTSTR value)
{
    CAppAF* pApp = dynamic_cast<CAppAF*>(AfxGetApp());
    return pApp == NULL ? AfxGetApp()->WriteProfileString(section, entry, value) != 0
                        : pApp->WriteUserString(section, entry, value);
}

int DynamicGetProfileInt(LPCTSTR section, LPCTSTR entry, int def)
{
    CAppAF* pApp = dynamic_cast<CAppAF*>(AfxGetApp());
    return pApp == NULL ? AfxGetApp()->GetProfileInt(section, entry, def)
                        : pApp->GetUserInt(section, entry, def);
}

bool DynamicWriteProfileInt(LPCTSTR section, LPCTSTR entry, int value)
{
    CAppAF* pApp = dynamic_cast<CAppAF*>(AfxGetApp());
    return pApp == NULL ? AfxGetApp()->WriteProfileInt(section, entry, value) != 0
                        : pApp->WriteUserInt(section, entry, value);
}
}  // namespace AFLib
