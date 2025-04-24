#include "StdAfx.h"

#include "AFLibGlobal.h"

#include "AFLibExcBreak.h"
#include "AFLibFileKiller.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringTokenizerAlt.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

BOOL AFXAPI AfxFullPath(_Pre_notnull_ _Post_z_ LPTSTR lpszPathOut, LPCTSTR lpszFileIn);

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibThread::CExcBreak;

namespace {
class TFullPathFunc
{
private:
    // source path
    LPCTSTR m_path;

    // verify the path?
    bool m_bVerify;

public:
    TFullPathFunc(LPCTSTR path, bool bVerify)
    {
        m_bVerify = bVerify;

        if (path[0] == chrDQuote)
        {
            int len = _tcslen(path);
            m_path = CString(path + 1, len >= 2 && StringEndsWith(path, strDQuote) ? len - 2 : len - 1);
        }
        else
            m_path = path;
    }

    int operator()(LPTSTR buffer, int length)
    {
        if (m_bVerify)
        {
            if (!AfxFullPath(buffer, m_path)) buffer[0] = 0;
            return 0;
        }
        else
        {
            LPTSTR p = NULL;
            return ::GetFullPathName(m_path, length, buffer, &p);
        }
    }
};

int GetCurrentDirectoryFunc(LPTSTR buffer, int length)
{
    return GetCurrentDirectory(length, buffer);
}

int GetTempPathFunc(LPTSTR buffer, int length)
{
    return GetTempPath(length, buffer);
}

void GetModuleFileNameFunc(LPTSTR buffer, int length)
{
    GetModuleFileName(AfxGetApp()->m_hInstance, buffer, length);
}

CString PrivateGetTmpPath()
{
    CString s = GetNiceString(GetTempPathFunc);
    ASSERT(!s.IsEmpty());
    AppendSlash(s);
    Capitalize(s);
    return s;
}

CString PrivateGetExeFullPath()
{
    CString s = GetUglyString(GetModuleFileNameFunc);
    Capitalize(s);
    return s;
}

CString PrivateGetExePath()
{
    const CString& pathExe = GetExeFullPath();
    CString s, sTemp;
    SplitPath(pathExe, s, sTemp);
    return s;
}

CString PrivateGetWindowsPath()
{
    CString s = GetNiceString(::GetWindowsDirectory);
    AppendSlash(s);
    return s;
}

CString PrivateGetSystemPath()
{
    CString s = GetNiceString(::GetSystemDirectory);
    AppendSlash(s);
    return s;
}

CString PrivateGetUserPath()
{
    CString s;
    SHGetSpecialFolderPath(NULL, s.GetBuffer(MAX_PATH), CSIDL_PERSONAL, false);
    s.ReleaseBuffer();
    AppendSlash(s);
    return s;
}

CString PrivateGetCmdPath()
{
    CString s = IsWin95() ? GetWindowsPath() + _T("Command.com") : GetSystemPath() + _T("Cmd.exe");

    Capitalize(s);
    s = QuoteString(s) + _T(" /C ");
    return s;
}
}  // namespace

