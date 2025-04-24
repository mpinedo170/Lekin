#pragma once

#include "AFLibArrays.h"
#include "AFLibImageAF.h"

/////////////////////////////////////////////////////////////////////////////
// TMenuBitmapper
// Class for transferring images from toolbars to menus

namespace AFLibGui {
class AFLIB TMenuBitmapper
{
private:
    DEFINE_COPY_AND_ASSIGN(TMenuBitmapper);

    // init the "check" icon
    void InitIcon();

    // extract image from list
    static void MakeImage(CImageAF& imgDest, CImageList* pImgList, int iBitmap);

public:
    // represents one button
    struct TButton
    {
        UINT m_idc;
        CImageAF m_imgRegular;   // regular image
        CImageAF m_imgChecked;   // image with a check on top
        CImageAF m_imgDisabled;  // disabled image

        // key for sorting
        int GetGLKey() const
        {
            return m_idc;
        }
    };

protected:
    HICON m_icoCheck;                             // icon with a check
    CSize m_szButton;                             // size of a menu check
    AFLib::CSortedIntArray<TButton> m_arrButton;  // array of menu bitmaps

    // overridables
    enum TImageType
    {
        titRegular,
        titChecked,
        titDisabled
    };

    // creates menu image from "regular" and "disabled"
    virtual void CreateImage(
        CImageAF& imgDest, TImageType tit, const CImageAF& imgRegular, const CImageAF& imgDisabled);

    // extracts two menu images from the button
    virtual void GetBitmaps(HBITMAP& hBmpRegular, HBITMAP& hBmpChecked, const TButton* pBut, bool bDisabled);

public:
    // stretch images to fit into "menu checkmark" space
    bool m_bStretchImg;

    TMenuBitmapper();
    explicit TMenuBitmapper(const CSize& szButton);
    virtual ~TMenuBitmapper();

    // add button from a resource
    void Add(UINT idc, UINT nIDBitmap);

    // add button from an image
    void Add(UINT idc, const CImageAF& img);

    // add button from two images
    void Add(UINT idc, const CImageAF& imgRegular, const CImageAF& imgDisabled);

    // add one button from a tool bar
    void Add(UINT idc, CToolBar& toolBar);

    // add one button, changing command ID
    void Add(UINT idc, CToolBar& toolBar, int index, bool bByCommand);

    // add all from a tool bar
    void Add(CToolBar& toolBar);

    // get button size
    const CSize& GetButtonSize() const
    {
        return m_szButton;
    }

    // get number of buttons
    int GetButtonCount() const
    {
        return m_arrButton.GetSize();
    }

    // remove all buttons
    void Clear()
    {
        m_arrButton.DestroyAll();
    }

    // get button information
    const TButton* GetButton(int index, bool bByCommand) const;

    // remove a button
    void RemoveButton(int index, bool bByCommand);

    // do the work -- set images in a menu
    void UpdateMenu(CMenu* pMenu);
};
}  // namespace AFLibGui
