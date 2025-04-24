#pragma once

#include "AFLibPointers.h"

/////////////////////////////////////////////////////////////////////////////
// CIntArray
// Integer array

namespace AFLib {
class AFLIB CIntArray : public CArray<int, int>
{
public:
    // check the sum between idxFirst and idxLast; if sum < minSum, pad each element
    int CheckSum(int idxFirst, int idxLast, int minSum, bool bProportional);

    // sort in ascending order
    void Sort();

    // search in a sorted array
    int BSearch(int key, bool& bFound) const;
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CUintArray
// Unsigned integer array

namespace AFLib {
class AFLIB CUintArray : public CArray<UINT, UINT>
{
public:
    // sort in ascending order
    void Sort();

    // search in a sorted array
    int BSearch(UINT key, bool& bFound) const;
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CInt64Array
// 64-bit integer array

namespace AFLib {
class AFLIB CInt64Array : public CArray<INT64, INT64>
{
public:
    // sort in ascending order
    void Sort();

    // search in a sorted array
    int BSearch(INT64 key, bool& bFound) const;
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CUint64Array
// 64-bit integer array

namespace AFLib {
class AFLIB CUint64Array : public CArray<UINT64, UINT64>
{
public:
    // sort in ascending order
    void Sort();

    // search in a sorted array
    int BSearch(UINT64 key, bool& bFound) const;
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CFloatArray
// Floating-point array

namespace AFLib {
class AFLIB CFloatArray : public CArray<float, float>
{
public:
    // sort in ascending order
    void Sort();

    // search in a sorted array
    int BSearch(float key, bool& bFound) const;
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CDoubleArray
// Double-precision floating-point array

namespace AFLib {
class AFLIB CDoubleArray : public CArray<double, double>
{
public:
    // sort in ascending order
    void Sort();

    // search in a sorted array
    int BSearch(double key, bool& bFound) const;

    // compute the mean and standard deviation of the array or part of the array
    void GetMeanStdev(double& mean, double& stdev, int count = -1) const;

    // get skewness of the array
    double GetSkewness() const;

    // get skewness of the array or part of the array (mean and stdev are known)
    double GetSkewness(double mean, double stdev, int count = -1) const;

    // get kurtosis of the array
    double GetKurtosis() const;

    // get kurtosis of the array or part of the array (mean and stdev are known)
    double GetKurtosis(double mean, double stdev, int count = -1) const;

    // get fractile in the sorted array or part of the array (0 <= f <= 1)
    double GetFractile(double f, int count = -1) const;

