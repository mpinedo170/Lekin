#pragma once

#include "AFLibMaps.h"

/////////////////////////////////////////////////////////////////////////////
// TDBField
// A string with integer identifier.
// Speeds up recordset data access by field name.

namespace AFLibDB {
class AFLIB TDBField
{
private:
    CString m_str;  // name of the field
    int m_ID;       // system-wide field ID

    // map of known fields
    static AFLib::CIntMap<CString, LPCTSTR> m_mapID;

    // global ID (increment)
    static int m_IDGlobal;

public:
    explicit TDBField(LPCTSTR str);

    operator const CString&() const
    {
        return m_str;
    }

    // explicit conversion into string
    const CString& S() const
    {
        return m_str;
    }

    // get the ID
    int GetID() const
    {
        return m_ID;
    }

    // get the ID for a given field
    static int GetFieldID(LPCTSTR str)
    {
        return m_mapID.GetAt(str);
    }
};
}  // namespace AFLibDB
