#include "StdAfx.h"

#include "AFLibWildCard.h"

#include "AFLibStringParser.h"
#include "AFLibStrings.h"

using AFLib::TWildCard;

const int TWildCard::MXM = INT_MAX - 1;

/////////////////////////////////////////////////////////////////////////////
// TWildCard::TPatList

TWildCard::TPatList::TPatList()
{
    m_arrPattern.SetSize(0, 32);
}

TWildCard::TPatList::TPatList(const TPatList& patList)
{
    int count = patList.m_arrPattern.GetSize();
    m_arrPattern.SetSize(count);

    for (int i = 0; i < count; ++i) m_arrPattern[i] = new TPattern(*patList.m_arrPattern[i]);
}

TWildCard::TPatList::~TPatList()
{
    m_arrPattern.DestroyAll();
}

bool TWildCard::TPatList::AddPattern(TPattern* pPat)
{
    int n = m_arrPattern.GetSize() - 1;

    if (n < 0 || m_arrPattern[n]->m_iset != pPat->m_iset)
    {
        m_arrPattern.Add(pPat);
        return true;
    }

    TPattern* pPat2 = m_arrPattern[n];
    pPat->m_nMinMatch += pPat2->m_nMinMatch;

    if (pPat2->m_nMaxMatch >= MXM - pPat->m_nMaxMatch)
        pPat->m_nMaxMatch = MXM;
    else
        pPat->m_nMaxMatch += pPat2->m_nMaxMatch;

    delete pPat2;
    m_arrPattern[n] = pPat;
    return false;
}

namespace {
inline WCHAR GetCh(LPCVOID s, int charLength)
{
    return charLength == 2 ? *reinterpret_cast<LPCWSTR>(s) : *reinterpret_cast<LPCSTR>(s);
}
}  // namespace

bool TWildCard::TPatList::Matches(LPCVOID s, int charLength, int index) const
{
    WCHAR ch = GetCh(s, charLength);

    if (index >= m_arrPattern.GetSize())
    {  // end of pattern, check if no extra characters in string
        return ch == 0;
    }

    const TPattern* pPat = m_arrPattern[index];

    for (int nMatch = 0; nMatch <= pPat->m_nMaxMatch; ++nMatch)
    {  // check if matched current pattern min number of times; if so, try to match the tail
        if (nMatch >= pPat->m_nMinMatch && Matches(s, charLength, index + 1)) return true;

        // match current pattern once again
        if (ch == 0 || !pPat->m_iset.Contains(ch)) return false;

        s = reinterpret_cast<LPCBYTE>(s) + charLength;
        ch = GetCh(s, charLength);
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// TWildCard

TWildCard::TWildCard()
{
    NewPatList();
}

TWildCard::TWildCard(const TWildCard& wc)
{
    *this = wc;
}

TWildCard::~TWildCard()
{
    m_arrPatList.DestroyAll();
}

TWildCard& TWildCard::operator=(const TWildCard& wc)
{
    m_arrPatList.DestroyAll();
    m_arrPatList.SetSize(wc.m_arrPatList.GetSize());

    for (int i = 0; i < wc.m_arrPatList.GetSize(); ++i) m_arrPatList[i] = new TPatList(*wc.m_arrPatList[i]);

    return *this;
}

void TWildCard::NewPatList()
{
    m_arrPatList.Add(new TPatList);
}

void TWildCard::AddPattern(TPattern* pPat)
{
    m_arrPatList[m_arrPatList.GetSize() - 1]->AddPattern(pPat);
}

bool TWildCard::Matches(LPCSTR s) const
{
    for (int i = 0; i < m_arrPatList.GetSize(); ++i)
        if (m_arrPatList[i]->Matches(s, 1, 0)) return true;
    return false;
}

bool TWildCard::Matches(LPCWSTR s) const
{
    for (int i = 0; i < m_arrPatList.GetSize(); ++i)
        if (m_arrPatList[i]->Matches(s, 2, 0)) return true;
    return false;
}
