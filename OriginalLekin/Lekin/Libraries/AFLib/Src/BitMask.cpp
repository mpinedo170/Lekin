#include "StdAfx.h"

#include "AFLibBitMask.h"

#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"

using namespace AFLib;
using namespace AFLibIO;

namespace {
const UINT ONES = UINT(-1);
const int NBITS = sizeof(int) * 8;
const CString strL = _T("L");
const CString strH = _T("H");

UINT CreateMask(int nBits)
{
    return nBits >= NBITS ? ONES : (1U << nBits) - 1;
}
}  // namespace

TBitMask::TBitMask(int xFirst, int xLast)
{
    m_xFirst = xFirst;
    m_xLast = xLast;
}

TBitMask::TBitMask(const TBitMask& bm)
{
    *this = bm;
}

TBitMask& TBitMask::operator=(const TBitMask& bm)
{
    if (this == &bm) return *this;

    m_xFirst = bm.m_xFirst;
    m_xLast = bm.m_xLast;
    m_arrSel.Copy(bm.m_arrSel);
    return *this;
}

bool TBitMask::operator==(const TBitMask& bm) const
{
    ASSERT(m_xFirst == bm.m_xFirst);
    ASSERT(m_xLast == bm.m_xLast);

    if (this == &bm) return true;
    if (m_arrSel.GetSize() != bm.m_arrSel.GetSize()) return false;

    for (int i = 0; i < m_arrSel.GetSize(); ++i)
        if (m_arrSel[i] != bm.m_arrSel[i]) return false;

    return true;
}

int TBitMask::GetHashKey() const
{
    UINT key = 0;
    for (int i = 0; i < m_arrSel.GetSize(); ++i) key ^= m_arrSel[i];
    return int(key);
}

void TBitMask::Clear()
{
    m_arrSel.RemoveAll();
}

void TBitMask::ClearZeros()
{
    int i = m_arrSel.GetSize();
    while (--i >= 0)
        if (m_arrSel[i] != 0) break;
    m_arrSel.SetSize(i + 1);
}

int TBitMask::GetOneCount() const
{
    int s = 0;
    static const UINT mask = 1U << (NBITS - 1);

    for (int i = 0; i < m_arrSel.GetSize(); ++i)
    {
        UINT v = m_arrSel[i];
        if (v == ONES)
        {
            s += NBITS;
            continue;
        }

        for (; v != 0; v <<= 1)
            if (v & mask) ++s;
    }
    return s;
}

void TBitMask::Save(TStringBuffer& sb, bool bSaveLower, bool bSaveHigher) const
{
    sb += _T('(');

    if (bSaveLower)
    {
        sb += strL;
        sb.AppendInt(m_xFirst);
        sb += chrSpace;
    }

    if (bSaveHigher)
    {
        sb += strH;
        sb.AppendInt(m_xLast);
        sb += chrSpace;
    }

    for (int i = 0; i < m_arrSel.GetSize(); ++i)
    {
        if (i > 0) sb += chrSpace;
        sb.AppendHex(m_arrSel[i], 8);
    }

    sb += _T(')');
}

void TBitMask::Load(TStringSpacer& sp, bool bKeepBounds, int xFirstDefault)
{
    int xFirst = 0;
    int xLast = -1;

    m_arrSel.RemoveAll();
    sp.TestSymbolHard(_T("("));

    if (bKeepBounds)
    {
        xFirst = m_xFirst;
        xLast = m_xLast;
    }

    if (sp.TestSymbol(strL))
        m_xFirst = sp.ReadInt();
    else if (xFirstDefault < INT_MAX && xFirstDefault <= m_xLast)
        m_xFirst = xFirstDefault;

    if (sp.TestSymbol(strH)) m_xLast = max(sp.ReadInt(), m_xFirst);

    while (!sp.TestSymbol(_T(")"))) m_arrSel.Add(sp.ReadHex());

    if (xFirst <= xLast) SetBounds(xFirst, xLast);

    CutXLast();
}

void TBitMask::CutXLast()
{
    int xLength = GetLength();
    int n = (xLength - 1) / NBITS + 1;

    if (m_arrSel.GetSize() >= n)
    {
        m_arrSel.SetSize(n);
        m_arrSel[n - 1] &= CreateMask(xLength - (n - 1) * NBITS);
    }

    ClearZeros();
}

void TBitMask::SetBounds(int xFirst, int xLast)
{
    int diff = xFirst - m_xFirst;

    if (diff > 0)
        DeleteAt(m_xFirst, diff);
    else
        InsertAt(m_xFirst, false, -diff);

    m_xFirst = xFirst;
    m_xLast = max(xFirst, xLast);
    CutXLast();
}