namespace AFLibIO {
void AppendSlash(CString& path)
{
    int len = path.GetLength();
    if (len > 0 && path[len - 1] != chrBSlash) path += strBSlash;
}

void RemoveSlash(CString& path)
{
    int len = path.GetLength();
    if (len < 2) return;
    if (path[len - 1] != chrBSlash) return;
    if (strPathSeparators.Find(path[len - 2]) >= 0) return;
    path.Delete(len - 1);
}

bool IsValidFileChar(TCHAR ch)
{
    SAFE_CONST_STR(strValidSymbols, _T(" !@#$%^&()-=_+'~`"));
    return IsAlnum(ch) || strValidSymbols.Find(ch) >= 0;
}

void TestFileName(LPCTSTR fileName, bool bWhiteEndOk)
{
    if (IsEmptyStr(fileName)) ThrowMessage(LocalAfxString(IDS_DITEM_ERR_TITLE_EMP));

    if (!bWhiteEndOk)
        if (IsSpace(fileName[0]) || IsSpace(fileName[_tcslen(fileName) - 1]))
            ThrowMessage(LocalAfxString(IDS_DITEM_ERR_TITLE_WHT));

    for (LPCTSTR ss = fileName; *ss != 0; ++ss)
        if (!IsValidFileChar(*ss)) ThrowMessage(LocalAfxString(IDS_DITEM_ERR_TITLE_ILL, CString(*ss)));
}

CString GetFullPath(LPCTSTR path)
{
    CString s = GetNiceString(TFullPathFunc(path, false));
    Capitalize(s);
    return s;
}

CString GetFullPathVerify(LPCTSTR path)
{
    CString s = GetUglyString(TFullPathFunc(path, true));
    Capitalize(s);
    return s;
}

void SplitPath(LPCTSTR fullPath, CString& path, CString& file)
{
    TStringTokenizerAlt sp(fullPath, strPathSeparators);
    path.Empty();
    file.Empty();

    while (true)
    {
        file = sp.ReadStr();
        TCHAR ch = sp.GetLastSeparator();
        if (ch == 0) break;

        path += file;
        path += ch == chrSlash ? chrBSlash : ch;
    }

    Capitalize(path);
    AppendSlash(path);
}

AFLIB CString GetFileExtension(LPCTSTR path)
{
    CString path2;
    CString file;

    SplitPath(path, path2, file);
    int iDot = file.ReverseFind(chrDot);

    return iDot < 0 ? strEmpty : file.Mid(iDot);
}

CString GetCurrentPath()
{
    CString s = GetNiceString(GetCurrentDirectoryFunc);
    AppendSlash(s);
    Capitalize(s);
    return s;
}

#define DEFINE_FUNC(func)                   \
    const CString& func()                   \
    {                                       \
        SAFE_CONST_STR(s, Private##func()); \
        return s;                           \
    }

DEFINE_FUNC(GetTmpPath)
DEFINE_FUNC(GetExeFullPath)
DEFINE_FUNC(GetExePath)
DEFINE_FUNC(GetWindowsPath)
DEFINE_FUNC(GetSystemPath)
DEFINE_FUNC(GetUserPath)
DEFINE_FUNC(GetCmdPath)

#undef DEFINE_FUNC

bool CheckDirectory(CString& path, bool bCreateOk)
{
    if (path.IsEmpty()) return false;
    AppendSlash(path);
    path = GetFullPath(path);
    if (bCreateOk) CreateDirectory(path, NULL);

    CFileFind FF;
    return FF.FindFile(path + strStar) != 0;
}

void CheckDirectoryThrow(CString& path, bool bCreateOk)
{
    if (!CheckDirectory(path, true)) ThrowMessage(LocalAfxString(IDS_CANNOT_CREATE_DIR, path));
}

bool ClearDirectory(LPCTSTR path)
{
    TFileKiller fk(path, false);
    return fk.Run();
}

bool KillDirectory(LPCTSTR path)
{
    TFileKiller fk(path, true);
    return fk.Run();
}

CString GetTmpFileName(LPCTSTR name, LPCTSTR path)
{
    TCHAR buffer[MAX_PATH] = {0};
    GetTempFileName(IsEmptyStr(path) ? GetTmpPath() : path, name, 0, buffer);
    return buffer;
}

INT64 FileSize(LPCTSTR name)
{
    CFileFind FF;
    if (!FF.FindFile(name)) return -1;

    FF.FindNextFile();
    return FF.GetLength();
}

bool EjectDisk(TCHAR driveLetter)
{
    DWORD bytes = 0;
    TCHAR s[] = _T("\\\\.\\ :");
    s[4] = driveLetter;

    HANDLE hDisk = ::CreateFile(s, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (hDisk == INVALID_HANDLE_VALUE) return false;

    DeviceIoControl(hDisk, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &bytes, 0);
    DeviceIoControl(hDisk, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &bytes, 0);

    bool bOk = DeviceIoControl(hDisk, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &bytes, 0) != 0;
    CloseHandle(hDisk);
    return bOk;
}
}  // namespace AFLibIO

namespace {
COleDateTime UTCToOle(const FILETIME& ftUTC)
{
    if (ftUTC.dwHighDateTime == 0 && ftUTC.dwLowDateTime == 0) return odtZero;
    /*
        // Conversion to Local Time is necessary in MFC for .NET 2003 (not SP1)
        FILETIME ftLocal = { 0 };
        if (!FileTimeToLocalFileTime(&ftUTC, &ftLocal))
          return odtZero;

        COleDateTime odt(ftLocal);
    */
    COleDateTime odt(ftUTC);

    return odt.GetStatus() == COleDateTime::valid ? odt : odtZero;
}

COleDateTime LocalGetFFTime(const CFileFind& FF, int timeIndex)
{
    try
    {
        FILETIME ftUTC = {0};

        switch (timeIndex)
        {
            case 0:
                FF.GetCreationTime(&ftUTC);
                break;
            case 1:
                FF.GetLastAccessTime(&ftUTC);
                break;
        }

        COleDateTime odtRes = UTCToOle(ftUTC);
        if (odtRes != odtZero) return odtRes;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    // get last write time if timeIndex is not supported
    try
    {
        FILETIME ftUTC = {0};
        FF.GetLastWriteTime(&ftUTC);
        return UTCToOle(ftUTC);
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    return odtZero;
}

inline COleDateTime LocalGetFileTime(LPCTSTR name, int timeIndex)
{
    CFileFind FF;
    if (!FF.FindFile(name)) return odtZero;

    FF.FindNextFile();
    return LocalGetFFTime(FF, timeIndex);
}

bool LocalSetFileTime(LPCTSTR name, const COleDateTime& odt, int timeIndex)
{
    FILETIME* arrTime[3] = {{NULL}};
    HANDLE hFile =
        CreateFile(name, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return false;

    SYSTEMTIME st = {0};
    FILETIME ftLocal = {0};
    FILETIME ftUTC = {0};

    odt.GetAsSystemTime(st);
    arrTime[timeIndex] = &ftUTC;

    bool bOk = SystemTimeToFileTime(&st, &ftLocal) && LocalFileTimeToFileTime(&ftLocal, &ftUTC) &&
               SetFileTime(hFile, arrTime[0], arrTime[1], arrTime[2]);

    CloseHandle(hFile);
    return bOk;
}

bool CheckAttr(LPCTSTR path, bool bDir)
{
    DWORD attr = GetFileAttributes(path);
    return attr == INVALID_FILE_ATTRIBUTES
               ? false
               : (attr & FILE_ATTRIBUTE_DIRECTORY) == (bDir ? FILE_ATTRIBUTE_DIRECTORY : 0);
}

bool PrivateNeedSwapBytes(TTypeTextFile ttf)
{
    static const WORD a = 1;
    return *LPCSTR(&a) == 1 ? ttf == ttfUnicodeBE : ttf == ttfUnicodeLE;
}

void PrivateSwapBytes(LPWSTR s, int length)
{
    for (int i = 0; i < length; ++i) s[i] = _byteswap_ushort(s[i]);
}

int PrivateFileRead(CFile& Fin, LPSTR buffer, int length)
{
    int lenRead = 0;

    try
    {
        lenRead = Fin.Read(buffer, length);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    for (LPSTR s = buffer;;)
    {
        s = reinterpret_cast<LPSTR>(memchr(s, 0, lenRead - (s - buffer)));
        if (s == NULL) break;
        *s = chrSpaceA;
    }

    return lenRead;
}

int PrivateFileRead(CFile& Fin, LPWSTR buffer, int length, TTypeTextFile ttf)
{
    int lenRead = 0;

    try
    {
        lenRead = Fin.Read(buffer, length * 2);
        if (lenRead % 2 == 1) Fin.Seek(-1, CFile::current);
        lenRead /= 2;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    if (PrivateNeedSwapBytes(ttf)) PrivateSwapBytes(buffer, lenRead);

    for (LPWSTR s = buffer;;)
    {
        s = wmemchr(s, 0, lenRead - (s - buffer));
        if (s == NULL) break;
        *s = chrSpaceW;
    }

    return lenRead;
}

void PrivateFileWriteWithSwap(CFile& Fout, CStringW& s, TTypeTextFile ttf)
{
    int length = s.GetLength();
    LPWSTR s2 = s.GetBuffer();

    PrivateSwapBytes(s2, length);
    Fout.Write(s2, length * 2);
    s.ReleaseBuffer();
}
}  // namespace

namespace {
struct LANGANDCODEPAGE
{
    WORD wLanguage;
    WORD wCodePage;
};
}  // namespace

namespace AFLibPrivate {
VS_FIXEDFILEINFO LocalGetVersion(LPVOID buffer, LPCTSTR name)
{
    VS_FIXEDFILEINFO* pVer = NULL;
    UINT bufSize = 0;

    if (!VerQueryValue(buffer, strBSlash, reinterpret_cast<LPVOID*>(&pVer), &bufSize))
        ThrowMessage(LocalAfxString(IDS_NO_VERSION, name));
    return *pVer;
}

CStringW LocalGetVersionData(LPVOID buffer, LPCWSTR tag, LPCTSTR name)
{
    SAFE_CONST_STR_W(strTrans, "\\VarFileInfo\\Translation");
    SAFE_CONST_STR_W(strFileInfo, "\\StringFileInfo\\");
    SAFE_STATIC_SB_W(path);
    LANGANDCODEPAGE* pTranslate = NULL;
    LPWSTR pData = NULL;
    UINT bufSize = 0;

    if (!VerQueryValueW(buffer, strTrans, reinterpret_cast<LPVOID*>(&pTranslate), &bufSize))
        ThrowMessage(LocalAfxString(IDS_NO_VERSION, name));

    int nLang = bufSize / sizeof(LANGANDCODEPAGE);

    for (int i = 0; i < nLang; ++i)
    {
        path.Empty();
        path += strFileInfo;
        path.AppendHex(pTranslate[i].wLanguage, 4);
        path.AppendHex(pTranslate[i].wCodePage, 4);
        path += chrBSlashW;
        path += tag;

        if (!VerQueryValueW(buffer, path, reinterpret_cast<LPVOID*>(&pData), &bufSize)) continue;

        while (bufSize > 0 && pData[bufSize - 1] == 0) --bufSize;

        return CStringW(pData, bufSize);
    }

    ThrowMessage(LocalAfxString(IDS_NO_VERSION_DATA, name, CString(tag)));
    return strEmptyW;
}
}  // namespace AFLibPrivate

namespace AFLibIO {
COleDateTime FileGetCreationTime(LPCTSTR name)
{
    return LocalGetFileTime(name, 0);
}

COleDateTime FileGetLastAccessTime(LPCTSTR name)
{
    return LocalGetFileTime(name, 1);
}

COleDateTime FileGetLastWriteTime(LPCTSTR name)
{
    return LocalGetFileTime(name, 2);
}

COleDateTime FileGetCreationTime(const CFileFind& FF)
{
    return LocalGetFFTime(FF, 0);
}

COleDateTime FileGetLastAccessTime(const CFileFind& FF)
{
    return LocalGetFFTime(FF, 1);
}

COleDateTime FileGetLastWriteTime(const CFileFind& FF)
{
    return LocalGetFFTime(FF, 2);
}

bool FileSetCreationTime(LPCTSTR name, const COleDateTime& odt)
{
    return LocalSetFileTime(name, odt, 0);
}

bool FileSetLastAccessTime(LPCTSTR name, const COleDateTime& odt)
{
    return LocalSetFileTime(name, odt, 1);
}

bool FileSetLastWriteTime(LPCTSTR name, const COleDateTime& odt)
{
    return LocalSetFileTime(name, odt, 2);
}

bool FileExists(LPCTSTR path)
{
    if (StringEndsWith(path, strBSlash))
    {
        CString path2(path, _tcslen(path) - 1);
        return CheckAttr(path2, true);
    }
    else
        return CheckAttr(path, false);
}

int FileRead(CFile& Fin, LPSTR buffer, int length, TTypeTextFile ttf)
{
    if (ttf == ttfAscii) return PrivateFileRead(Fin, buffer, length);

    CAutoVectorPtr<WCHAR> s(new WCHAR[length]);
    int lenRead = PrivateFileRead(Fin, s, length, ttf);
    return ::WideCharToMultiByte(_AtlGetConversionACP(), 0, s, lenRead, buffer, length, NULL, NULL);
}

int FileRead(CFile& Fin, LPWSTR buffer, int length, TTypeTextFile ttf)
{
    if (ttf != ttfAscii) return PrivateFileRead(Fin, buffer, length, ttf);

    CAutoVectorPtr<CHAR> s(new CHAR[length]);
    int lenRead = PrivateFileRead(Fin, s, length);
    return ::MultiByteToWideChar(_AtlGetConversionACP(), 0, s, lenRead, buffer, length);
}

CString FileRead(CFile& Fin, int length, TTypeTextFile ttf)
{
    CString buffer;
    LPTSTR s = buffer.GetBuffer(length + 1);
    int lenRead = FileRead(Fin, s, length, ttf);

    s[lenRead] = 0;
    buffer.ReleaseBuffer();
    return buffer;
}

void FileWrite(CFile& Fout, LPCSTR s, TTypeTextFile ttf)
{
    if (ttf == ttfAscii)
        Fout.Write(s, strlen(s));
    else
    {
        CStringW s2(s);
        if (PrivateNeedSwapBytes(ttf))
            PrivateFileWriteWithSwap(Fout, s2, ttf);
        else
            Fout.Write(s2, s2.GetLength() * 2);
    }
}

void FileWrite(CFile& Fout, LPCWSTR s, TTypeTextFile ttf)
{
    if (ttf == ttfAscii)
    {
        CStringA s2(s);
        Fout.Write(s2, s2.GetLength());
    }
    else if (PrivateNeedSwapBytes(ttf))
    {
        CStringW s2(s);
        PrivateFileWriteWithSwap(Fout, s2, ttf);
    }
    else
        Fout.Write(s, wcslen(s) * 2);
}

void FileWriteSignature(CFile& Fout, TTypeTextFile ttf, bool bSkipAscii)
{
    if (ttf == ttfAscii && bSkipAscii) return;
    Fout.Write(arrStrTextPrefix[ttf], arrLenTextPrefix[ttf]);
}

VS_FIXEDFILEINFO FileGetVersion(LPCTSTR name)
{
    DWORD temp = 0;
    int len = GetFileVersionInfoSize(name, &temp);

    if (len <= 0) ThrowMessage(LocalAfxString(IDS_NO_VERSION, name));

    LPVOID pBuffer = _alloca(len);

    if (GetFileVersionInfo(name, 0, len, pBuffer) == 0) ThrowMessage(LocalAfxString(IDS_NO_VERSION, name));
    VS_FIXEDFILEINFO ver = LocalGetVersion(pBuffer, name);
    return ver;
}

INT64 GetDiskSpace(LPCTSTR path)
{
    DWORD SectorsPerCluster = 0;
    DWORD BytesPerSector = 0;
    DWORD NumberOfFreeClusters = 0;
    DWORD TotalNumberOfClusters = 0;
    CString pathRoot;

    TStringTokenizerAlt sp(path, strPathSeparators);
    CString s = sp.ReadStr();

    switch (sp.GetLastSeparator())
    {
        case chrColon:
            // drive letter
            pathRoot = s + _T(":\\");
            break;

        case 0:
            return 0;

        default:
            // UNC path: has to start with 2 back-slashes
            if (!s.IsEmpty()) return 0;

            s = sp.ReadStr();
            if (!s.IsEmpty() || sp.IsEof() || sp.GetLastSeparator() == chrColon) return 0;

            s = sp.ReadStr();
            if (s.IsEmpty() || sp.GetLastSeparator() == chrColon) return 0;

            pathRoot = _T("\\\\") + s + strBSlash;

            if (!sp.IsEof())
            {
                s = sp.ReadStr();
                if (s.IsEmpty() || sp.GetLastSeparator() == chrColon) return 0;
                pathRoot += s + strBSlash;
            }
            break;
    }

    if (!GetDiskFreeSpace(
            pathRoot, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters))
        return 0;

    return INT64(NumberOfFreeClusters) * SectorsPerCluster * BytesPerSector;
}
}  // namespace AFLibIO
