#include "StdAfx.h"
#include "Colors.h"
#include "DCMem.h"

LPCTSTR FontSystem = _T("Tahoma");
LPCTSTR FontRoman = _T("Times New Roman");
const LPCTSTR FontSymbol = _T("Symbol");

/////////////////////////////////////////////////
// color manipulation

bool IsMono(CDC* pDC)
{ return pDC!=NULL && pDC->GetDeviceCaps(NUMCOLORS) == 2; }

bool ColorsOk(CWnd* pWnd)
{ CDC* pDC = pWnd->GetDC();
  int colors = pDC->GetDeviceCaps(NUMCOLORS);
  pWnd->ReleaseDC(pDC);
  return colors<0 || colors>256;
}

COLORREF GetColorLighter(COLORREF color, int index)
{ if (index == 0) return color;

  COLORREF color2 = 0;
  int i, kMax = 0;

  for (i=0; i<24; i+=8)
    kMax = max(kMax, int((color >> i) & 0xFF));
  kMax = kMax==0 ? 96 : kMax * 2 / 3;
  if (index == 1) kMax = kMax * 2 / 3;

  for (i=0; i<24; i+=8)
  { int k = (color >> i) & 0xFF;
    if (k == 0) k = kMax;
    color2 |= k << i;
  }
  return color2;
}

COLORREF GetColorMean(COLORREF c1, COLORREF c2, double coefficient)
{ COLORREF c = 0;
  for (int i=0; i<3; i++)
  { int mask = 0xFF << i*8;
    c += int(0.5 + (c1 & mask) * (1-coefficient) + (c2 & mask) * coefficient) & mask;
  }
  return c;
}

/////////////////////////////////////////////////
// bitmap

CSize GetBitmapSize(const CBitmap& bmp)
{ BITMAP bmInfo;
  if (bmp.GetObject(sizeof(bmInfo), &bmInfo) != sizeof(bmInfo))
    return CSize(0, 0);
  return CSize(bmInfo.bmWidth, bmInfo.bmHeight);
}

void CreateScreenBitmap(CBitmap& bmp, int cx, int cy)
{ CDC* pDC = AfxGetMainWnd()->GetDC();
  bmp.CreateCompatibleBitmap(pDC, cx, cy);
  AfxGetMainWnd()->ReleaseDC(pDC);
}

void DrawBitmap(CDC* pDC, const CBitmap& bmp, int x, int y)
{ CDCMem dcMem((CBitmap*)&bmp);
  dcMem.SetBkColor(0xC0C0C0);
  dcMem.Paste(pDC, x, y);
}

inline COLORREF normal2(bool bMenu)
{ return bMenu ? white : normal(); }

void LoadNeatBitmap(CBitmap& bmp, int id, bool bMenu)
{ COLORMAP map[] = { { grayD, dark() }, { grayL, normal2(bMenu) }, { white, light() } };
  bmp.LoadMappedBitmap(id, 0, map, bMenu ? 2 : 3);
}

void LoadDisabledBitmap(CBitmap& bmp, int id, bool bMenu)
{ // original bitmap
  CBitmap orgBmp;
  if (bMenu)
  { COLORMAP col = { white, grayL };
    orgBmp.LoadMappedBitmap(id, 0, &col, 1);
  }
  else orgBmp.LoadBitmap(id);
  CSize sz = GetBitmapSize(orgBmp);

  // convert bitmap into a monocrome one: "dark" colors become black, "light" -- white
  CDCMem dcBin(sz.cx, sz.cy, true);
  DrawBitmap(&dcBin, orgBmp, 0, 0);

  // create light-colored glyph
  CDCMem dcTmp(sz.cx, sz.cy, false);
  dcTmp.SetBkColor(normal2(bMenu));
  dcTmp.SetTextColor(light());
  dcBin.Paste(&dcTmp, 0, 0);

  // create resulting bitmap
  CreateScreenBitmap(bmp, sz.cx, sz.cy);
  CDCMem dcRes(&bmp);

  // copy dark-colored glyph
  dcRes.SetBkColor(normal2(bMenu));
  dcRes.SetTextColor(dark());
  dcBin.Paste(&dcRes, 0, 0);

  // combine light and dark glyphs
  dcRes.MaskBlt(1, 1, sz.cx-1, sz.cy-1, &dcTmp, 0, 0, *dcBin.m_pBmp,
    1, 1, 0xAACC0000);
}

