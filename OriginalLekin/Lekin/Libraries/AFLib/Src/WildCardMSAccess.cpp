#include "StdAfx.h"

#include "AFLibWildCardMSAccess.h"

#include "AFLibStrings.h"

using AFLib::TWildCardMSAccess;
using namespace AFLibIO;

TWildCardMSAccess::TWildCardMSAccess(LPCSTR wildcard) : m_wildcard(wildcard)
{
    Init();
}

TWildCardMSAccess::TWildCardMSAccess(LPCWSTR wildcard) : m_wildcard(wildcard)
{
    Init();
}

void TWildCardMSAccess::Init()
{
    LPCWSTR s = m_wildcard;

    while (true)
    {
        WCHAR ch = *s;
        if (ch == 0) break;
        ++s;

        switch (ch)
        {
            case chrStarW:
            {
                TPattern* pPat = new TPattern;
                pPat->m_iset.Invert();
                pPat->m_nMinMatch = 0;
                pPat->m_nMaxMatch = MXM;
                AddPattern(pPat);
                break;
            }

            case chrQuestionW:
            {
                TPattern* pPat = new TPattern;
                pPat->m_iset.Invert();
                AddPattern(pPat);
                break;
            }

            case chrPoundW:
            {
                TPattern* pPat = new TPattern;
                pPat->m_iset.Add(_T('0'), _T('9'));
                AddPattern(pPat);
                break;
            }

            case L'[':
                ReadBracket(s);
                break;

            case chrSemicolonW:
                NewPatList();
                break;

            default:
            {
                TPattern* pPat = new TPattern;
                pPat->m_iset.Add(ch);
                AddPattern(pPat);
                break;
            }
        }
    }
}

void TWildCardMSAccess::ReadBracket(LPCWSTR& s)
{
    TPattern* pPat = new TPattern;
    bool bInvert = false;

    if (*s == chrExclamW)
    {
        ++s;
        bInvert = true;
    }

    while (*s != 0)
    {
        if (*s == L']')
        {
            ++s;
            break;
        }

        WCHAR chFirst = pPat->m_iset.m_min;

        if (*s != chrDashW)
        {
            chFirst = *s;
            ++s;
        }

        if (*s != chrDashW)
        {
            pPat->m_iset.Add(chFirst);
            continue;
        }

        ++s;

        WCHAR chLast = pPat->m_iset.m_max;

        if (*s != L']')
        {
            chLast = *s;
            ++s;
        }

        pPat->m_iset.Add(chFirst, chLast);
    }

    if (bInvert) pPat->m_iset.Invert();
    AddPattern(pPat);
}