void TBitMask::GetOneMask(int x, bool bRight, int& n, UINT& mask) const
{
    x -= m_xFirst;
    n = x / NBITS;
    mask = bRight ? CreateMask(x % NBITS + 1) : ~CreateMask(x % NBITS);
}

bool TBitMask::GetMasks1(int x1, int x2, int& n1, int& n2, UINT& mask1, UINT& mask2) const
{
    x1 = max(m_xFirst, x1);
    x2 = min(x2, m_xLast);
    if (x1 > x2) return false;

    GetOneMask(x1, false, n1, mask1);
    GetOneMask(x2, true, n2, mask2);
    return true;
}

void TBitMask::GetMasks2(int& n1, int& n2, UINT& mask1, UINT& mask2) const
{
    if (n1 == n2)
    {
        mask1 &= mask2;
        mask2 = 0;
    }
}

bool TBitMask::GetMasks(int x1, int x2, int& n1, int& n2, UINT& mask1, UINT& mask2) const
{
    if (!GetMasks1(x1, x2, n1, n2, mask1, mask2)) return false;

    if (n2 >= m_arrSel.GetSize())
    {
        n2 = m_arrSel.GetSize() - 1;
        if (n2 < n1) return false;
        mask2 = ONES;
    }

    GetMasks2(n1, n2, mask1, mask2);
    return true;
}

bool TBitMask::GetMasksExtend(int x1, int x2, int& n1, int& n2, UINT& mask1, UINT& mask2)
{
    if (!GetMasks1(x1, x2, n1, n2, mask1, mask2)) return false;

    if (n2 >= m_arrSel.GetSize()) m_arrSel.SetSize(n2 + 1);

    GetMasks2(n1, n2, mask1, mask2);
    return true;
}

void TBitMask::ShiftRight(int x, int dx)
{
    if (dx <= 0) return;

    int n = 0;
    UINT mask = 0;
    GetOneMask(x, false, n, mask);

    // if this is a tail with zeros only, nothing to shift...
    int nLast = m_arrSel.GetSize() - 1;
    if (n > nLast) return;

    // first, do the front word
    UINT shifted = m_arrSel[n] >> dx;

    if (n < nLast) shifted |= m_arrSel[n + 1] << (NBITS - dx);

    m_arrSel[n] = (m_arrSel[n] & ~mask) | (shifted & mask);
    if (n >= nLast) return;

    // shift all words
    while (++n < nLast)
    {
        shifted = m_arrSel[n + 1] << (NBITS - dx);
        m_arrSel[n] = (m_arrSel[n] >> dx) | shifted;
    }

    // shift last word
    m_arrSel[n] = m_arrSel[n] >> dx;
}

void TBitMask::InsertAt(int x, bool bSelected, int count)
{
    ASSERT(x >= m_xFirst && x - 1 <= m_xLast);
    if (count <= 0) return;

    // figure out the starting word and bit
    int n = 0;
    UINT mask = 0;
    GetOneMask(x, false, n, mask);

    if (n >= m_arrSel.GetSize()) m_arrSel.SetSize(n + 1);

    // remember the left side of the starting word
    UINT remainder = m_arrSel[n] & mask;

    // clear the left side of the starting word
    if (bSelected)
        m_arrSel[n] |= mask;
    else
        m_arrSel[n] &= ~mask;

    // shift left by whole words
    int dn = (count - 1) / NBITS + 1;
    UINT val = bSelected ? ONES : 0;
    m_arrSel.InsertAt(n + 1, val, dn);

    // fill in the last inserted word with bits from first word
    m_arrSel[n + dn] &= ~mask;
    m_arrSel[n + dn] |= remainder;

    // shift right by partial word
    ShiftRight(x + count, dn * NBITS - count);

    if (m_xLast < INT_MAX - count)
        m_xLast += count;
    else
        m_xLast = INT_MAX;

    ClearZeros();
}

void TBitMask::DeleteAt(int x, int count)
{
    ASSERT(x >= m_xFirst && x - 1 <= m_xLast);

    count = min(count, m_xLast - x + 1);
    if (count <= 0) return;

    // figure out the starting word and bit
    int n = 0;
    UINT mask = 0;
    GetOneMask(x, false, n, mask);

    int dn = min(count / NBITS, m_arrSel.GetSize() - n);

    // remove whole words
    if (dn > 0) m_arrSel.RemoveAt(n + 1, dn);

    // remove remaining bits
    ShiftRight(x, count % NBITS);
    m_xLast = max(m_xLast - count, x);
    ClearZeros();
}

