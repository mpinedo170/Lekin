#include "StdAfx.h"

#include "AFLibArrays.h"

#include "AFLibMaps.h"
#include "AFLibMathGlobal.h"

using namespace AFLibMath;

namespace AFLib {
int BinarySearch(
    LPCVOID key, LPCVOID base, int num, int width, int(_cdecl* compare)(LPCVOID, LPCVOID), bool& bFound)
{
    bFound = false;
    if (num == 0) return 0;

    int dd = compare(key, base);
    if (dd <= 0)
    {
        bFound = dd == 0;
        return 0;
    }

    dd = compare(key, LPBYTE(base) + (num - 1) * width);
    if (dd == 0)
    {
        bFound = true;
        return num - 1;
    }
    if (dd > 0) return num;

    int a = 0, b = num - 1;
    while (b - a > 1)
    {
        int c = (a + b) / 2;
        dd = compare(key, LPBYTE(base) + c * width);
        if (dd == 0)
        {
            bFound = true;
            return c;
        }
        if (dd > 0)
            a = c;
        else
            b = c;
    }

    return b;
}

int CompareGL(int key1, int key2)
{
    return key1 < key2 ? -1 : key1 > key2 ? 1 : 0;
}

int CompareGL(UINT key1, UINT key2)
{
    return key1 < key2 ? -1 : key1 > key2 ? 1 : 0;
}

int CompareGL(double key1, double key2)
{
    return key1 < key2 ? -1 : key1 > key2 ? 1 : 0;
}

int CompareGL(LPCTSTR key1, LPCTSTR key2)
{
    return _tcsicmp(key1, key2);
}
}  // namespace AFLib