    // get median
    double GetMedian(int count = -1) const
    {
        return GetFractile(0.5, count);
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// Search functions

namespace AFLib {  // binary search -- unlike standard, finds index even if element is not present
AFLIB int BinarySearch(
    LPCVOID key, LPCVOID base, int num, int width, int(_cdecl* compare)(LPCVOID, LPCVOID), bool& bFound);

// sort strings in accending alphabetical order
AFLIB void SortStrings(CStringArray& arr);

// search in a sorted array of strings
AFLIB int BSearchStrings(LPCTSTR key, const CStringArray& arr, bool& bFound);

// search in a sorted array of strings; ignore lettercase
AFLIB int BSearchStringsIC(LPCTSTR key, const CStringArray& arr, bool& bFound);

// standard "compare" functions applicable in most cases
AFLIB int CompareGL(int key1, int key2);
AFLIB int CompareGL(UINT key1, UINT key2);
AFLIB int CompareGL(double key1, double key2);
AFLIB int CompareGL(LPCTSTR key1, LPCTSTR key2);
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSmartArray
// Unsorted array of pointers.
// Calling destructors on elements is NOT automatic.

namespace AFLib {
template <class VALUE>
class CSmartArray : public CTypedPtrArray<CPtrArray, VALUE*>
{
private:
    typedef public CTypedPtrArray<CPtrArray, VALUE*> super;

public:
    typedef int (*TCompareFunc)(const VALUE*, const VALUE*);

    static const VALUE* const BadPointer;

private:
    static __declspec(thread) TCompareFunc m_pFunc;

    static int compare(LPCVOID a1, LPCVOID a2)
    {
        return (*m_pFunc)(*(const VALUE**)a1, *(const VALUE**)a2);
    }

public:
    const VALUE* GetAt(int index) const
    {
        return super::GetAt(index);
    }

    VALUE*& GetAt(int index)
    {
        return super::ElementAt(index);
    }

    const VALUE* operator[](int index) const
    {
        return super::GetAt(index);
    }

    VALUE*& operator[](int index)
    {
        return super::ElementAt(index);
    }

    VALUE** GetData()
    {
        return reinterpret_cast<VALUE**>(super::GetData());
    }

    const VALUE* const* GetData() const
    {
        return reinterpret_cast<const VALUE* const*>(super::GetData());
    }

    // find element in array
    virtual int FindExact(const VALUE* pValue) const
    {
        for (int i = 0; i < GetSize(); ++i)
            if (GetAt(i) == pValue) return i;
        return -1;
    }

    // remove all elements (call destructors)
    void DestroyAll()
    {
        for (int i = GetSize(); --i >= 0;) delete GetAt(i);
        RemoveAll();
    }

    // remove some elements (call destructors)
    void DestroyAt(int index, int count = 1)
    {
        if (count <= 0) return;
        for (int i = min(index + count, GetSize()); --i >= index;) delete GetAt(index);
        RemoveAt(index, count);
    }

    // find and destroy
    void DestroyExact(VALUE* pValue)
    {
        int index = FindExact(pValue);
        if (index >= 0) RemoveAt(index);
        delete pValue;
    }

    // find and remove (do not call descructor)
    void RemoveExact(const VALUE* pValue)
    {
        int index = FindExact(pValue);
        if (index >= 0) RemoveAt(index);
    }

    // remove all null elements
    void RemoveNulls()
    {
        for (int i = GetSize(); --i >= 0;)
            if (GetAt(i) == NULL) RemoveAt(i);
    }

    // sort according to given function
    void SortByElement(TCompareFunc pFunc)
    {
        m_pFunc = pFunc;
        qsort(GetData(), GetSize(), sizeof(VALUE*), compare);
        m_pFunc = NULL;
    }
};

template <class VALUE>
const VALUE* const CSmartArray<VALUE>::BadPointer = (VALUE*)0xCDCDCDCD;

template <class VALUE>
typename CSmartArray<VALUE>::TCompareFunc CSmartArray<VALUE>::m_pFunc = NULL;
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CConstArray
// Unsorted array of const pointers.

namespace AFLib {
template <class VALUE>
class CConstArray : public CArray<const VALUE*, const VALUE*>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CKeyedArray
// Keyed array of pointers.  Element is supposed to have GetGLKey() function.
// Has Sort, linear search.  Sorting is NOT done by default.

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE>
class CKeyedArray : public CSmartArray<VALUE>
{
private:
    typedef public CSmartArray<VALUE> super;

protected:
    // compare function for sorting
    static int _cdecl compareKeys(LPCVOID p1, LPCVOID p2)
    {
        const VALUE* pValue1 = *reinterpret_cast<const VALUE* const*>(p1);
        const VALUE* pValue2 = *reinterpret_cast<const VALUE* const*>(p2);

        return pValue1 == NULL   ? (pValue2 == NULL ? 0 : -1)
               : pValue2 == NULL ? 1
                                 : CompareGL(pValue1->GetGLKey(), pValue2->GetGLKey());
    }

public:
    // linear search -- find key in array, return index
    int LSearch(ARG_KEY key, const VALUE* pExclude = BadPointer) const
    {
        for (int i = 0; i < GetSize(); ++i)
        {
            const VALUE* pValue = GetAt(i);
            if (pValue != NULL && pValue != pExclude && CompareGL(key, pValue->GetGLKey()) == 0) return i;
        }
        return -1;
    }

protected:
    // overridable search function
    virtual int ProtectedFindKeyIndex(ARG_KEY key, const VALUE* pExclude) const
    {
        return LSearch(key, pExclude);
    }

public:
    // find key, return const element
    const VALUE* FindKey(ARG_KEY key, const VALUE* pExclude = BadPointer) const
    {
        int index = ProtectedFindKeyIndex(key, pExclude);
        return index >= 0 ? GetAt(index) : NULL;
    }

    // find key, return element
    VALUE* FindKey(ARG_KEY key, const VALUE* pExclude = BadPointer)
    {
        int index = ProtectedFindKeyIndex(key, pExclude);
        return index >= 0 ? GetAt(index) : NULL;
    }

    // sort array by key
    void SortByKey()
    {
        qsort(GetData(), GetSize(), sizeof(VALUE*), compareKeys);
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CKeyedIntArray
// Keyed array with integer key

namespace AFLib {
template <class VALUE>
class CKeyedIntArray : public CKeyedArray<int, int, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CKeyedUintArray
// Keyed array with unsigned integer key

namespace AFLib {
template <class VALUE>
class CKeyedUintArray : public CKeyedArray<UINT, UINT, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CKeyedDblArray
// Keyed array with double key

namespace AFLib {
template <class VALUE>
class CKeyedDblArray : public CKeyedArray<double, double, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CKeyedStrArray
// Keyed array with string key

namespace AFLib {
template <class VALUE>
class CKeyedStrArray : public CKeyedArray<CString, LPCTSTR, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSortedArray
// Sorted array of pointers.  Binary search

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE>
class CSortedArray : public CKeyedArray<KEY, ARG_KEY, VALUE>
{
private:
    typedef public CKeyedArray<KEY, ARG_KEY, VALUE> super;

protected:
    // compare key and element (handles null elements)
    static int _cdecl compareKeyElem(LPCVOID p1, LPCVOID p2)
    {
        const KEY* pKey = reinterpret_cast<const KEY*>(p1);
        const VALUE* pT2 = *reinterpret_cast<const VALUE* const*>(p2);

        return pT2 == NULL ? 1 : CompareGL(*pKey, pT2->GetGLKey());
    }

    // binary search
    int ProtectedBSearch(ARG_KEY key, bool& bFound) const
    {
        return BinarySearch(&key, GetData(), GetSize(), sizeof(VALUE*), compareKeyElem, bFound);
    }

    // find key; look around for the exact match with pFind
    // if pFind == BadPointer, look for element != pExclude
    int LookAround(ARG_KEY key, const VALUE* pFind, const VALUE* pExclude) const;

public:
    // find index for pValue -- overridden -- does binary search
    virtual int FindExact(const VALUE* pValue) const
    {
        if (pValue != NULL) return LookAround(pValue->GetGLKey(), pValue, BadPointer);
        return !IsEmpty() && GetAt(0) == NULL ? 0 : -1;
    }

    // insert in order of keys
    void Insert(VALUE* pValue)
    {
        bool bFound = false;
        int index = pValue == NULL ? 0 : ProtectedBSearch(pValue->GetGLKey(), bFound);
        InsertAt(index, pValue);
    }

    // binary search -- find index
    int BSearch(ARG_KEY key, const VALUE* pExclude = BadPointer) const
    {
        return LookAround(key, BadPointer, pExclude);
    }

protected:
    virtual int ProtectedFindKeyIndex(ARG_KEY key, const VALUE* pExclude) const
    {
        return BSearch(key, pExclude);
    }
};

template <class KEY, class ARG_KEY, class VALUE>
int CSortedArray<KEY, ARG_KEY, VALUE>::LookAround(
    ARG_KEY key, const VALUE* pFind, const VALUE* pExclude) const
{
    bool bFound = false;
    int index = ProtectedBSearch(key, bFound);
    if (!bFound) return -1;

    for (int i = index; i < GetSize(); ++i)
    {
        const VALUE* pValue = GetAt(i);
        if (pValue != pExclude && (pFind == BadPointer || pFind == pValue)) return i;
        if (CompareGL(pValue->GetGLKey(), key) > 0) break;
    }

    for (int i = index; --i >= 0;)
    {
        const VALUE* pValue = GetAt(i);
        if (pValue != pExclude && (pFind == BadPointer || pFind == pValue)) return i;
        if (CompareGL(pValue->GetGLKey(), key) < 0) break;
    }

    return -1;
}
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSortedIntArray
// Sorted array with integer key

namespace AFLib {
template <class VALUE>
class CSortedIntArray : public CSortedArray<int, int, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSortedUintArray
// Sorted array with unsigned integer key

namespace AFLib {
template <class VALUE>
class CSortedUintArray : public CSortedArray<UINT, UINT, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSortedDblArray
// Sorted array with double key

namespace AFLib {
template <class VALUE>
class CSortedDblArray : public CSortedArray<double, double, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSortedStrArray
// Sorted array with string key

namespace AFLib {
template <class VALUE>
class CSortedStrArray : public CSortedArray<CString, LPCTSTR, VALUE>
{};
}  // namespace AFLib