bool TBitMask::TestInterval(int x1, int x2, bool bSelected) const
{
    int n1, n2;
    UINT mask1, mask2;

    if (bSelected && (x1 < m_xFirst || x2 > m_xLast || x2 - m_xFirst >= m_arrSel.GetSize() * NBITS))
        return false;
    if (!GetMasks(x1, x2, n1, n2, mask1, mask2)) return !bSelected;

    if ((m_arrSel[n1] & mask1) != (bSelected ? mask1 : 0)) return false;
    if ((m_arrSel[n2] & mask2) != (bSelected ? mask2 : 0)) return false;

    UINT v = bSelected ? ONES : 0;
    for (int i = n1 + 1; i < n2; ++i)
        if (m_arrSel[i] != v) return false;
    return true;
}

bool TBitMask::IsSelected(int x) const
{
    if (x < m_xFirst || x > m_xLast) return false;
    x -= m_xFirst;

    int n = x / NBITS;
    if (n >= m_arrSel.GetSize()) return false;

    int bit = x % NBITS;
    return (m_arrSel[n] & (1U << bit)) != 0;
}

bool TBitMask::FindInterval(int& x1, int& x2) const
{
    int i, n;
    x1 = x2 = m_xFirst - 1;

    for (n = 0;; ++n)
    {
        if (n >= m_arrSel.GetSize()) return false;
        if (m_arrSel[n] != 0) break;
    }

    UINT v = m_arrSel[n];
    for (i = 0; i < NBITS; ++i)
        if ((v & (1U << i)) != 0) break;

    x1 = n * NBITS + i + m_xFirst;
    for (n = m_arrSel.GetSize(); --n >= 0;)
        if (m_arrSel[n] != 0) break;

    v = m_arrSel[n];
    for (i = NBITS; --i >= 0;)
        if ((v & (1U << i)) != 0) break;

    x2 = n * NBITS + i + m_xFirst;
    return true;
}

void TBitMask::Operation(int x1, int x2, TLogicalFunction lfn)
{
    if (lfn == lfnAnd)
    {
        ClearInterval(m_xFirst, x1 - 1);
        ClearInterval(x2 + 1, m_xLast);
        return;
    }

    int n1, n2;
    UINT mask1, mask2;
    if (!GetMasksExtend(x1, x2, n1, n2, mask1, mask2)) return;
    UINT v = ONES;

    switch (lfn)
    {
        case lfnAndN:
            m_arrSel[n1] &= ~mask1;
            m_arrSel[n2] &= ~mask2;
            v = 0;
            break;

        case lfnOr:
            m_arrSel[n1] |= mask1;
            m_arrSel[n2] |= mask2;
            break;

        case lfnXor:
            m_arrSel[n1] ^= mask1;
            m_arrSel[n2] ^= mask2;
            for (int i = n1 + 1; i < n2; ++i) m_arrSel[i] ^= v;
            ClearZeros();
            return;
    }

    for (int i = n1 + 1; i < n2; ++i) m_arrSel[i] = v;
    ClearZeros();
}

void TBitMask::Operation(const TBitMask& bm, TLogicalFunction lfn)
{
    if (&bm == this)
    {
        if (lfn == lfnAndN || lfn == lfnXor) Clear();
        return;
    }

    ASSERT(m_xFirst == bm.m_xFirst);
    ASSERT(m_xLast == bm.m_xLast);

    int lenMin = min(m_arrSel.GetSize(), bm.m_arrSel.GetSize());
    int lenOper = 0;

    switch (lfn)
    {
        case lfnAnd:
            m_arrSel.SetSize(lenMin);

        case lfnAndN:
            lenOper = lenMin;
            break;

        default:
            m_arrSel.SetSize(max(m_arrSel.GetSize(), bm.m_arrSel.GetSize()));
            lenOper = bm.m_arrSel.GetSize();
            break;
    }

    for (int i = 0; i < lenOper; ++i) switch (lfn)
        {
            case lfnAnd:
                m_arrSel[i] &= bm.m_arrSel[i];
                break;

            case lfnAndN:
                m_arrSel[i] &= ~bm.m_arrSel[i];
                break;

            case lfnOr:
                m_arrSel[i] |= bm.m_arrSel[i];
                break;

            case lfnXor:
                m_arrSel[i] ^= bm.m_arrSel[i];
                break;
        }

    ClearZeros();
}
