#include "StdAfx.h"

#include "NSIBuilder.h"

CWinApp theApp;

using namespace std;

CString strRoot;

const CString x86file = _T("vcredist_x86");
const CString x64file = _T("vcredist_x64");
const CString redistPath =
    _T("C:\\Program Files (x86)\\Microsoft Visual Studio\\")
    _T("2017\\Community\\VC\\Redist\\MSVC\\14.16.27012\\");

void process(LPCTSTR path, CFile& Fout)
{
    CStringArray arrDir;
    bool bFirst = true;

    CFileFind FF;
    bool bMore = FF.FindFile(strRoot + path + _T("\\*.*")) != 0;

    while (bMore)
    {
        bMore = FF.FindNextFile() != 0;
        if (FF.IsDots()) continue;

        if (FF.IsDirectory())
        {
            arrDir.Add(FF.GetFileName());
            continue;
        }

        if (bFirst)
        {
            FileWrite(
                Fout, strEol + _T("SetOutPath ") + strDQuote + _T("$INSTDIR\\") + path + strDQuote + strEol);
            bFirst = false;
        }

        FileWrite(Fout,
            _T("  File ") + strDQuote + strRoot + path + strBSlash + FF.GetFileName() + strDQuote + strEol);
    }

    FF.Close();
    for (int i = 0; i < arrDir.GetSize(); i++) process(path + strBSlash + arrDir[i], Fout);
}

// figure out the location of the MSVC redistributable packages
bool detectRedistLocations(CString& x86path, CString& x64path)
{
    CRegKey key = OpenRegSubKey(
        HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders"), false);

    TCHAR buffer[MAX_PATH];
    BYTE data[MAX_PATH];

    x86path.Empty();
    x64path.Empty();

    for (int i = 0; x86path.IsEmpty() || x64path.IsEmpty(); ++i)
    {
        DWORD length = MAX_PATH;
        DWORD tempLength = MAX_PATH;
        DWORD type = 0;

        if (RegEnumValue(key, i, buffer, &length, NULL, &type, data, &tempLength) != ERROR_SUCCESS)
            return false;

        if (StringEndsWithIC(buffer, x86file + strBSlash))
            x86path = buffer;
        else if (StringEndsWithIC(buffer, x64file + strBSlash))
            x64path = buffer;
    }

    return true;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;

    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        cout << _T("Fatal Error: MFC initialization failed") << endl;
        return 1;
    }

    // we dynamically find out Lekin build path
    CString path = GetExeFullPath();
    TStringTokenizerStr sp(path, _T("Lekin\\"));
    sp.SetIgnoreCase(true);
    int start = 0;

    while (true)
    {
        sp.ReadStr();
        if (sp.IsEof()) break;
        start = sp.GetPos();
    }

    if (start == 0)
    {
        cout << "\"Lekin\" was not found in the full path of NSIBuilder.exe." << endl;
        cout << "Build aborted." << endl;
        return 1;
    }

    path.Delete(start, path.GetLength() - start);
    strRoot = path + "Misc\\";

    try
    {
        /*
        CString x86path;
        CString x64path;

        if (!detectRedistLocations(x86path, x64path))
        {
            cout << "MSVC redistributable executables were not found." << endl;
            cout << "Build aborted." << endl;
            return 1;
        }
        */

        CFile Fout1(path + _T("Install\\LocalSys.nsh"), CFile::modeCreate | CFile::modeWrite);
        FileWrite(Fout1, "!define LOCAL_X86_PATH " + QuoteString(redistPath) + strEol);
        FileWrite(Fout1, "!define LOCAL_X64_PATH " + QuoteString(redistPath) + strEol);
        FileWrite(Fout1, "!define X86_EXE " + QuoteString(x86file + extExe) + strEol);
        FileWrite(Fout1, "!define X64_EXE " + QuoteString(x64file + extExe) + strEol);

        CFile Fout2(path + _T("Install\\Data.nsh"), CFile::modeCreate | CFile::modeWrite);
        process(_T("Data"), Fout2);
        process(_T("Data2"), Fout2);
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
    }
    return 0;
}
