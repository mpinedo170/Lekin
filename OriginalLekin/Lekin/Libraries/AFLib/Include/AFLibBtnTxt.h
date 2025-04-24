#pragma once

#include "AFLibBtnOwner.h"

/////////////////////////////////////////////////////////////////////////////
// CBtnOwner
// Owner-draw button with Unicode text

namespace AFLibGui {
class AFLIB CBtnTxt : public CBtnOwner
{
private:
    typedef CBtnOwner super;
    DEFINE_COPY_AND_ASSIGN(CBtnTxt);

protected:
    CStringW m_title;  // title
    COLORREF m_color;  // foreground color
    CFont m_font;      // "explicit" font for arrow buttons

public:
    CBtnTxt();

    // set button title
    void SetTitle(LPCSTR title);
    void SetTitle(LPCWSTR title);

    // for "arrow" buttons
    enum TArrow
    {
        arUp,
        arDown,
        arLeft,
        arRight
    };

    // create an arrow button
    void SetArrowTitle(TArrow ar);

    // set button foreground color
    void SetColor(COLORREF color, bool bRedraw = true);

    // get button title
    const CStringW& GetTitle()
    {
        return m_title;
    }

    // get button foreground color
    COLORREF GetColor()
    {
        return m_color;
    }

    virtual void EnableButtonToolTip(bool bEnable);

protected:
    virtual void Draw(CDC* pDC);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CBtnTxt)
};
}  // namespace AFLibGui
