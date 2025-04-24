#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CImageAF image
// A bitmap image with a few extra functions

namespace AFLibGui {
class AFLIB CImageAF : public CImage
{
private:
    typedef CImage super;
    DEFINE_COPY_AND_ASSIGN(CImageAF);

    CDC m_dc;  // Device context (remembers device parameters)

    // device context with "old bitmap"
    struct CDCAF : public CDC
    {
        HBITMAP m_hBmpOld;
    };

public:
    // image class for LoadNeatBitmap() and LoadDisabledBitmap()
    enum TBitmapClass
    {
        bmcWindow,
        bmcMenu,
        bmcButton
    };

private:
    // white if bMenu, normal background color otherwise
    static COLORREF GetColorNormal2(TBitmapClass bmc);

public:
    CImageAF();

    // same as CImage::Create()
    BOOL Create(int nWidth, int nHeight, int nBPP, DWORD dwFlags = 0);
    BOOL Create(const CSize& sz, int nBPP, DWORD dwFlags = 0);

    // create image, compatible with given device context
    void CreateCompatible(int nWidth, int nHeight, CDC* pDC);
    void CreateCompatible(const CSize& sz, CDC* pDC);

    // create image, compatible with the screen
    void CreateScreen(int nWidth, int nHeight);
    void CreateScreen(const CSize& sz);

    // create a monochrome image
    void CreateMonochrome(int nWidth, int nHeight);
    void CreateMonochrome(const CSize& sz);

    // create a monochrome image representing a "glyph" of existing image
    void CreateGlyph(const CImageAF& img, COLORREF colorBack);

    // create bitmap for a halftone brush
    void CreateHalftone(COLORREF colorL, COLORREF colorD);

    // load bitmap from resource
    void LoadBitmap(UINT nIDBitmap);

    // looad bitmap, converting some colors
    void LoadMappedBitmap(UINT nIDBitmap, const COLORMAP* lpColorMap, int nMapSize);

    // load bitmap, converting colors to fit into current screen scheme
    void LoadNeatBitmap(UINT nIDBitmap, TBitmapClass bmc);

    // creates a "disabled" image of the bitmap
    void LoadDisabledBitmap(UINT nIDBitmap, TBitmapClass bmc);

    // create a brush from the image
    void MakeBrush(CBrush& brush);

    // fill the image with color
    void Fill(COLORREF color);

    // get the device context
    CDC* GetCDC() const;

    // release the device context
    static void ReleaseCDC(CDC* pDC);

    // get image size
    CSize GetSize() const
    {
        return CSize(GetWidth(), GetHeight());
    }
};
}  // namespace AFLibGui
