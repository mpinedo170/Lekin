#pragma once

#include "AFLibCardIntervalSet.h"

/////////////////////////////////////////////////////////////////////////////
// TWildCard
// Generic wildcards/regular expressions.
// Actual wildcards must be implemented in derived classes

namespace AFLib {
class AFLIB TWildCard
{
protected:
    // single characher match
    struct TPattern
    {
    public:
        TWCharIntervalSet m_iset;  // set of satisfying chars
        int m_nMinMatch;           // min number of matches
        int m_nMaxMatch;           // max number of matches

        TPattern()
        {
            m_nMinMatch = m_nMaxMatch = 1;
        }
    };

    // full pattern
    class TPatList
    {
    private:
        AFLib::CSmartArray<TPattern> m_arrPattern;

    public:
        TPatList();
        TPatList(const TPatList& patList);
        ~TPatList();

        // add pattern; may modify previous
        bool AddPattern(TPattern* pPat);

        // recursive helper for TWildCard::Matches()
        bool Matches(LPCVOID s, int charLength, int index) const;
    };

    // array of patterns
    AFLib::CSmartArray<TPatList> m_arrPatList;

    // create new pattern list
    void NewPatList();

    // add pattern; may modify previous
    void AddPattern(TPattern* pPat);

    // protected constructor
    TWildCard();

    // max value of m_nMaxMatch
    static const int MXM;

public:
    TWildCard(const TWildCard& wc);
    ~TWildCard();

    TWildCard& operator=(const TWildCard& wc);

    // This function implements the wildcards
    bool Matches(LPCSTR s) const;
    bool Matches(LPCWSTR s) const;
};
}  // namespace AFLib
