#include <windows.h>
#include <iostream>

using namespace std;

void GetMaxValues(int& MaxJob, int& MaxWkc, int& MaxMch)
{
#ifdef _DEBUG
    MaxJob = 20;
    MaxWkc = 15;
    MaxMch = 30;
#else
    STARTUPINFO strInfo = {sizeof(STARTUPINFO)};
    GetStartupInfo(&strInfo);
    if (strInfo.dwX != 0xABFE)
    {
        cout << "This program is a part of LEKIN scheduling system.\n";
        cout << "It can't be used standalone.\n";
        exit(1);
    }
    MaxJob = strInfo.dwY;
    MaxWkc = strInfo.dwXSize;
    MaxMch = strInfo.dwYSize;
#endif
}
