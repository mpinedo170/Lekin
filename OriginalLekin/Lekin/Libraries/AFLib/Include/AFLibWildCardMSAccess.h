#pragma once

#include "AFLibWildCard.h"

/////////////////////////////////////////////////////////////////////////////
// TWildCardMSAccess
// Implementation of the MS Access wildcards
// Supported wild-characters: '*', '?', '#'; sets: [a-z], '!' negation
// Examples:
//       '[a-g]l*i?n' matches 'florian'
//       '[!abc]*e' matches 'smile'
//       '[#]#' matches '#1'
// Also supports multiple masks, separated with semicolons
//   (put a semicolon in [] to make it a part of a pattern)

namespace AFLib {
class AFLIB TWildCardMSAccess : public TWildCard
{
private:
    CStringW m_wildcard;

    // helper for Init()
    void ReadBracket(LPCWSTR& s);

    // helper for both constructors
    void Init();

public:
    explicit TWildCardMSAccess(LPCSTR wildcard);
    explicit TWildCardMSAccess(LPCWSTR wildcard);
};
}  // namespace AFLib
