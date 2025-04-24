#pragma once

#include "AFLibArrays.h"
#include "AFLibMaps.h"

/////////////////////////////////////////////////////////////////////////////
// CCollection
// Unsorted array with fast access (a combination of an array and a map)

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE>
class CCollection
{
private:
    CKeyedArray<KEY, ARG_KEY, VALUE> m_arr;  // underlying array
    CPointerMap<KEY, ARG_KEY, VALUE> m_map;  // fast access map

protected:
    // add value to the map; check for duplicates
    void CheckAndAdd(VALUE* pValue)
    {
        if (pValue == NULL) return;
        ASSERT(m_map.GetAt(pValue->GetGLKey()) == NULL);
        m_map.SetAt(pValue->GetGLKey(), pValue);
    }

public:
    // number of items in collection
    int GetSize() const
    {
        return m_arr.GetSize();
    }

    // number of not-null items
    int GetNotNullCount() const
    {
        return m_map.GetCount();
    }

    // const accessor by index
    const VALUE* operator[](int index) const
    {
        return m_arr[index];
    }

    // accessor by index
    VALUE* operator[](int index)
    {
        return m_arr[index];
    }

    // const accessor by key
    const VALUE* FindKey(ARG_KEY key) const
    {
        return m_map.GetAt(key);
    }

    // accessor by key
    VALUE* FindKey(ARG_KEY key)
    {
        return m_map.GetAt(key);
    }

    // linear search for an exact match
    int FindExact(const VALUE* pValue) const
    {
        return m_arr.FindExact(pValue);
    }

    // sort array
    void SortByKey()
    {
        m_arr.SortByKey();
    }

    // add element
    void Add(VALUE* pValue)
    {
        CheckAndAdd(pValue);
        m_arr.Add(pValue);
    }

    // insert element
    void InsertAt(int index, VALUE* pValue)
    {
        CheckAndAdd(pValue);
        m_arr.InsertAt(index, pValue);
    }

    // replace element
    void ReplaceAt(int index, VALUE* pReplace)
    {
        VALUE* pValue = m_arr[index];
        if (pValue == pReplace) return;

        if (pValue != NULL) m_map.RemoveKey(pValue->GetGLKey());

        CheckAndAdd(pReplace);
        m_arr[index] = pReplace;
    }

    // remove element
    void RemoveAt(int index)
    {
        ReplaceAt(index, NULL);
        m_arr.RemoveAt(index);
    }

    // remove element, free memory
    void DestroyAt(int index)
    {
        VALUE* pValue = m_arr[index];
        RemoveAt(index);
        delete pValue;
    }

    // replace element, delete the old one
    void DestroyReplaceAt(int index, VALUE* pReplace)
    {
        VALUE* pValue = m_arr[index];
        ReplaceAt(index, pReplace);
        delete pValue;
    }

    // remove all elements
    void RemoveAll()
    {
        m_map.RemoveAll();
        m_arr.RemoveAll();
    }

    // remove all elements, free memory
    void DestroyAll()
    {
        m_map.RemoveAll();
        m_arr.DestroyAll();
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CIntCollection
// Collection with integer key

namespace AFLib {
template <class VALUE>
class CIntCollection : public CCollection<int, int, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CUintCollection
// Collection with unsigned integer key

namespace AFLib {
template <class VALUE>
class CUintCollection : public CCollection<UINT, UINT, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CDblCollection
// Collection with double key

namespace AFLib {
template <class VALUE>
class CDblCollection : public CCollection<double, double, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CStrCollection
// Collection with string key

namespace AFLib {
template <class VALUE>
class CStrCollection : public CCollection<CString, LPCTSTR, VALUE>
{};
}  // namespace AFLib
