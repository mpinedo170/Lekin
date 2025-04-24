#include "StdAfx.h"

#include "Parameter.h"

//*****
int equal(double f1, double f2)
{
    return fabs(f1 - f2) < fSmall;
}

void wait()
{
    char a;
    cout << "Please press enter to continue";
    cin.get(a);
    cout << endl;
}

void InitRandom()
{
    // Seed the random-number generator with current time so that
    // the numbers will be different every time we run.
    //
    srand((unsigned)time(NULL));
}

double Random()
{
    // generate random 0-1
    double rnd1;
    rnd1 = rand();
    rnd1 /= RAND_MAX + 1.0;
    return rnd1;
}

int Random(int x)
{
    // generate an integer random variable 1 - x;
    if (x > 0)
    {
        double xx, rnd1;
        rnd1 = (double)rand();
        rnd1 /= (RAND_MAX + 1.0);
        xx = x * rnd1 + 1;
        return ((int)xx);
    }
    else
        return 0;
}

int Random(int iFrom, int iTo)
{
    // generate an int random variable iFrom - iTo
    int iTemp = iTo - iFrom + 1;
    iTemp = Random(iTemp) - 1;
    return (iFrom + iTemp);
}

double Random(double fFrom, double fTo)
{
    // generate a float random variable 0.1 - x
    long iTemp = (int)((fTo - fFrom) * 10);
    double fTemp1;
    if (iTemp > 0) fTemp1 = Random(iTemp) - 1;
    return (fFrom + fTemp1 / 10);
}

double Random(double fFrom, double fTo, int idigit)
{
    // generate a float random variable with defined digits
    double f10s = 1;
    int i;
    for (i = 1; i <= idigit; i++) f10s *= 10;
    long iTemp = (int)((fTo - fFrom) * f10s);
    double fTemp1;
    if (iTemp > 0) fTemp1 = Random(iTemp) - 1;
    return (fFrom + fTemp1 / f10s);
}

void mIDtoa(char* szInt5, TMachineID mID, int)
{
    char* sPtr = szInt5;
    sPtr += 5;
    *sPtr = '\0';
    sPtr--;
    char c0 = '0', cTemp;
    int iTemp;
    for (int i = 1; i <= 5; i++)
    {
        iTemp = mID % 10;
        mID /= 10;
        cTemp = (char)(c0 + iTemp);
        *sPtr = cTemp;
        sPtr--;
    }
}

void jIDtoa(char* szInt5, TJobID jID, int)
{
    char* sPtr = szInt5;
    sPtr += 5;
    *sPtr = '\0';
    sPtr--;
    char c0 = '0', cTemp;
    int iTemp;
    for (int i = 1; i <= 5; i++)
    {
        iTemp = jID % 10;
        jID /= 10;
        cTemp = (char)(c0 + iTemp);
        *sPtr = cTemp;
        sPtr--;
    }
}

void oIDtoa(char* szInt5, TOperationID oID, int)
{
    char* sPtr = szInt5;
    sPtr += 5;
    *sPtr = '\0';
    sPtr--;
    char c0 = '0', cTemp;
    int iTemp;
    for (int i = 1; i <= 5; i++)
    {
        iTemp = oID % 10;
        oID /= 10;
        cTemp = (char)(c0 + iTemp);
        *sPtr = cTemp;
        sPtr--;
    }
}

void wIDtoa(char* szInt5, TWorkcenterID wID, int)
{
    char* sPtr = szInt5;
    sPtr += 5;
    *sPtr = '\0';
    sPtr--;
    char c0 = '0', cTemp;
    int iTemp;
    for (int i = 1; i <= 5; i++)
    {
        iTemp = wID % 10;
        wID /= 10;
        cTemp = (char)(c0 + iTemp);
        *sPtr = cTemp;
        sPtr--;
    }
}

void ltoa(long wID, char* szInt5)
{
    char* sPtr = szInt5;
    int iN;
    if (wID)
        iN = (int)log10(double(wID)) + 1;
    else
        iN = 0;
    sPtr += (iN);
    *sPtr = '\0';
    if (iN) sPtr--;
    char c0 = '0', cTemp;
    long iTemp;
    for (int i = 1; i <= iN; i++)
    {
        iTemp = wID % 10;
        wID /= 10;
        cTemp = (char)(c0 + iTemp);
        *sPtr = cTemp;
        sPtr--;
    }
}

