#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TLCBDraw structure (List-Combo-Button)
// Holds draw and measure data for owner-draw list boxes, comboboxes, and buttons
//   (also possible to use for menus)
// Performs common operations in owner-drawing

namespace AFLibGui {
class AFLIB TLCBDraw
{
private:
    DEFINE_COPY_AND_ASSIGN(TLCBDraw);

    HWND m_hWnd;        // HWND of the list box or combobox or button
    HMENU m_hMenu;      // handle of the owner-draw menu
    UINT m_CtlType;     // type of the control -- ODT_COMBOBOX, ODT_LISTBOX, or ODT_MENU
    CFont* m_pFontOld;  // "old font" -- gets selected into DC before exit
    bool m_bMeasure;    // are we currently doing measure?
    bool m_bDelFont;    // need to delete font when done?
    CRgn* m_pRgnOld;    // old clipping region

    // controllig CWnd object (list box, combobox, button)
    CWnd* GetWnd()
    {
        return CWnd::FromHandle(m_hWnd);
    }

    // null all variables
    void PrivateClear();

    // finish startup work
    void StartFont();

    // computes item font
    void CreateFont();

    // computes item height & width
    void ComputeHYText();

    // list box or combo box has strings?
    bool HasStrings();

public:
    DWORD m_itemData;  // "item data" from DrawItemStruct
    CRect m_rect;      // drawing rectangle
    int m_index;       // item index
    CString m_line;    // item text

    bool m_bCtrlFocused;  // is control in focus?
    bool m_bChecked;      // menu item or CheckBox/RadioButton checked
    bool m_bSelected;     // is item selected?
    bool m_bFocused;      // is item focused?
    bool m_bDisabled;     // is item disabled?

    CDC m_dc;            // device context (DC for drawing)
    CFont* m_pFont;      // current font
    COLORREF m_colBack;  // backgrownd color
    COLORREF m_colText;  // text color

    int m_hText;  // text height
    int m_yText;  // text y-offset from the top

    explicit TLCBDraw(UINT CtlType);
    ~TLCBDraw();

    // are we a listbox/combobox?
    bool IsLC();

    // are we a button?
    bool IsButton();

    // are we a menu?
    bool IsMenu();

    // is item valid?
    bool IsValid();

    // returns a MeasureItemStruct for initial measuring
    LPMEASUREITEMSTRUCT GetMIS();

    // set m_hWnd and m_hMenu
    void MiniStart(HANDLE handle);

    // start drawing
    bool StartDraw(LPDRAWITEMSTRUCT lpDrawItemStruct, bool bFocusDetected = false);

    // start measuring
    void StartMeasure(HANDLE handle, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

    // process the "Compare" event
    int CompareItems(LPCOMPAREITEMSTRUCT lpCompareItemStruct);

    // draws button borders
    void StartDrawButton();

    // selects the drawing region in m_dc
    void StartRegion();

    // selects font and remembers it for possible future deletion
    void SelectFont(CFont* pFont, bool bDelFont = true);

    // resets m_dc, clears the structure
    void Finish();

    // default item text
    void GetItemText();

    // default draw function
    void Draw(const CUIntArray& arrTab);

    // default measure function
    CSize Measure();
};
}  // namespace AFLibGui
