#pragma once

#include "AFLibStrings.h"

/////////////////////////////////////////////////////////////////////////////
// GUI-related global functions and constants

class CRichEditCtrl;

namespace AFLib {
class T3State;
}

namespace AFLibGui {  // frequently used colors
const COLORREF colorBlack = RGB(0, 0, 0);
const COLORREF colorWhite = RGB(255, 255, 255);
const COLORREF colorGrayL = RGB(192, 192, 192);
const COLORREF colorGrayM = RGB(159, 159, 159);
const COLORREF colorGrayD = RGB(128, 128, 128);
const COLORREF colorAuto = (COLORREF)-1;

inline COLORREF colorLight()
{
    return GetSysColor(COLOR_BTNHILIGHT);
}

inline COLORREF colorLighty()
{
    return GetSysColor(COLOR_3DLIGHT);
}

inline COLORREF colorNormal()
{
    return GetSysColor(COLOR_BTNFACE);
}

inline COLORREF colorDark()
{
    return GetSysColor(COLOR_BTNSHADOW);
}

inline COLORREF colorDarkest()
{
    return GetSysColor(COLOR_3DDKSHADOW);
}

inline COLORREF colorText()
{
    return GetSysColor(COLOR_BTNTEXT);
}

inline COLORREF colorWindow()
{
    return GetSysColor(COLOR_WINDOW);
}

inline COLORREF colorWindowText()
{
    return GetSysColor(COLOR_WINDOWTEXT);
}

inline COLORREF colorHighlight()
{
    return GetSysColor(COLOR_HIGHLIGHT);
}

inline COLORREF colorHighlightText()
{
    return GetSysColor(COLOR_HIGHLIGHTTEXT);
}

// is DC monochrome?
AFLIB bool IsMono(CDC* pDC);

// set clipping rectangle (in logical units)
AFLIB void SetClipRect(CDC* pDC, const CRect& rect);

// draw text in a rectangle, according to current CDC align flags
AFLIB void TextInRect(CDC* pDC, const CRect& rect, LPCTSTR str);

// draw "thicker" text
AFLIB void DrawGlowingText(CDC* pDC, int x, int y, LPCTSTR str, COLORREF clrGlow = -1);

// GetTextExtent on Unicode strings
AFLIB int GetTextWidthW(CDC* pDC, LPCWSTR s, int length = -1);

// enough colors for the program?
AFLIB bool ColorsOk(CWnd* pWnd);

// black on the printer; default text color on the screen
AFLIB COLORREF GetColorAutoFg(CDC* pDC);

// white on the printer; default window color on the screen
AFLIB COLORREF GetColorAutoBg(CDC* pDC);

// makes a color lighter by taking the black out
AFLIB COLORREF GetColorLighter(COLORREF color, int index);

// find a mean of two colors
AFLIB COLORREF GetColorMean(COLORREF c1, COLORREF c2, double coefficient = 0.5);

// get a measure of lightness for a given color
AFLIB int GetColorGray(COLORREF color);

// how different are the given two colors?
AFLIB int GetColorDiff(COLORREF c1, COLORREF c2);

// create a halftone brush
AFLIB void CreateHalftoneBrush(CBrush& brush, COLORREF colorL = colorAuto, COLORREF colorD = colorAuto);

// adds a bitmap to a rich edit control
AFLIB void AddBitmapToRichEdit(CRichEditCtrl& wndRich, HBITMAP hBitmap, const CSize& sz);

// load font according to typeface, size (in pixels) and attributes.
AFLIB void CreateAbsoluteFont(CFont& font, LPCTSTR face, int pixelSize, bool bBold, bool bItalic);

// same, but size is in "points" (multiplied by 10).  If pDC is NULL, screen is used
AFLIB void CreateLogicalFont(CFont& font, CDC* pDC, LPCTSTR face, double pointSize, bool bBold, bool bItalic);

// create fixed-width font
AFLIB void CreateCourierFont(CFont& font, CDC* pDC);

// get font size
AFLIB CSize GetFontSize(CDC* pDC, CFont* pFont, LPCTSTR str);

// just get the height of the font (if null, current font)
AFLIB int GetFontHeight(CDC* pDC, CFont* pFont = NULL);

// create an alternating ("dotted") pen
AFLIB void CreateDottedPen(CPen& pen, COLORREF color);

// draw horizontal line that looks "3d"
AFLIB void Draw3dLine(CDC* pDC, int x1, int x2, int y);

// draw "focus" (dotted black rectangle)
AFLIB void DrawFocusRect(CDC* pDC, const CRect& rect, int thickness = 1, COLORREF color = colorAuto);

// draw a thick line using a brush
AFLIB void AndrewLineTo(CDC* pDC, int x, int y);
AFLIB void AndrewLineTo(CDC* pDC, const CPoint& pt);

// helper function for AndrewLineTo(): get offset from center line
AFLIB CSize AndrewGetOffset(const CPoint& p1, const CPoint& p2, int sz);

// move window; do not change Z-order
AFLIB bool AndrewMoveWindow(CWnd* pWnd, int x, int y, int cx, int cy, bool bRepaint = true);
AFLIB bool AndrewMoveWindow(CWnd* pWnd, const CRect& rect, bool bRepaint = true);

// use Browse dialog
AFLIB bool BrowseForDir(CString& dir, LPCTSTR prompt = NULL);

// DDX checkbox to a boolean variable
AFLIB void DDX_Check(CDataExchange* pDX, UINT idc, bool& value);

// DDX checkbox to a 3-state
AFLIB void DDX_Check(CDataExchange* pDX, UINT idc, AFLib::T3State& value);

// DDX a directory
AFLIB void DDX_Dir(CDataExchange* pDX, UINT idc, CString& value, bool bCreateOk, bool bUndefOk);

// DDX a bunch of checkboxes into a bitmask
AFLIB void DDX_Mask(CDataExchange* pDX, UINT idc, int count, int& value, int errorID);

// performs DDX if data is Nan / input string is empty; returns true is DDX is finished
AFLIB bool DDX_EmptyDbl(CDataExchange* pDX, UINT idc, double& value, bool bUndefOk);

// DDX an edit box that's a percentage value (the value shown is 100 times the valus stored)
AFLIB void DDX_Percentage(CDataExchange* pDX, UINT idc, double& value);

// same as DDV_MinMaxDouble, but value, minVal, and maxVal are multiplied by 100
AFLIB void DDV_MinMaxPercentage(CDataExchange* pDX, double const& value, double minVal, double maxVal);

// set min and max of a spin button (if exists)
AFLIB void DDX_SpinMinMax(CDataExchange* pDX, UINT idc, int minVal, int maxVal);

// DDX a combo box; data is the "item data"
AFLIB void DDX_CBData(CDataExchange* pDX, UINT idc, int& value);

// get window class name
AFLIB CString GetWindowClass(HWND hWnd);

// for tool tips -- find interface child window from point
AFLIB HWND FindLiveChildFromPoint(HWND hWnd, POINT pt);

// center rectangle with regards to another rectangle
AFLIB void CenterRectOnRect(CRect& rect, const CRect& rectMain);

// find a string among menu items
AFLIB int FindMenuItem(HMENU hMenu, LPCTSTR what);

// insert menu from hSource into hDest before index
AFLIB void CopyMenu(HMENU hSource, HMENU hDest, int index);
AFLIB void CopyMenu(int idSource, HMENU hDest, int index);

// check if given key will trigger a menu response
AFLIB bool IsKeyHotInMenu(HMENU hMenu, TCHAR ch);

// show a menu as a response to a right-click
AFLIB void RightClickMenu(CWnd* pWnd, HMENU hMenu, CPoint point);

// change font in dialog template (in place); fails if the new name can't fit
AFLIB bool ChangeTemplateFont(LPDLGTEMPLATE pTemplate, LPCTSTR face, int size);

// create an arrow-plus cursor
AFLIB HCURSOR GetArrowPlusCursor();

// if log is not empty, show it in a window
AFLIB void DisplayLog(LPCTSTR message, LPCTSTR log);
AFLIB void DisplayLog(UINT idsMessage, LPCTSTR log);

// load string, display message box
AFLIB UINT AfxMB(UINT ids, AFLIB_NINE_STRINGS);
AFLIB UINT AfxMB2(UINT nType, UINT ids, AFLIB_NINE_STRINGS);

// show message box
AFLIB UINT AfxMB(LPCTSTR msg, UINT idHelp = 0);
AFLIB UINT AfxMB2(UINT nType, LPCTSTR msg, UINT idHelp = 0);

// show message box about a duplicate title
AFLIB UINT AfxMBDupTitle();

// check if DEVMODE printing structure is Unicode
AFLIB bool IsDevModeUnicode(LPCVOID pDevMode);

// check if DEVNAMES printing structure is Unicode
AFLIB bool IsDevNamesUnicode(LPDEVNAMES pDevNames);

// convert DEVMODE printing structure to Unicode (always allocate new memory block)
AFLIB HGLOBAL DevMode2W(HGLOBAL hDevMode);

// convert DEVMODE printing structure to Ascii (always allocate new memory block)
AFLIB HGLOBAL DevMode2A(HGLOBAL hDevMode);

// convert DEVNAMES printing structure to Unicode (always allocate new memory block)
AFLIB HGLOBAL DevNames2W(HGLOBAL hDevNames);

// convert DEVNAMES printing structure to Ascii (always allocate new memory block)
AFLIB HGLOBAL DevNames2A(HGLOBAL hDevNames);
}  // namespace AFLibGui