/////////////////////////////////////////////////
// font

void GetFont(CDC* pDC, CFont& font, LPCTSTR face, int size, bool bAbs,
  bool bBold, bool bItalic)
{ LOGFONT logFont;
  memset(&logFont, 0, sizeof(logFont));
  logFont.lfWeight = bBold ? 700 : 400;
  logFont.lfItalic = bItalic;
  _tcscpy(logFont.lfFaceName, face);

  logFont.lfCharSet = ANSI_CHARSET;
  logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  logFont.lfQuality = PROOF_QUALITY;
  logFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

  if (!bAbs)
  { CWnd* pWnd = NULL;
    if (pDC == NULL)
    { pWnd = CWnd::GetDesktopWindow();
      pDC = pWnd->GetDC();
    }

    size = -MulDiv(size, pDC->GetDeviceCaps(LOGPIXELSY), 720);
    if (pWnd) pWnd->ReleaseDC(pDC);
  }

  logFont.lfHeight = size;
  font.CreateFontIndirect(&logFont);
}

void GetCourierFont(CDC* pDC, CFont& font)
{ GetFont(pDC, font, _T("Courier New"), 80, false, false, false); }

CSize GetFontSize(CDC* pDC, CFont* pFont, LPCTSTR str)
{ HWND hWnd = NULL;
  HDC hDC = NULL;

  if (pDC == NULL)
  { hWnd = CWnd::GetDesktopWindow()->m_hWnd;
    hDC = ::GetDC(hWnd);
  }
  else hDC = pDC->m_hAttribDC;

  ASSERT(hDC != NULL);
  if (str == NULL) str = _T("Wg");

  HGDIOBJ hFontOld = NULL;
  if (pFont != NULL)
    hFontOld = ::SelectObject(hDC, pFont->m_hObject);

  CSize sz;
  ::GetTextExtentPoint32(hDC, str, _tcslen(str), &sz);

  if (hFontOld != NULL)
    ::SelectObject(hDC, hFontOld);

  if (hWnd != NULL)
    ::ReleaseDC(hWnd, hDC);

  return sz;
}

/////////////////////////////////////////////////
// lines

CSize GetDP(CPoint p1, CPoint p2, int sz)
{ CSize dp0 = p2 - p1;
  double temp = sqrt(double(dp0.cx*dp0.cx + dp0.cy*dp0.cy)) / sz;
  static CSize dp;
  dp.cx = int(dp0.cy / temp);
  dp.cy = int(-dp0.cx / temp);
  return dp;
}

void AndrewLineTo(CDC* pDC, int x, int y)
{ AndrewLineTo(pDC, CPoint(x, y)); }

void AndrewLineTo(CDC* pDC, CPoint pt)
{ CPen pen; pen.CreateStockObject(NULL_PEN);
  CPen* pPenOld = pDC->SelectObject(&pen);
  LOGPEN logPen; pPenOld->GetLogPen(&logPen);

  CPoint pt2 = pDC->GetCurrentPosition();
  CSize dp = GetDP(pt, pt2, logPen.lopnWidth.x/2+1);
  CSize dp2(dp); pDC->LPtoDP(&dp2);

  if (abs(dp2.cx) + abs(dp2.cy) <= 1)
  { pDC->SelectObject(pPenOld);
    pDC->LineTo(pt); return;
  }

  CBrush brush(logPen.lopnColor);
  CBrush* pBrushOld = pDC->SelectObject(&brush);

  CPoint pp[4] = { pt+dp, pt-dp, pt2-dp, pt2+dp };

  pDC->Polygon(pp, 4);
  pDC->SelectObject(pBrushOld);
  pDC->SelectObject(pPenOld);
  pDC->MoveTo(pt);
}
