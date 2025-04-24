#pragma once

#include "AFLibArrays.h"
#include "AFLibStringBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// TIntervalSet
// Set of intervals of any elementary type: intersection, union, lookup

namespace AFLib {
template <class VALUE>
class TIntervalSet
{
public:
    // these should be static, but then the project doesn't link
    const VALUE m_min;  // min value of type VALUE
    const VALUE m_max;  // max value of type VALUE

    // subinterval structure
    struct TPair
    {
        VALUE m_left;
        VALUE m_right;

        TPair()
        {}
        TPair(VALUE left, VALUE right) : m_left(left), m_right(right)
        {}

        // check if value is within the subinterval, or to the left, or to the right
        int compare(VALUE val) const
        {
            return val < m_left ? -1 : val > m_right ? 1 : 0;
        }
    };

protected:
    // result for a lookup of 2 values in the interval set
    struct TLookup
    {
        int m_iLeft;       // subinterval index for the left value
        int m_iRight;      // subinterval index for the right value
        bool m_bFndLeft;   // is the left value within a subinterval? (or between subintervals?)
        bool m_bFndRight;  // is the right value within a subinterval?

        TLookup(const TIntervalSet& iset, VALUE left, VALUE right);
    };

    friend TLookup;

    CArray<TPair> m_data;  // set of subintervals

    // compare a value to a subinterval
    static int _cdecl compareValPair(LPCVOID p1, LPCVOID p2)
    {
        const VALUE* pVal = reinterpret_cast<const VALUE*>(p1);
        const TPair* pPair = reinterpret_cast<const TPair*>(p2);

        return pPair->compare(*pVal);
    }

public:
    // the number of subintervals in the set
    int GetCount() const
    {
        return m_data.GetSize();
    }

    // subinterval with given index
    const TPair& GetPair(int index) const
    {
        return m_data[index];
    }

    // is the interval set empty?
    bool IsEmpty() const
    {
        return GetCount() <= 0;
    }

protected:
    // all constructors are protected
    TIntervalSet(VALUE mn, VALUE mx) : m_min(mn), m_max(mx)
    {}

    TIntervalSet(const TIntervalSet& iset) : m_min(iset.m_min), m_max(iset.m_max)
    {
        m_data.Copy(iset.m_data);
    }

// constructor for cross-types
#pragma warning(push)
#pragma warning(disable : 4018)
    // may generate signed/unsigned warnings in the lines marked with "!!!"
    template <class VALUE2>
    explicit TIntervalSet(const TIntervalSet<VALUE2>& iset, VALUE mn, VALUE mx) : m_min(mn), m_max(mx)
    {
        bool bUnsignedThis = m_min == VALUE(0);
        bool bUnsignedOther = iset.m_min == VALUE2(0);

        for (int i = 0; i < iset.GetCount(); ++i)
        {
            VALUE2 left = iset.GetPair(i).m_left;
            VALUE2 right = iset.GetPair(i).m_right;
            TPair p;
            bool bSet = false;

            if (!bUnsignedOther)
            {  // left may be < 0; need to compare to m_min
                if (bUnsignedThis)
                {  // "this" has no sign: m_min = 0
                    if (left < VALUE2(0))
                    {
                        if (right < VALUE2(0)) continue;
                        p.m_left = VALUE(0);
                        bSet = true;
                    }
                }
                else
                {                      // "this" has sign: m_min < 0!
                    if (left < m_min)  // !!!
                    {
                        if (right < m_min) continue;  // !!!
                        p.m_left = m_min;
                        bSet = true;
                    }
                }
            }

            if (!bSet)
            {  // we can't come here if left < m_min.
                // left may be < 0 when "this" has sign; then can compare left and m_max
                if (left > m_max) break;  // !!!
                p.m_left = VALUE(left);
            }

            // we can't come here if "this" is unsigned and right < 0.
            // otherwise, since m_max > 0, comparison is valid
            if (right > m_max)  // !!!
                p.m_right = m_max;
            else
                p.m_right = VALUE(right);

            m_data.Add(p);
        }
    }
#pragma warning(pop)

    // find a subinterval and whether the value is within it
    int Find(VALUE val, int index, bool& bInside) const
    {
        return BinarySearch(
            &val, m_data.GetData() + index, GetCount() - index, sizeof(TPair), compareValPair, bInside);
    }

    // overridable -- convert value into a string
    virtual void ValToString(VALUE val, AFLibIO::TStringBuffer& s) const = 0;

    // add a subinterval (may merge some intervals; does not check if intervals are "next to each other")
    int SimpleAdd(VALUE left, VALUE right);

