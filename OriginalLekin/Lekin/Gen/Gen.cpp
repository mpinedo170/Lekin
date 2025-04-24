#include "StdAfx.h"

#include "Gen.h"

using namespace std;

const int Njob = 100;
const int Nmch = 20;

void OutMch()
{
    ofstream F("a.mch");
    F << "Ordinary:\n";
    for (int i = 0; i < Nmch; i++)
    {
        char s[5];
        sprintf(s, "W%03d", i + 1);
        F << "Workcenter: " << s << "\n";
        F << "  Release: 0\n";
        F << "  Status: A\n\n";
    }
}

void OutJob()
{
    ofstream F("a.job");
    F << "Shop: Job\n";

    for (int i = 0; i < Njob; i++)
    {
        bool bBusy[Nmch] = {false};
        int r = 0;  // TRandom::Get().Uniform(200);
        int d = r + TRandom::Get().Uniform(999 - r);
        int w = 1 + TRandom::Get().Uniform(4);
        int n = Nmch;  // TRandom::Get().Uniform(Nmch) + 1;

        char s[5];
        if (i == 999)
            sprintf(s, "JA00");
        else
            sprintf(s, "J%03d", i + 1);

        F << "Job: " << s << "\n";
        F << "  Release: " << r << "\n";
        F << "  Due: " << d << "\n";
        F << "  Weight: " << w << "\n";

        for (int j = 0; j < n; j++)
        {
            int m = TRandom::Get().Uniform(Nmch - j);
            int k1 = 0, k2 = 0;
            while (true)
            {
                if (!bBusy[k1])
                    if (k2 == m)
                        break;
                    else
                        k2++;
                k1++;
            }

            m = k1;
            bBusy[m] = true;
            sprintf(s, "W%03d", m + 1);
            int p = 1 + TRandom::Get().Uniform(15);
            F << "  Oper: " << s << ";" << p << ";A\n";
        }
        F << "\n";
    }
}

int main()
{
    OutMch();
    OutJob();
    return 0;
}
