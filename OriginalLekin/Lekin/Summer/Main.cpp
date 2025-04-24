#define _WIN32_WINNT 0x0502  // Win NT version (XP SP1)
#define _WIN32_IE 0x0601     // Internet Explorer 6.0 SP1

#include <afx.h>
#include <afxwin.h>
#include <iostream>

#include "..\ControlSum.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    LPWORD pBuffer = NULL;
    int result = 1;

    try
    {
        CFile F(argv[1], CFile::modeReadWrite);
        CFileStatus status;
        F.GetStatus(status);
        ControlSum(F, pBuffer, NULL, true);
        int size = int(F.GetLength());
        F.Seek(0, CFile::begin);
        F.Write(pBuffer, size);
        F.Close();
        F.SetStatus(argv[1], status);
        cout << "Control sum updated.\n";
        result = 0;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        cout << "File error!\n";
    }

    delete pBuffer;
    return result;
}
