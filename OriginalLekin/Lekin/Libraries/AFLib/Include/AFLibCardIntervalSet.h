#pragma once

#include "AFLibIntervalSet.h"

/////////////////////////////////////////////////////////////////////////////
// TCardIntervalSet
// Interval set for cardinal types: -- and ++ are defined for VALUE.
// Inversion can be defined here.

namespace AFLib {
template <class VALUE>
class TCardIntervalSet : public TIntervalSet<VALUE>
{
private:
    typedef TIntervalSet<VALUE> super;

protected:
    // for a set of subintervals, check if and two are right next to each other
    void CheckEnds(int iFirst, int iLast);

    TCardIntervalSet(VALUE mn, VALUE mx) : super(mn, mx)
    {}

    TCardIntervalSet(const TCardIntervalSet& iset) : super(iset)
    {}

    template <class VALUE2>
    explicit TCardIntervalSet(const TIntervalSet<VALUE2>& iset, VALUE mn, VALUE mx) : super(iset, mn, mx)
    {
        CheckEnds(1, GetCount() - 1);
    }

    // ProtectedAdd() now checks the ends
    virtual void ProtectedAdd(VALUE left, VALUE right)
    {
        int index = SimpleAdd(left, right);
        if (index >= 0) CheckEnds(index, index + 1);
    }

    // remove an interval
    virtual void ProtectedRemove(VALUE left, VALUE right);

public:
    // remove subinterval
    void Remove(VALUE left, VALUE right)
    {
        ProtectedRemove(left, right);
    }

    // remove single value
    void Remove(VALUE val)
    {
        ProtectedRemove(val, val);
    }

    // Union checks interval ends
    void Union(const TCardIntervalSet& iset)
    {
        super::Union(iset);
        CheckEnds(1, GetCount() - 1);
    }

    // invert the entire interval set
    void Invert();
};

// implementations of TCardIntervalSet functions
template <class VALUE>
void TCardIntervalSet<VALUE>::CheckEnds(int iFirst, int iLast)
{
    iLast = min(iLast, GetCount() - 1);
    iFirst = max(iFirst, 1);

    for (int i = iLast; i >= iFirst; --i)
    {
        VALUE val = m_data[i].m_left;
        if (val <= m_min) break;

        --val;
        if (val > m_data[i - 1].m_right) continue;

        m_data[i - 1].m_right = m_data[i].m_right;
        m_data.RemoveAt(i);
    }
}

template <class VALUE>
void TCardIntervalSet<VALUE>::ProtectedRemove(VALUE left, VALUE right)
{
    if (right < left) return;
    TLookup lk(*this, left, right);

    if (lk.m_iLeft >= GetCount() || (lk.m_iRight == 0 && !lk.m_bFndRight)) return;

    if (lk.m_bFndLeft && lk.m_bFndRight && lk.m_iLeft == lk.m_iRight && left > m_data[lk.m_iLeft].m_left &&
        right < m_data[lk.m_iLeft].m_right)
    {  // removal splits subperiod in two
        TPair p(right, m_data[lk.m_iLeft].m_right);

        ++p.m_left;
        m_data.InsertAt(lk.m_iLeft + 1, p);

        m_data[lk.m_iLeft].m_right = left;
        --m_data[lk.m_iLeft].m_right;
        return;
    }

    int firstKill = lk.m_iLeft + 1;
    int lastKill = lk.m_iRight - 1;

    if (lk.m_bFndLeft)
    {
        if (left == m_data[lk.m_iLeft].m_left)
            --firstKill;
        else
        {  // part of interval # m_iLeft stays
            m_data[lk.m_iLeft].m_right = left;
            --m_data[lk.m_iLeft].m_right;
        }
    }

    if (lk.m_bFndRight)
    {
        if (right == m_data[lk.m_iRight].m_right)
            ++lastKill;
        else
        {  // part of interval # m_iRight stays
            m_data[lk.m_iRight].m_left = right;
            ++m_data[lk.m_iRight].m_left;
        }
    }

    if (lastKill >= firstKill) m_data.RemoveAt(firstKill, lastKill - firstKill + 1);
}

template <class VALUE>
void TCardIntervalSet<VALUE>::Invert()
{
    if (IsEmpty())
    {
        m_data.Add(TPair(m_min, m_max));
        return;
    }

    CArray<TPair> data;
    data.SetSize(0, GetCount() + 1);

    VALUE firstLeft = m_data[0].m_left;
    if (firstLeft > m_min)
    {
        --firstLeft;
        data.Add(TPair(m_min, firstLeft));
    }

    for (int i = 1; i < GetCount(); ++i)
    {
        TPair p(m_data[i - 1].m_right, m_data[i].m_left);
        ++p.m_left;
        --p.m_right;
        data.Add(p);
    }

    VALUE lastRight = m_data[GetCount() - 1].m_right;
    if (lastRight < m_max)
    {
        ++lastRight;
        data.Add(TPair(lastRight, m_max));
    }

    m_data.Copy(data);
}
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// TCharIntervalSet
// ASCII character interval set

namespace AFLib {
class AFLIB TCharIntervalSet : public TCardIntervalSet<CHAR>
{
private:
    typedef TCardIntervalSet<CHAR> super;

protected:
    virtual void ValToString(CHAR val, AFLibIO::TStringBuffer& s) const;

public:
    TCharIntervalSet() : super(CHAR_MIN, CHAR_MAX)
    {}