    // add a subinterval -- overridable
    virtual void ProtectedAdd(VALUE left, VALUE right)
    {
        SimpleAdd(left, right);
    }

public:
    bool operator==(const TIntervalSet& iset) const
    {
        if (GetCount() != iset.GetCount()) return false;

        for (int i = 0; i < GetCount(); ++i)
        {
            const TPair& p1 = GetPair(i);
            const TPair& p2 = iset.GetPair(i);

            if (p1.m_left != p2.m_left || p1.m_right != p2.m_right) return false;
        }

        return true;
    }

    bool operator!=(const TIntervalSet& iset) const
    {
        return !(*this == iset);
    }

    // add subinterval
    void Add(VALUE left, VALUE right)
    {
        ProtectedAdd(left, right);
    }

    // add single value
    void Add(VALUE val)
    {
        ProtectedAdd(val, val);
    }

    // merge two interval sets
    void Union(const TIntervalSet& iset)
    {
        for (int i = 0; i < iset.GetCount(); ++i)
        {
            const TPair& pair = iset.GetPair(i);
            SimpleAdd(pair.m_left, pair.m_right);
        }
    }

    // intersect two interval sets
    void Intersect(const TIntervalSet& iset);

    // check if value belongs to the interval set
    bool Contains(VALUE val) const
    {
        bool bInside = false;
        Find(val, 0, bInside);
        return bInside;
    }

    // convert interval set to string
    virtual CString ToString() const;
};

// implementation of TLookup constructor
template <class VALUE>
TIntervalSet<VALUE>::TLookup::TLookup(const TIntervalSet& iset, VALUE left, VALUE right)
{
    m_iLeft = iset.Find(left, 0, m_bFndLeft);
    m_iRight = m_iLeft + iset.Find(right, m_iLeft, m_bFndRight);
}

// implementations of TIntervalSet functions
template <class VALUE>
int TIntervalSet<VALUE>::SimpleAdd(VALUE left, VALUE right)
{
    if (right < left) return -1;
    TLookup lk(*this, left, right);

    if (lk.m_iRight == lk.m_iLeft && !lk.m_bFndLeft && !lk.m_bFndRight)
    {
        m_data.InsertAt(lk.m_iLeft, TPair(left, right));
        return lk.m_iLeft;
    }

    if (!lk.m_bFndLeft) m_data[lk.m_iLeft].m_left = left;

    int nKill = lk.m_iRight - lk.m_iLeft;

    if (lk.m_bFndRight)
        m_data[lk.m_iLeft].m_right = m_data[lk.m_iRight].m_right;
    else
    {
        m_data[lk.m_iLeft].m_right = right;
        --nKill;
    }

    ASSERT(nKill >= 0);
    m_data.RemoveAt(lk.m_iLeft + 1, nKill);
    return lk.m_iLeft;
}

template <class VALUE>
void TIntervalSet<VALUE>::Intersect(const TIntervalSet& iset)
{
    int i1 = 0;
    int i2 = 0;
    CArray<TPair> data;

    data.Copy(m_data);
    m_data.RemoveAll();
    m_data.SetSize(0, data.GetSize() + iset.GetCount() - 2);

    while (i1 < data.GetSize() && i2 < iset.GetCount())
    {
        const TPair& p1 = data[i1];
        const TPair& p2 = iset.GetPair(i2);

        if (p1.m_right < p2.m_left)
        {
            ++i1;
            continue;
        }

        if (p1.m_left > p2.m_right)
        {
            ++i2;
            continue;
        }

        TPair p;
        p.m_left = max(p1.m_left, p2.m_left);

        if (p1.m_right < p2.m_right)
        {
            p.m_right = p1.m_right;
            ++i1;
        }
        else
        {
            p.m_right = p2.m_right;
            ++i2;
        }
        m_data.Add(p);
    }
}

template <class VALUE>
CString TIntervalSet<VALUE>::ToString() const
{
    AFLibIO::TStringBuffer s;

    if (GetCount() == 0) return _T("empty");

    for (int i = 0; i < GetCount(); ++i)
    {
        if (i > 0) s += _T("; ");

        ValToString(m_data[i].m_left, s);
        s += _T(" - ");
        ValToString(m_data[i].m_right, s);
    }

    return LPCTSTR(s);
}
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// TDblIntervalSet
// Floating-point interval set

namespace AFLib {
class AFLIB TDblIntervalSet : public TIntervalSet<double>
{
private:
    typedef TIntervalSet<double> super;

    // in order to avoid including H file with DBL_MAX
    static const double m_privateMax;

protected:
    virtual void ValToString(double val, AFLibIO::TStringBuffer& s) const;

public:
    TDblIntervalSet() : super(-m_privateMax, m_privateMax)
    {}

    TDblIntervalSet(const TDblIntervalSet& iset) : super(iset)
    {}

    template <class VALUE2>
    explicit TDblIntervalSet(const TIntervalSet<VALUE2>& iset) : super(iset, -m_privateMax, m_privateMax)
    {}
};
}  // namespace AFLib
