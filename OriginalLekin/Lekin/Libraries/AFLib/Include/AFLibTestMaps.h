#pragma once

#include "AFLibGlobal.h"
#include "AFLibMaps.h"

/////////////////////////////////////////////////////////////////////////////
// CTestMap
// Generic set of elements with insert/remove operations

namespace AFLib {
template <class KEY, class ARG_KEY>
class CTestMap
{
protected:
    typedef typename CSmartMap<KEY, ARG_KEY, int, int>::CPair CPair;

    // the actual map is a private member: no method inheritance
    CSmartMap<KEY, ARG_KEY, int, int> m_map;

    // perform a logical operation with the key
    void PrivateOperation(ARG_KEY key, TLogicalFunction lfn);

public:
    // remove all elements from the set
    void Clear()
    {
        m_map.RemoveAll();
    }

    // insert an element
    void Include(ARG_KEY key)
    {
        m_map.SetAt(key, 1);
    }

    // remove an element
    void Remove(ARG_KEY key)
    {
        m_map.RemoveKey(key);
    }

    // check if an element belons to the set
    bool Check(ARG_KEY key) const
    {
        return m_map.Exists(key);
    }

    // get number of elements in the set
    int GetCount() const
    {
        return m_map.GetCount();
    }

    // is set empty?
    bool IsEmpty() const
    {
        return GetCount() == 0;
    }

    // get starting position for iterations
    POSITION GetStartPosition() const
    {
        if (IsEmpty()) return NULL;
        return POSITION(m_map.PGetFirstAssoc());
    }

    // iterate to the next position
    const KEY& GetNextAssoc(POSITION& rNextPosition) const;

    // perform a logical operation with another set
    void Operation(const CTestMap& map, TLogicalFunction lfn);
};

template <class KEY, class ARG_KEY>
void CTestMap<KEY, ARG_KEY>::PrivateOperation(ARG_KEY key, TLogicalFunction lfn)
{
    switch (lfn)
    {
        case lfnAndN:
            Remove(key);
            break;
        case lfnOr:
            Include(key);
            break;
        case lfnXor:
            if (Check(key))
                Remove(key);
            else
                Include(key);
            break;
    }
}

template <class KEY, class ARG_KEY>
const KEY& CTestMap<KEY, ARG_KEY>::GetNextAssoc(POSITION& rNextPosition) const
{
    const CPair* pPair = reinterpret_cast<const CPair*>(rNextPosition);
    ASSERT(pPair != NULL);

    const CPair* pPairNew = m_map.PGetNextAssoc(pPair);
    rNextPosition = POSITION(pPairNew);
    return pPair->key;
}

template <class KEY, class ARG_KEY>
void CTestMap<KEY, ARG_KEY>::Operation(const CTestMap& map, TLogicalFunction lfn)
{
    if (&map == this)
    {
        if (lfn == lfnAndN || lfn == lfnXor) Clear();
    }
    else if (lfn == lfnAnd)
    {
        CTestMap mapNew;
        for (POSITION pos = map.GetStartPosition(); pos != NULL;)
        {
            const KEY& key = map.GetNextAssoc(pos);
            if (Check(key)) mapNew.Include(key);
        }

        Clear();
        for (POSITION pos = mapNew.GetStartPosition(); pos != NULL;)
        {
            const KEY& key = mapNew.GetNextAssoc(pos);
            Include(key);
        }
    }
    else
    {
        for (POSITION pos = map.GetStartPosition(); pos != NULL;)
        {
            const KEY& key = map.GetNextAssoc(pos);
            PrivateOperation(key, lfn);
        }
    }
}
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CPtrTestMap
// Set of pointers

namespace AFLib {
template <class KEY>
class CPtrTestMap : public CTestMap<KEY*, KEY*>
{
public:
    void Destroy(KEY* pKey)
    {
        Remove(pKey);
        delete pKey;
    }

    void DestroyAll()
    {
        POSITION pos = m_map.GetStartPosition();
        while (pos != NULL)
        {
            KEY* pKey = NULL;
            int value = 0;
            m_map.GetNextAssoc(pos, pKey, value);
            delete pKey;
        }
        Clear();
    }
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CConstPtrTestMap
// Set of const pointers.
// No Destroy's since const pointer can't be deleted.

namespace AFLib {
template <class KEY>
class CConstPtrTestMap : public CTestMap<const KEY*, const KEY*>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSTestMap
// Set of integers

namespace AFLib {
class CITestMap : public CTestMap<int, int>
{};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// CSTestMap
// Set of strings

namespace AFLib {
class CSTestMap : public CTestMap<CString, LPCTSTR>
{};
}  // namespace AFLib