void dtoXa(char* szTemp, double ll, int nn, int dd)  // dd digits, nn total chars
{
    char c_i[50] = "", c_d[50] = "", c_i2[50] = "", c_d2[50] = "";
    int len_i, len_d, iDig;
    long ll_i, ll_d;
    double d;
    *szTemp = 0;
    if (ll < 0)
    {
        _tcscpy(szTemp, "-");
        ll *= -1;
    }
    if (ll > 0)
        iDig = (int)log10(ll);
    else
    {
        _tcscpy(szTemp, "0");
        return;
    }
    if ((iDig + dd + 2) <= nn)
    {
        ll_i = (long)ll;
        d = ll - ll_i;
        for (int i = 1; i <= dd; i++) d *= 10;
        ll_d = (long)(d + .5);
        //*****
        if (ll_d)
            if (int(log10(double(ll_d))) >= dd)
            {
                ll_d = 0;
                ll_i++;
            }
        //*****
        ltoa(ll_d, c_d);  // dec point part
        len_d = _tcslen(c_d);
        ltoa(ll_i, c_i);  // integer part
        for (int i = len_d; i < dd; i++) strcat(c_d2, "0");
        strcat(c_d2, c_d);
        if (dd > 0)
        {
            strcat(c_i, ".");
            strcat(c_i, c_d2);
        }
        len_i = _tcslen(c_i);
        // for(i=len_i;i<nn;i++) strcat(c_i2," ");
        strcat(c_i2, c_i);
        strcat(szTemp, c_i2);
    }
    else
    {
        if (nn < 3) return;
        dd = (dd < (nn - 3)) ? dd : (nn - 3);
        ll_i = (long)(ll / (pow(10., iDig)));
        ltoa(ll_i, c_i);
        ll = ll - (ll_i * pow(10., iDig));
        ll_d = (long)(ll / (pow(10., (iDig - dd))));
        if (ll_d) strcat(c_i, ".");
        ltoa(ll_d, c_d);
        strcat(szTemp, c_i);
        strcat(szTemp, c_d);
        strcat(szTemp, "e");
        ltoa(iDig, c_d);
        strcat(szTemp, c_d);
    }
}

int strcount(char* st, char lookfor)
{
    int temp = 0;
    char* ptr;
    for (ptr = st; *ptr != '\0'; ptr++)
    {
        if (*ptr == lookfor) temp++;
    }
    return temp;
}

void strextc(char* result, char* st, char lookfor)
{
    char* ptr;
    for (ptr = st; (*ptr != '\0') && (*ptr != lookfor); ptr++)
    {
        *result++ = *ptr;
    }
    *result = '\0';
    if (*ptr == '\0')
        _tcscpy(st, ptr);
    else
        _tcscpy(st, ++ptr);
}

int strextc_int(char* st, char lookfor)
{
    int temp;
    char extc[6];
    strextc(extc, st, lookfor);
    temp = atoi(extc);
    return temp;
}

TJobID strextc_JobID(char* st, char lookfor)
{
    return (TJobID)strextc_int(st, lookfor);
}

TMachineID strextc_MachineID(char* st, char lookfor)
{
    return (TMachineID)strextc_int(st, lookfor);
}

TOperationID strextc_OperationID(char* st, char lookfor)
{
    return (TOperationID)strextc_int(st, lookfor);
}

CString std_filename(LPCSTR szName, LPCSTR szExt)
{
    LPCSTR dot = _tcschr(szName, '.');
    LPCSTR slash = _tcspbrk(szName, "\\/");
    if (dot <= slash)
        return CString(szName) + szExt;
    else
        return szName;
}

void roundoff(double& fX, int iDigit)
{
    int iRev = 0;
    double fTemp, fLeft, fDig2 = 1;
    if (fX < 0)
    {
        fX *= -1;
        iRev = 1;
    }
    fTemp = floor(fX);
    fLeft = fX - fTemp;
    for (int i = 1; i <= iDigit; i++)
    {
        fDig2 *= 10;
        fLeft *= 10;
    }
    fLeft += 0.5;
    fX = fTemp + floor(fLeft) / fDig2;
    if (iRev) fX *= -1;
}
