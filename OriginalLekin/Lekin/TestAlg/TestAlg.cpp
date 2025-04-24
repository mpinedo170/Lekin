#include <windows.h>
#include <iostream>

using namespace std;

int main()
{
    int i = 1;
    Sleep(3000);
    while (i != 0)
    {
        cout << "Input => ";
        cin >> i;
        cout << "Got " << i << "\n";
        Sleep(i);
    }
    return i;
}