    TCharIntervalSet(const TCharIntervalSet& iset) : super(iset)
    {}

    template <class VALUE2>
    explicit TCharIntervalSet(const TIntervalSet<VALUE2>& iset) : super(iset, CHAR_MIN, CHAR_MAX)
    {}
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// TWCharIntervalSet
// Unicode character interval set

namespace AFLib {
class AFLIB TWCharIntervalSet : public TCardIntervalSet<WCHAR>
{
private:
    typedef TCardIntervalSet<WCHAR> super;

protected:
    virtual void ValToString(WCHAR val, AFLibIO::TStringBuffer& s) const;

public:
    TWCharIntervalSet() : super(WCHAR_MIN, WCHAR_MAX)
    {}

    TWCharIntervalSet(const TWCharIntervalSet& iset) : super(iset)
    {}

    template <class VALUE2>
    explicit TWCharIntervalSet(const TIntervalSet<VALUE2>& iset) : super(iset, WCHAR_MIN, WCHAR_MAX)
    {}
};
}  // namespace AFLib

#ifdef _UNICODE
#define TTCharIntervalSet TWCharIntervalSet
#else
#define TTCharIntervalSet TCharIntervalSet
#endif

/////////////////////////////////////////////////////////////////////////////
// TIntIntervalSet
// Integer interval set

namespace AFLib {
class AFLIB TIntIntervalSet : public TCardIntervalSet<int>
{
private:
    typedef TCardIntervalSet<int> super;

protected:
    virtual void ValToString(int val, AFLibIO::TStringBuffer& s) const;

public:
    TIntIntervalSet() : super(INT_MIN, INT_MAX)
    {}

    TIntIntervalSet(const TIntIntervalSet& iset) : super(iset)
    {}

    template <class VALUE2>
    explicit TIntIntervalSet(const TIntervalSet<VALUE2>& iset) : super(iset, INT_MIN, INT_MAX)
    {}
};
}  // namespace AFLib

/////////////////////////////////////////////////////////////////////////////
// TUintIntervalSet
// Unsigned integer interval set

namespace AFLib {
class AFLIB TUintIntervalSet : public TCardIntervalSet<UINT>
{
private:
    typedef TCardIntervalSet<UINT> super;

protected:
    virtual void ValToString(UINT val, AFLibIO::TStringBuffer& s) const;

public:
    TUintIntervalSet() : super(0, UINT_MAX)
    {}

    TUintIntervalSet(const TUintIntervalSet& iset) : super(iset)
    {}

    template <class VALUE2>
    explicit TUintIntervalSet(const TIntervalSet<VALUE2>& iset) : super(iset, 0, UINT_MAX)
    {}
};
}  // namespace AFLib
