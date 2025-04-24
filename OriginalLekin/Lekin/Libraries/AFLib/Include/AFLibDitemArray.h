#pragma once

#include "AFLibArrays.h"
#include "AFLibDitem.h"

/////////////////////////////////////////////////////////////////////////////
// TDitemArray
// Array of Ditems

namespace AFLibGui {
class AFLIB TDitemArray : public AFLib::CKeyedStrArray<TDitem>
{
public:
    // change directories for all elements of the array
    void ChangeDirs(LPCTSTR oldPath, LPCTSTR newPath);
};
}  // namespace AFLibGui