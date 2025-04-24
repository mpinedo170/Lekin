#include "StdAfx.h"

#include "AFLibMenuBitmapper.h"

#include "AFLibGuiGlobal.h"
#include "AFLibResource.h"
#include "Local.h"

using AFLibGui::CImageAF;
using AFLibGui::TMenuBitmapper;

TMenuBitmapper::TMenuBitmapper() :
    m_szButton(GetSystemMetrics(SM_CXMENUCHECK), GetSystemMetrics(SM_CYMENUCHECK))
{
    InitIcon();
}

TMenuBitmapper::TMenuBitmapper(const CSize& szButton) : m_szButton(szButton)
{
    InitIcon();
}

TMenuBitmapper::~TMenuBitmapper()
{
    Clear();
    DestroyIcon(m_icoCheck);
}

void TMenuBitmapper::InitIcon()
{
    m_icoCheck = LoadIcon(AFLibPrivate::AFLibDLL.hModule, MAKEINTRESOURCE(IDI_CHECK));
    m_bStretchImg = false;
}

void TMenuBitmapper::MakeImage(CImageAF& imgDest, CImageList* pImgList, int iBitmap)
{
    IMAGEINFO infoImg;
    pImgList->GetImageInfo(iBitmap, &infoImg);
    CSize sz = CRect(infoImg.rcImage).Size();

    imgDest.CreateScreen(sz);
    CDC* pDC = imgDest.GetCDC();
    pImgList->DrawEx(pDC, iBitmap, CPoint(0, 0), sz, colorWhite, GetSysColor(COLOR_MENUTEXT), ILD_NORMAL);
    CImageAF::ReleaseCDC(pDC);
}

void TMenuBitmapper::CreateImage(
    CImageAF& imgDest, TImageType tit, const CImageAF& imgRegular, const CImageAF& imgDisabled)
{
    bool bDisabled = tit == titDisabled;
    if (bDisabled && imgDisabled.IsNull())
    {
        imgDest.Destroy();
        return;
    }

    if (imgDest.IsNull()) imgDest.CreateScreen(m_szButton);
    imgDest.Fill(colorWhite);

    const CImageAF& imgSource = bDisabled ? imgDisabled : imgRegular;
    CSize sz = imgSource.GetSize();
    CDC* pDC = imgDest.GetCDC();

    if (m_bStretchImg)
        imgSource.StretchBlt(*pDC, 0, 0, m_szButton.cx, m_szButton.cy, 0, 0, sz.cx, sz.cy);
    else
    {
        int dx = (m_szButton.cx - sz.cx) / 2;
        int dy = (m_szButton.cy - sz.cy) / 2;
        imgSource.BitBlt(*pDC, dx, dy, m_szButton.cx, m_szButton.cy, 0, 0);
    }

    if (tit == titChecked)
    {
        int dx = (m_szButton.cx - 8) / 2;
        int dy = (m_szButton.cy - 8) / 2;
        pDC->DrawIcon(dx, dy, m_icoCheck);
    }

    CImageAF::ReleaseCDC(pDC);
}

void TMenuBitmapper::GetBitmaps(
    HBITMAP& hBmpRegular, HBITMAP& hBmpChecked, const TButton* pBut, bool bDisabled)
{
    if (bDisabled)
    {
        hBmpRegular = hBmpChecked =
            pBut->m_imgDisabled.IsNull() ? HBITMAP(pBut->m_imgRegular) : HBITMAP(pBut->m_imgDisabled);
    }
    else
    {
        hBmpRegular = pBut->m_imgRegular;
        hBmpChecked = pBut->m_imgChecked;
    }
}

void TMenuBitmapper::Add(UINT idc, UINT nIDBitmap)
{
    CImageAF imgRegular;
    CImageAF imgDisabled;

    imgRegular.LoadNeatBitmap(nIDBitmap, CImageAF::bmcMenu);
    imgDisabled.LoadDisabledBitmap(nIDBitmap, CImageAF::bmcMenu);
    Add(idc, imgRegular, imgDisabled);
}

void TMenuBitmapper::Add(UINT idc, const CImageAF& img)
{
    CImageAF imgDummy;
    Add(idc, img, imgDummy);
}

void TMenuBitmapper::Add(UINT idc, const CImageAF& imgRegular, const CImageAF& imgDisabled)
{
    TButton* pBut = m_arrButton.FindKey(idc);
    if (pBut == NULL)
    {
        pBut = new TButton;
        pBut->m_idc = idc;
        m_arrButton.Insert(pBut);
    }

    CreateImage(pBut->m_imgRegular, titRegular, imgRegular, imgDisabled);
    CreateImage(pBut->m_imgChecked, titChecked, imgRegular, imgDisabled);
    CreateImage(pBut->m_imgDisabled, titDisabled, imgRegular, imgDisabled);
}

void TMenuBitmapper::Add(UINT idc, CToolBar& toolBar, int index, bool bByCommand)
{
    CToolBarCtrl& tbCtrl = toolBar.GetToolBarCtrl();

    CImageList* pImgListReg = tbCtrl.GetImageList();
    CImageList* pImgListDis = tbCtrl.GetDisabledImageList();

    if (bByCommand)
    {
        index = tbCtrl.CommandToIndex(index);
        if (index < 0) return;
    }

    TBBUTTON infoBut;
    tbCtrl.GetButton(index, &infoBut);

    if (infoBut.fsStyle == TBSTYLE_SEP) return;
    if (idc == 0) idc = infoBut.idCommand;

    int iBitmap = infoBut.iBitmap;
    if (iBitmap < 0) return;

    CImageAF imgRegular;
    MakeImage(imgRegular, pImgListReg, iBitmap);

    if (pImgListDis == NULL)
        Add(idc, imgRegular);
    else
    {
        CImageAF imgDisabled;
        MakeImage(imgDisabled, pImgListDis, iBitmap);
        Add(idc, imgRegular, imgDisabled);
    }
}

void TMenuBitmapper::Add(UINT idc, CToolBar& toolBar)
{
    Add(idc, toolBar, idc, true);
}

void TMenuBitmapper::Add(CToolBar& toolBar)
{
    for (int i = 0; i < toolBar.GetCount(); ++i) Add(0, toolBar, i, false);
}

const TMenuBitmapper::TButton* TMenuBitmapper::GetButton(int index, bool bByCommand) const
{
    if (bByCommand) index = m_arrButton.BSearch(index);

    return index < 0 || index >= GetButtonCount() ? NULL : m_arrButton[index];
}

void TMenuBitmapper::RemoveButton(int index, bool bByCommand)
{
    if (bByCommand) index = m_arrButton.BSearch(index);

    if (index >= 0 && index < GetButtonCount()) m_arrButton.DestroyAt(index);
}

void TMenuBitmapper::UpdateMenu(CMenu* pMenu)
{
    for (int i = 0; i < GetButtonCount(); ++i)
    {
        TButton* pBut = m_arrButton[i];
        int idc = pBut->m_idc;
        HBITMAP hBmpRegular = NULL;
        HBITMAP hBmpChecked = NULL;
        bool bDisabled = (pMenu->GetMenuState(idc, MF_BYCOMMAND) & (MF_DISABLED | MF_GRAYED)) != 0;

        GetBitmaps(hBmpRegular, hBmpChecked, pBut, bDisabled);
        SetMenuItemBitmaps(*pMenu, idc, MF_BYCOMMAND, hBmpRegular, hBmpChecked);
    }
}
