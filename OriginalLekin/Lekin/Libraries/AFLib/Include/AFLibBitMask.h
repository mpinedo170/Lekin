#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"
#include "AFLibGlobal.h"

namespace AFLibIO {
class TStringSpacer;
}

/////////////////////////////////////////////////////////////////////////////
// TBitMask
// Regular bit mask, or bit array [x1..x2] (both bounds inclusive)

namespace AFLib {
class AFLIB TBitMask
{
private:
    CUintArray m_arrSel;  // array that keeps the mask

    int m_xFirst;  // left bound
    int m_xLast;   // right bound

    // get mask for a point in array
    void GetOneMask(int x, bool bRight, int& n, UINT& mask) const;

    // the first part of GetMasks
    bool GetMasks1(int x1, int x2, int& n1, int& n2, UINT& mask1, UINT& mask2) const;

    // the second part of GetMasks
    void GetMasks2(int& n1, int& n2, UINT& mask1, UINT& mask2) const;

    // for a given interval, find corresponding array interval and masks
    bool GetMasks(int x1, int x2, int& n1, int& n2, UINT& mask1, UINT& mask2) const;

    // find array interval and masks; extend array if necessary
    bool GetMasksExtend(int x1, int x2, int& n1, int& n2, UINT& mask1, UINT& mask2);

    // shift right (delete bits)
    void ShiftRight(int x, int dx);

    // cut extra elements of m_arrSel
    void CutXLast();

    // remove tail zeros from m_arrSel
    void ClearZeros();

public:
    explicit TBitMask(int xFirst = 0, int xLast = INT_MAX);
    TBitMask(const TBitMask& bm);

    TBitMask& operator=(const TBitMask& bm);
    bool operator==(const TBitMask& bm) const;
    int GetHashKey() const;

    bool operator!=(const TBitMask& bm) const
    {
        return !(*this == bm);
    }

    // clear all bits
    void Clear();

    // set the bounds of the mask
    void SetBounds(int xFirst, int xLast);

    // count the raized bits
    int GetOneCount() const;

    // save in a string
    void Save(AFLibIO::TStringBuffer& sb, bool bSaveLower, bool bSaveHigher) const;

    // load from a string
    void Load(AFLibIO::TStringSpacer& sp, bool bKeepBounds, int xFirstDefault = INT_MAX);

    // insert bits
    void InsertAt(int x, bool bSelected, int count = 1);

    // delete bits
    void DeleteAt(int x, int count = 1);

    // perform a bitwise logical operation with a different bit mask
    void Operation(const TBitMask& bm, TLogicalFunction lfn);

    // perform a logical operation with a string of ones from x1 to x2
    void Operation(int x1, int x2, TLogicalFunction lfn);

    // test if the interval is: bSelected=true -- selected; bSelected=false -- unselected
    bool TestInterval(int x1, int x2, bool bSelected) const;

    // find the first and last raized bits
    bool FindInterval(int& x1, int& x2) const;

    // is a single bit selected?
    bool IsSelected(int x) const;

    // set bits to 1 between x1 and x2
    void SetInterval(int x1, int x2)
    {
        Operation(x1, x2, lfnOr);
    }

    // set bits to 0 between x1 and x2
    void ClearInterval(int x1, int x2)
    {
        Operation(x1, x2, lfnAndN);
    }

    // invert bits between x1 and x2
    void InvertInterval(int x1, int x2)
    {
        Operation(x1, x2, lfnXor);
    }

    // set bit x to 1
    void SetBit(int x)
    {
        Operation(x, x, lfnOr);
    }

    // set bit x to 0
    void ClearBit(int x)
    {
        Operation(x, x, lfnAndN);
    }

    // invert bit x
    void InvertBit(int x)
    {
        Operation(x, x, lfnXor);
    }

    // get starting point
    int GetFirst() const
    {
        return m_xFirst;
    }

    // get ending point
    int GetLast() const
    {
        return m_xLast;
    }

    // get bitmask length
    int GetLength() const
    {
        return m_xLast - m_xFirst + 1;
    }
};
}  // namespace AFLib
