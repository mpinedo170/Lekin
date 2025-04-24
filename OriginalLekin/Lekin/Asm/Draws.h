#pragma once

// draw
void DrawUnder(CDC* pDC, const CRect& rect);
int DrawFancy(CDC* pDC, const CRect& rect, TObjective ob, bool bBold);
void DrawCheck(CDC* pDC, const CRect& rect, bool bCheck);
