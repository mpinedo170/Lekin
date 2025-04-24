#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <fstream>
#include <iostream>

using namespace std;

char* id[2] = {"HID_PLUGIN0", "HID_LOG0"};
int val[2];
char s[256];

int main()
{
    int i, j;
    ifstream F("Hlp\\Asm.hm");
    if (!F) return 1;

    while (F)
    {
        F.getline(s, 256);
        if (!isalpha(s[0])) continue;
        for (i = 2; --i >= 0;)
            if (!_tcsnicmp(s, id[i], _tcslen(id[i]))) break;
        if (i < 0) continue;

        char* ss = s + _tcslen(id[i]);
        while (isspace(*ss)) ss++;
        if (*ss == '0') ss++;
        if (toupper(*ss) == 'X') ss++;
        _stscanf(ss, "%x", &val[i]);
    }

    ofstream G("Hlp\\Asm2.hm");
    if (!G) return 1;
    for (i = 0; i < 2; i++)
        for (j = 1; j < 100; j++)
        {
            G << id[i] << " 0x" << hex << val[i] + j << "\n";
        }
    return 0;
}
