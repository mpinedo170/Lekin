#pragma once

#include "AFLibStrings.h"

namespace AFLibPrivate {  // find a suitable hash table size for an CMap
AFLIB int FindHashSize(int size);
}

/////////////////////////////////////////////////////////////////////////////
// CSmartMap
// Growing hash table

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CSmartMap : public CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>
{
private:
    typedef CMap<KEY, ARG_KEY, VALUE, ARG_VALUE> super;

    // take all elements under pAssoc, put them in the table
    void RehashBucket(CAssoc* pAssoc);

public:
    explicit CSmartMap(int nBlockSize = 1 << 6) : super(nBlockSize)
    {}

    // prepare to add n elements; grow hash table if too small
    void GrowBy(int n);

    // key present in the table?
    bool Exists(ARG_KEY key) const
    {
        VALUE value;
        return Lookup(key, value) != 0;
    }

    // put element in map
    void SetAt(ARG_KEY key, ARG_VALUE newValue)
    {
        super::SetAt(key, newValue);
        GrowBy(0);
    }
};

template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CSmartMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RehashBucket(CAssoc* pAssoc)
{
    struct CAssoc2 : public CPair
    {
        CAssoc2* pNext;
        UINT nHashValue;
    };

    for (CAssoc2* pAssoc2 = reinterpret_cast<CAssoc2*>(pAssoc); pAssoc2 != NULL;)
    {
        int nBucket = pAssoc2->nHashValue % m_nHashTableSize;
        CAssoc2* pAssocThis = pAssoc2;
        pAssoc2 = pAssoc2->pNext;

        pAssocThis->pNext = reinterpret_cast<CAssoc2*>(m_pHashTable[nBucket]);
        m_pHashTable[nBucket] = reinterpret_cast<CAssoc*>(pAssocThis);
    }
}

template <class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CSmartMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GrowBy(int n)
{
    int oldSize = m_nHashTableSize;
    int newSize = GetCount() + n;
    if (newSize < oldSize * 4 / 5) return;

    newSize = AFLibPrivate::FindHashSize(newSize);
    if (newSize == oldSize) return;

    CAssoc** pHashTable = m_pHashTable;
    m_nHashTableSize = newSize;
    m_pHashTable = new CAssoc*[m_nHashTableSize];

    if (m_pHashTable != NULL)
    {
        memset(m_pHashTable, 0, sizeof(CAssoc*) * m_nHashTableSize);

        if (GetCount() > 0)
        {
            for (int nBucket = 0; nBucket < oldSize; ++nBucket) RehashBucket(pHashTable[nBucket]);
        }
    }

    delete[] pHashTable;
}
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CPointerMap
// Hash table of pointers

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE>
class CPointerMap : public CSmartMap<KEY, ARG_KEY, VALUE*, VALUE*>
{
private:
    typedef CSmartMap<KEY, ARG_KEY, VALUE*, VALUE*> super;

public:
    // get value at key; NULL if not in map
    VALUE* GetAt(ARG_KEY key) const
    {
        VALUE* pValue = NULL;
        return Lookup(key, pValue) ? pValue : NULL;
    }

    // find key, remove, call destructor
    void DestroyAt(ARG_KEY key)
    {
        VALUE* pValue = GetAt(key);
        delete pValue;
        RemoveKey(key);
    }

    // clear map, call destructors on elements
    void DestroyAll()
    {
        POSITION pos = GetStartPosition();
        while (pos != NULL)
        {
            KEY key;
            VALUE* pValue = NULL;
            GetNextAssoc(pos, key, pValue);
            delete pValue;
        }
        RemoveAll();
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CConstPointerMap
// Hash table of const pointers
// No Destroy's since const pointer can't be deleted

namespace AFLib {
template <class KEY, class ARG_KEY, class VALUE>
class CConstPointerMap : public CSmartMap<KEY, ARG_KEY, const VALUE*, const VALUE*>
{
private:
    typedef CSmartMap<KEY, ARG_KEY, const VALUE*, const VALUE*> super;

public:
    // get value at key; NULL if not in map
    const VALUE* GetAt(ARG_KEY key) const
    {
        const VALUE* pValue = NULL;
        return Lookup(key, pValue) ? pValue : NULL;
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CP2PMap
// CPointerMap where key is also a pointer

namespace AFLib {
template <class KEY, class VALUE>
class CP2PMap : public CPointerMap<const KEY*, const KEY*, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CS2PMapA
// CPointerMap with CStringA key

namespace AFLib {
template <class VALUE>
class CS2PMapA : public CPointerMap<CStringA, LPCSTR, VALUE>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CS2PMapW
// CPointerMap with CStringW key

namespace AFLib {
template <class VALUE>
class CS2PMapW : public CPointerMap<CStringW, LPCWSTR, VALUE>
{};
}  // namespace AFLib

#ifdef _UNICODE
#define CS2PMap CS2PMapW
#else
#define CS2PMap CS2PMapA
#endif

/////////////////////////////////////////////////////////////////////////////
// CS2SMap
// String to string map

namespace AFLib {
class CS2SMap : public CSmartMap<CString, LPCTSTR, CString, LPCTSTR>
{
private:
    typedef CSmartMap<CString, LPCTSTR, CString, LPCTSTR> super;

public:
    // get value at key; default value if not in map
    CString GetAt(LPCTSTR key, LPCTSTR def = strEmpty)
    {
        CString value;
        return Lookup(key, value) ? value : def;
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CIntMap
// Map of integers

namespace AFLib {
template <class KEY, class ARG_KEY>
class CIntMap : public CSmartMap<KEY, ARG_KEY, int, int>
{
private:
    typedef CSmartMap<KEY, ARG_KEY, int, int> super;

public:
    // get value at key; default value if not in map
    int GetAt(ARG_KEY key, int def = -1) const
    {
        int a = def;
        Lookup(key, a);
        return a;
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CP2IntMap
// Pointer to int map

namespace AFLib {
template <class KEY>
class CP2IntMap : public CIntMap<const KEY*, const KEY*>
{};
}  // namespace AFLib
