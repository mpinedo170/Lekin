#pragma once

#include "AFLibArrays.h"

/////////////////////////////////////////////////////////////////////////////
// CHeap
// Heap structure: contains pointers;
//   insert in O(log(n)), remove in O(log(n)), find min in O(1).
// Also provides CKeyedArray const functions.

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE>
class CHeap
{
private:
    CKeyedArray<KEY, ARG_KEY, VALUE> m_arrData;  // contents

public:
    // compare elements; default -- by key, override if necessary
    virtual int CompareElements(const VALUE* pT1, const VALUE* pT2) const
    {
        return CompareGL(pT1->GetGLKey(), pT2->GetGLKey());
    }

    // called when elem is moved in heap
    virtual void SetIndex(VALUE* pValue, int index)
    {}

protected:
    // compare 2 elements already in the heap
    int Compare(int index1, int index2) const
    {
        return CompareElements(m_arrData[index1], m_arrData[index2]);
    }

    // swap 2 elements in the heap
    void Swap(int index1, int index2)
    {
        if (index1 == index2) return;
        VALUE* pValue = m_arrData[index1];
        m_arrData[index1] = m_arrData[index2];
        m_arrData[index2] = pValue;
        SetIndex(m_arrData[index1], index1);
        SetIndex(m_arrData[index2], index2);
    }

    // compare element with the 2 "below" it, swap and repeat if necessary
    int Sink(int index)
    {
        while (true)
        {
            int indexNew = index * 2;
            if (indexNew >= GetSize()) return index;

            if (indexNew < GetSize() - 1 && Compare(indexNew + 1, indexNew) <= 0) ++indexNew;

            if (Compare(index, indexNew) <= 0) return index;

            Swap(index, indexNew);
            index = indexNew;
        }
    }

    // compare element with one "above" it, swap and repeat if necessary
    int Lift(int index)
    {
        while (index > 0)
        {
            int indexNew = index / 2;
            if (Compare(indexNew, index) <= 0) return index;
            Swap(index, indexNew);
            index = indexNew;
        }
        return index;
    }

public:
    // clear the heap
    void RemoveAll()
    {
        m_arrData.RemoveAll();
    }

    // clear the heap, call destructors on elements
    void DestroyAll()
    {
        m_arrData.DestroyAll();
    }

    // clear (do not call destructors); copy another heap
    void Copy(const CHeap<KEY, ARG_KEY, VALUE>& src)
    {
        m_arrData.Copy(src.m_arrData);
    }

    // simple linear search (returns index)
    int LSearch(ARG_KEY key) const
    {
        return m_arrData.LSearch(key);
    }

    // simple linear search (returns element)
    const VALUE* FindKey(ARG_KEY key) const
    {
        return m_arrData.FindKey(key);
    }

    // simple linear search for an element
    int FindExact(const VALUE* pValue) const
    {
        return m_arrData.FindExact(pValue);
    }

    // get element at index
    const VALUE* GetAt(int index) const
    {
        return m_arrData[index];
    }

    // get element at index
    const VALUE* operator[](int index) const
    {
        return m_arrData[index];
    }

    // get size of the heap
    int GetSize() const
    {
        return m_arrData.GetSize();
    }

    // is heap empty?
    bool IsEmpty() const
    {
        return m_arrData.IsEmpty() != 0;
    }

    // lift or sink an element; call if element's state changed
    int Adjust(int index)
    {
        return index > 0 && Compare(index, index / 2) < 0 ? Lift(index) : Sink(index);
    }

    // add element to heap
    void Add(VALUE* pValue)
    {
        int index = GetSize();
        m_arrData.Add(pValue);
        SetIndex(pValue, index);
        Lift(index);
    }

    // remove arbitrary element from heap
    void RemoveAt(int index)
    {
        if (index >= GetSize()) return;

        if (index == GetSize() - 1)
        {
            m_arrData.SetSize(GetSize() - 1);
            return;
        }

        m_arrData[index] = m_arrData[GetSize() - 1];
        m_arrData.SetSize(GetSize() - 1);
        Sink(index);
    }

    // remove, call destructor
    void DestroyAt(int index)
    {
        if (index >= GetSize()) return;
        VALUE* pValue = m_arrData[index];
        RemoveAt(index);
        delete pValue;
    }

    // find and remove
    void RemoveExact(const VALUE* pValue)
    {
        int index = FindExact(pValue);
        if (index >= 0) RemoveAt(index);
    }

    // find and remove, call destructor
    void DestroyExact(VALUE* pValue)
    {
        int index = FindExact(pValue);
        if (index >= 0) RemoveAt(index);
        delete pValue;
    }

    // get lightest element, remove from heap
    VALUE* Pop()
    {
        VALUE* pValue = m_arrData[0];
        RemoveAt(0);
        return pValue;
    }

    // destroy lightest elemet in heap
    void DestroyPop()
    {
        DestroyAt(0);
    }

    // append an array of elements
    void Append(const CSmartArray<VALUE>& arr)
    {
        for (int i = 0; i < arr.GetSize(); ++i) Add(arr[i]);
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CIntHeap
// Heap with integer key

namespace AFLib {
template <class VALUE>
class CIntHeap : public CHeap<int, int, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CUintHeap
// Heap with unsigned integer key

namespace AFLib {
template <class VALUE>
class CUintHeap : public CHeap<UINT, UINT, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CDblHeap
// Heap with double key

namespace AFLib {
template <class VALUE>
class CDblHeap : public CHeap<double, double, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CStrHeap
// Heap with string key

namespace AFLib {
template <class VALUE>
class CStrHeap : public CHeap<CString, LPCTSTR, VALUE>
{};
}  // namespace AFLib