namespace {
const int arrPrimeNumber[] = {17, 29, 43, 67, 101, 151, 227, 347, 521, 787, 1181, 1777, 2671, 4007, 6011,
    9029, 13553, 20333, 30509, 45763, 68659, 103001, 154501, 231779, 347671, 521519, 782297, 1173463, 1760203,
    2640317, 3960497, 5940761, 8911141, 13366711, 20050081, 30075127, 45112693, 67669079, 101503627,
    152255461, 228383273, 342574909};

int compareStr(LPCVOID a1, LPCVOID a2)
{
    const CString& s1 = *reinterpret_cast<const CString*>(a1);
    const CString& s2 = *reinterpret_cast<const CString*>(a2);

    int a = s1.CompareNoCase(s2);
    return a == 0 ? s1.Compare(s2) : a;
}

int compareStrLptstr(LPCVOID a1, LPCVOID a2)
{
    LPCTSTR s1 = LPCTSTR(a1);
    const CString& s2 = *reinterpret_cast<const CString*>(a2);

    int a = -s2.CompareNoCase(s1);
    return a == 0 ? -s2.Compare(s1) : a;
}

int compareInt(LPCVOID p1, LPCVOID p2)
{
    int a1 = *reinterpret_cast<LPCINT>(p1);
    int a2 = *reinterpret_cast<LPCINT>(p2);
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

int compareUint(LPCVOID p1, LPCVOID p2)
{
    UINT a1 = *reinterpret_cast<LPCUINT>(p1);
    UINT a2 = *reinterpret_cast<LPCUINT>(p2);
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

int compareInt64(LPCVOID p1, LPCVOID p2)
{
    INT64 a1 = *reinterpret_cast<LPCINT64>(p1);
    INT64 a2 = *reinterpret_cast<LPCINT64>(p2);
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

int compareUint64(LPCVOID p1, LPCVOID p2)
{
    UINT64 a1 = *reinterpret_cast<LPCUINT64>(p1);
    UINT64 a2 = *reinterpret_cast<LPCUINT64>(p2);
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

int compareFloat(LPCVOID p1, LPCVOID p2)
{
    float a1 = *reinterpret_cast<const float*>(p1);
    float a2 = *reinterpret_cast<const float*>(p2);
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}

int compareDouble(LPCVOID p1, LPCVOID p2)
{
    double a1 = *reinterpret_cast<const double*>(p1);
    double a2 = *reinterpret_cast<const double*>(p2);
    return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}
}  // namespace

namespace AFLib {
void SortStrings(CStringArray& arr)
{
    qsort(arr.GetData(), arr.GetSize(), sizeof(CString), compareStr);
}

int BSearchStrings(LPCTSTR key, const CStringArray& arr, bool& bFound)
{
    return BinarySearch(key, arr.GetData(), arr.GetSize(), sizeof(CString), compareStrLptstr, bFound);
}

int BSearchStringsIC(LPCTSTR key, const CStringArray& arr, bool& bFound)
{
    int index = BinarySearch(key, arr.GetData(), arr.GetSize(), sizeof(CString), compareStrLptstr, bFound);
    if (bFound) return index;

    if ((index < arr.GetSize() && arr[index].CompareNoCase(key) == 0) ||
        (index > 0 && arr[index - 1].CompareNoCase(key) == 0))
        bFound = true;
    return index;
}

// CIntArray

void CIntArray::Sort()
{
    qsort(GetData(), GetSize(), sizeof(int), compareInt);
}

int CIntArray::BSearch(int key, bool& bFound) const
{
    return BinarySearch(&key, GetData(), GetSize(), sizeof(int), compareInt, bFound);
}

int CIntArray::CheckSum(int idxFirst, int idxLast, int minSum, bool bProportional)
{
    int s = 0;
    int count = idxLast - idxFirst + 1;

    for (int i = idxFirst; i <= idxLast; ++i) s += GetAt(i);

    int diff = minSum - s;
    if (diff <= 0) return s;

    for (int i = idxFirst; i <= idxLast; ++i)
    {
        int val = GetAt(i);
        int d = bProportional ? Rint(double(diff) / s * val) : diff / (idxLast + 1 - i);
        SetAt(i, val + d);
        diff -= d;
        s -= val;
    }

    return minSum;
}

// CUintArray

void CUintArray::Sort()
{
    qsort(GetData(), GetSize(), sizeof(UINT), compareUint);
}

int CUintArray::BSearch(UINT key, bool& bFound) const
{
    return BinarySearch(&key, GetData(), GetSize(), sizeof(UINT), compareUint, bFound);
}

// CInt64Array

void CInt64Array::Sort()
{
    qsort(GetData(), GetSize(), sizeof(int), compareInt64);
}

int CInt64Array::BSearch(INT64 key, bool& bFound) const
{
    return BinarySearch(&key, GetData(), GetSize(), sizeof(INT64), compareInt64, bFound);
}

// CUint64Array

void CUint64Array::Sort()
{
    qsort(GetData(), GetSize(), sizeof(int), compareUint64);
}

int CUint64Array::BSearch(UINT64 key, bool& bFound) const
{
    return BinarySearch(&key, GetData(), GetSize(), sizeof(UINT64), compareUint64, bFound);
}

// CFloatArray

void CFloatArray::Sort()
{
    qsort(GetData(), GetSize(), sizeof(float), compareFloat);
}

int CFloatArray::BSearch(float key, bool& bFound) const
{
    return BinarySearch(&key, GetData(), GetSize(), sizeof(float), compareFloat, bFound);
}

// CDoubleArray

void CDoubleArray::Sort()
{
    qsort(GetData(), GetSize(), sizeof(double), compareDouble);
}

int CDoubleArray::BSearch(double key, bool& bFound) const
{
    return BinarySearch(&key, GetData(), GetSize(), sizeof(double), compareDouble, bFound);
}

void CDoubleArray::GetMeanStdev(double& mean, double& stdev, int count) const
{
    double s = 0;
    double s2 = 0;
    int n = count < 0 ? GetSize() : count;

    for (int i = 0; i < n; ++i)
    {
        double x = GetAt(i);
        s += x;
        s2 += x * x;
    }

    mean = s / n;

    if (n <= 1)
        stdev = Nan;
    else
    {
        double var = (s2 - n * mean * mean) / (n - 1);
        stdev = var <= 0 ? 0 : sqrt(var);
    }
}

double CDoubleArray::GetSkewness() const
{
    double mean = 0;
    double stdev = 0;

    GetMeanStdev(mean, stdev);
    return GetSkewness(mean, stdev);
}

double CDoubleArray::GetSkewness(double mean, double stdev, int count) const
{
    int n = count < 0 ? GetSize() : count;
    double s = 0;

    if (n <= 2) return Nan;

    for (int i = 0; i < n; ++i)
    {
        double xx = (GetAt(i) - mean) / stdev;
        s += xx * xx * xx;
    }

    return (s * n) / (double(n - 1) * (n - 2));
}

double CDoubleArray::GetKurtosis() const
{
    double mean = 0;
    double stdev = 0;

    GetMeanStdev(mean, stdev);
    return GetKurtosis(mean, stdev);
}

double CDoubleArray::GetKurtosis(double mean, double stdev, int count) const
{
    int n = count < 0 ? GetSize() : count;
    double s = 0;

    if (n <= 3) return Nan;

    for (int i = 0; i < n; ++i)
    {
        double xx = (GetAt(i) - mean) / stdev;
        xx *= xx;
        s += xx * xx;
    }

    double kurt = (s * n * (n + 1)) / (double(n - 1) * (n - 2) * (n - 3));
    return kurt - (3. * (n - 1) * (n - 1)) / (double(n - 2) * (n - 3));
}

double CDoubleArray::GetFractile(double f, int count) const
{
    if (count < 0) count = GetSize();
    if (count <= 0) return Nan;

    double index = f * (count - 1);
    int n = int(index);

    return n < 0            ? GetAt(0)
           : n >= count - 1 ? GetAt(count - 1)
                            : GetAt(n) * (n + 1 - index) + GetAt(n + 1) * (index - n);
}
}  // namespace AFLib

int AFLibPrivate::FindHashSize(int size)
{
    static const int szPrimeNumber = sizeof(arrPrimeNumber) / sizeof(arrPrimeNumber[0]);

    size = size * 5 / 4;
    bool bFound = false;
    int index = AFLib::BinarySearch(&size, arrPrimeNumber, szPrimeNumber, sizeof(int), compareInt, bFound);
    return arrPrimeNumber[min(index, szPrimeNumber - 1)];
}
