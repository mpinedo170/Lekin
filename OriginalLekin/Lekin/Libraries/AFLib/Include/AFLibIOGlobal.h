#pragma once

/////////////////////////////////////////////////////////////////////////////
// Input/output-related global functions

namespace AFLibIO {  // 3 supported text file types: 8-bit, 16-bit little-endian, 16-bit big-endian
enum TTypeTextFile
{
    ttfAscii,
    ttfUnicodeLE,
    ttfUnicodeBE,
    ttfN
};

// append a slash to a path if necessary
AFLIB void AppendSlash(CString& path);

// remove a slash from a path if necessary
AFLIB void RemoveSlash(CString& path);

// is this a valid character for a filename?
AFLIB bool IsValidFileChar(TCHAR ch);

// test if this is a valid filename; throw if not
AFLIB void TestFileName(LPCTSTR fileName, bool bWhiteEndOk);

// get full file path from a partial path
AFLIB CString GetFullPath(LPCTSTR path);

// get full file path from a partial path; verify it (use AfxFullPath())
AFLIB CString GetFullPathVerify(LPCTSTR path);

// split full path into path name and file name
AFLIB void SplitPath(LPCTSTR fullPath, CString& path, CString& file);

// get extension (including dot); path may be full or partial
AFLIB CString GetFileExtension(LPCTSTR path);

// get current directory
AFLIB CString GetCurrentPath();

// get temporary path
AFLIB const CString& GetTmpPath();

// get module full path, including file name
AFLIB const CString& GetExeFullPath();

// get module path (just path, no file name)
AFLIB const CString& GetExePath();

// get Windows path (usually C:\WINNT\ or C:\Windows\)
AFLIB const CString& GetWindowsPath();

// get Windows system path (usually GetWindowsPath()\system32\)
AFLIB const CString& GetSystemPath();

// get user default path (usually C:\Documents and Settings\)
AFLIB const CString& GetUserPath();

// get Command.com command
AFLIB const CString& GetCmdPath();

// check if directory exists; maybe create
AFLIB bool CheckDirectory(CString& path, bool bCreateOk);

// check if directory exists; maybe create; if unsuccessful, throw exception
AFLIB void CheckDirectoryThrow(CString& path, bool bCreateOk);

// remove directory with subdirectories
AFLIB bool KillDirectory(LPCTSTR path);

// remove all files and subdirectories, but do not kill this directory
AFLIB bool ClearDirectory(LPCTSTR path);

// file length, or -1 if file not found
AFLIB INT64 FileSize(LPCTSTR name);

// create a temporary file name
AFLIB CString GetTmpFileName(LPCTSTR name, LPCTSTR path = NULL);

// get file time, oleDateZero if file not found
AFLIB COleDateTime FileGetCreationTime(LPCTSTR name);
AFLIB COleDateTime FileGetLastAccessTime(LPCTSTR name);
AFLIB COleDateTime FileGetLastWriteTime(LPCTSTR name);

// get file time from FileFind, oleDateZero if file not found
AFLIB COleDateTime FileGetCreationTime(const CFileFind& FF);
AFLIB COleDateTime FileGetLastAccessTime(const CFileFind& FF);
AFLIB COleDateTime FileGetLastWriteTime(const CFileFind& FF);

// set file time, return true if successful
AFLIB bool FileSetCreationTime(LPCTSTR name, const COleDateTime& odt);
AFLIB bool FileSetLastAccessTime(LPCTSTR name, const COleDateTime& odt);
AFLIB bool FileSetLastWriteTime(LPCTSTR name, const COleDateTime& odt);

// check if file exists; if path ends with a backslash, make sure it is a directory
AFLIB bool FileExists(LPCTSTR path);

// read string from a file into a prepared buffer (supports Ascii & Unicode text files)
int FileRead(CFile& Fin, LPSTR buffer, int length, TTypeTextFile ttf = ttfAscii);
int FileRead(CFile& Fin, LPWSTR buffer, int length, TTypeTextFile ttf = ttfAscii);

// read string from a text file (supports Ascii & Unicode text files)
AFLIB CString FileRead(CFile& Fin, int length, TTypeTextFile ttf = ttfAscii);

// write string into a file (supports Ascii & Unicode text files)
AFLIB void FileWrite(CFile& Fout, LPCSTR s, TTypeTextFile ttf = ttfAscii);
AFLIB void FileWrite(CFile& Fout, LPCWSTR s, TTypeTextFile ttf = ttfAscii);

// write the UTF-8 or Unicode signature into the start of the file; UTF-8 is normally ignored
AFLIB void FileWriteSignature(CFile& Fout, TTypeTextFile ttf, bool bSkipAscii = true);

// get version information from a file
AFLIB VS_FIXEDFILEINFO FileGetVersion(LPCTSTR name);

// eject disk from the drive (e.g., CD)
AFLIB bool EjectDisk(TCHAR driveLetter);

// disk free space
AFLIB INT64 GetDiskSpace(LPCTSTR path);

// number of bytes in megabyte
const int MegaByte = 1 << 20;
}  // namespace AFLibIO
