#include "StdAfx.h"
#include "AFLibWordMask.h"
#include "AFLibStrings.h"

using AFLib::TWordMask;

TWordMask::TMask::TMask()
{ m_bNot = false; }

void TWordMask::TMask::SetNot(bool bNot)
{ m_bNot = bNot; }

void TWordMask::TMask::Add(TCHAR ch)
{ Add(ch, ch); }

void TWordMask::TMask::Add(TCHAR chFrom, TCHAR chTo)
{ int index = FindIndex(chFrom);

  if (index == -1)
  { m_data += chFrom;
    m_data += chTo;
  }
}
/*
bool TWordMask::TMask::Matches(TCHAR ch) const;
bool TWordMask::TMask::IsStar() const;
*/
