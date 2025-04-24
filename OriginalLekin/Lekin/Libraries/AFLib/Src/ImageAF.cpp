#include "StdAfx.h"

#include "AFLibImageAF.h"

#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibResource.h"
#include "Local.h"

using AFLibGui::CImageAF;

CImageAF::CImageAF()
{}

BOOL CImageAF::Create(int nWidth, int nHeight, int nBPP, DWORD dwFlags)
{
    return super::Create(nWidth, nHeight, nBPP, dwFlags);
}

BOOL CImageAF::Create(const CSize& sz, int nBPP, DWORD dwFlags)
{
    return super::Create(sz.cx, sz.cy, nBPP, dwFlags);
}

void CImageAF::CreateCompatible(int nWidth, int nHeight, CDC* pDC)
{
    HBITMAP hBmp = ::CreateCompatibleBitmap(*pDC, nWidth, nHeight);
    m_dc.Attach(::CreateCompatibleDC(pDC->m_hAttribDC));
    Attach(hBmp);
}

void CImageAF::CreateCompatible(const CSize& sz, CDC* pDC)
{
    CreateCompatible(sz.cx, sz.cy, pDC);
}

void CImageAF::CreateScreen(int nWidth, int nHeight)
{
    CDC* pDCScreen = CWnd::GetDesktopWindow()->GetDC();
    HBITMAP hBmp = ::CreateCompatibleBitmap(*pDCScreen, nWidth, nHeight);
    CWnd::GetDesktopWindow()->ReleaseDC(pDCScreen);
    Attach(hBmp);
}

void CImageAF::CreateScreen(const CSize& sz)
{
    CreateScreen(sz.cx, sz.cy);
}

void CImageAF::CreateMonochrome(int nWidth, int nHeight)
{
    HBITMAP hBmp = ::CreateBitmap(nWidth, nHeight, 1, 1, NULL);
    Attach(hBmp);
}

void CImageAF::CreateMonochrome(const CSize& sz)
{
    CreateMonochrome(sz.cx, sz.cy);
}

void CImageAF::CreateGlyph(const CImageAF& img, COLORREF colorBack)
{
    CSize sz = img.GetSize();
    CreateMonochrome(sz);

    CDC* pDCSource = img.GetCDC();
    CDC* pDCDest = GetCDC();
    pDCSource->SetBkColor(colorBack);
    pDCDest->BitBlt(0, 0, sz.cx, sz.cy, pDCSource, 0, 0, SRCCOPY);

    ReleaseCDC(pDCSource);
    ReleaseCDC(pDCDest);
}

void CImageAF::CreateHalftone(COLORREF colorL, COLORREF colorD)
{
    COLORMAP map[] = {{colorWhite, colorL == colorAuto ? colorLight() : colorL},
        {colorGrayL, colorD == colorAuto ? GetColorNormal2(bmcButton) : colorD}};

    HBITMAP hBmp = CreateMappedBitmap(AFLibPrivate::AFLibDLL.hModule, IDB_HALFTONE_BRUSH, 0, map, 2);
    Attach(hBmp);
}

void CImageAF::LoadBitmap(UINT nIDBitmap)
{
    LoadFromResource(AfxFindResourceHandle(MAKEINTRESOURCE(nIDBitmap), RT_BITMAP), nIDBitmap);
}

void CImageAF::LoadMappedBitmap(UINT nIDBitmap, const COLORMAP* lpColorMap, int nMapSize)
{
    HBITMAP hBmp = CreateMappedBitmap(AfxFindResourceHandle(MAKEINTRESOURCE(nIDBitmap), RT_BITMAP), nIDBitmap,
        0, const_cast<LPCOLORMAP>(lpColorMap), nMapSize);
    Attach(hBmp);
}

COLORREF CImageAF::GetColorNormal2(TBitmapClass bmc)
{
    switch (bmc)
    {
        case bmcWindow:
            return colorWindow();
        case bmcButton:
            return colorNormal();
    }
    return colorWhite;
}

void CImageAF::LoadNeatBitmap(UINT nIDBitmap, TBitmapClass bmc)
{
    int nMapSize = 2;
    COLORMAP map[3] = {{0}};

    switch (bmc)
    {
        case bmcWindow:
            map[0].from = colorBlack;
            map[0].to = colorWindowText();
            map[1].from = colorWhite;
            map[1].to = colorWindow();
            break;

        case bmcMenu:
            map[0].from = colorGrayD;
            map[0].to = colorDark();
            map[1].from = colorGrayL;
            map[1].to = colorWhite;
            break;

        case bmcButton:
            map[0].from = colorGrayD;
            map[0].to = colorDark();
            map[1].from = colorGrayL;
            map[1].to = colorNormal();
            map[2].from = colorWhite;
            map[2].to = colorLight();
            nMapSize = 3;
            break;
    }

    LoadMappedBitmap(nIDBitmap, map, nMapSize);
}

void CImageAF::LoadDisabledBitmap(UINT nIDBitmap, TBitmapClass bmc)
{  // original bitmap
    CImageAF imgOriginal;
    if (bmc == bmcMenu)
    {
        static const COLORMAP col = {colorWhite, colorGrayL};
        imgOriginal.LoadMappedBitmap(nIDBitmap, &col, 1);
    }
    else
        imgOriginal.LoadBitmap(nIDBitmap);

    // convert bitmap into a monocrome one: "dark" colors become black, "light" -- white
    CImageAF imgBin;
    imgBin.CreateGlyph(imgOriginal, colorGrayL);
    CSize sz = imgOriginal.GetSize();

    // create light-colored glyph
    CImageAF imgGlyph;
    imgGlyph.CreateScreen(sz);
    CDC* pDC = imgGlyph.GetCDC();
    pDC->SetBkColor(GetColorNormal2(bmc));
    pDC->SetTextColor(colorLight());
    imgBin.BitBlt(*pDC, 0, 0);
    ReleaseCDC(pDC);

    // create resulting bitmap and copy dark-colored glyph
    CreateScreen(sz);
    pDC = GetCDC();
    pDC->SetBkColor(GetColorNormal2(bmc));
    pDC->SetTextColor(colorDark());
    imgBin.BitBlt(*pDC, 0, 0);

    // combine light and dark glyphs
    imgGlyph.MaskBlt(*pDC, 1, 1, sz.cx - 1, sz.cy - 1, 0, 0, imgBin, 1, 1, 0xAACC0000);
    ReleaseCDC(pDC);
}

void CImageAF::MakeBrush(CBrush& brush)
{
    HBRUSH hBrushReg = CreatePatternBrush(*this);
    brush.Attach(hBrushReg);
}

void CImageAF::Fill(COLORREF color)
{
    CDC* pDC = GetCDC();
    pDC->FillSolidRect(0, 0, GetWidth(), GetHeight(), color);
    ReleaseCDC(pDC);
}

CDC* CImageAF::GetCDC() const
{
    CDCAF* pDC = new CDCAF;
    pDC->Attach(CreateCompatibleDC(m_dc.m_hDC));
    pDC->m_hBmpOld = reinterpret_cast<HBITMAP>(pDC->SelectObject(*this));
    return pDC;
}

void CImageAF::ReleaseCDC(CDC* pDC)
{
    CDCAF* pDC2 = dynamic_cast<CDCAF*>(pDC);
    if (pDC2 == NULL) return;

    pDC2->SelectObject(pDC2->m_hBmpOld);
    delete pDC2;
}
