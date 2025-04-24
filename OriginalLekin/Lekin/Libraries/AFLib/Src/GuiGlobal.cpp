#include "StdAfx.h"

#include "AFLibGlobal.h"

#include "AFLibDCStateSaver.h"
#include "AFLibDlgLog.h"
#include "AFLibExcMessage.h"
#include "AFLibGuiGlobal.h"
#include "AFLibImageAF.h"
#include "AFLibMathGlobal.h"
#include "AFLibMenuItemInfo.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibMath::Rint;

namespace {
class TWndClassFunc
{
private:
    HWND m_hWnd;

public:
    explicit TWndClassFunc(HWND hWnd) : m_hWnd(hWnd)
    {}
    void operator()(LPTSTR buffer, int length)
    {
        ::GetClassName(m_hWnd, buffer, length);
    }
};

class TMenuItemFunc
{
private:
    HMENU m_hMenu;
    int m_index;

public:
    TMenuItemFunc(HMENU hMenu, int index) : m_hMenu(hMenu), m_index(index)
    {}
    int operator()(LPTSTR buffer, int length)
    {
        return ::GetMenuString(m_hMenu, m_index, buffer, length, MF_BYPOSITION);
    }
};
}  // namespace

/////////////////////////////////////////////////////////////////////////////
// DC

namespace AFLibGui {
bool IsMono(CDC* pDC)
{
    return pDC != NULL && pDC->GetDeviceCaps(NUMCOLORS) == 2;
}

void SetClipRect(CDC* pDC, const CRect& rect)
{
    CRect rectClip(rect);
    LPtoDP(pDC->m_hDC, LPPOINT(&rectClip), 2);

    CRgn rgn;
    rgn.CreateRectRgn(rectClip.left, rectClip.top, rectClip.right, rectClip.bottom);
    pDC->SelectClipRgn(&rgn, RGN_AND);
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Text

namespace AFLibGui {
void TextInRect(CDC* pDC, const CRect& rect, LPCTSTR str)
{
    if (IsEmptyStr(str)) return;
    UINT align1 = pDC->GetTextAlign();
    UINT align2 = DT_LEFT;
    if (align1 & TA_CENTER)
        align2 = DT_CENTER;
    else if (align1 & TA_RIGHT)
        align2 = DT_RIGHT;

    CRect rect2(rect);
    pDC->SetTextAlign(TA_LEFT);
    pDC->DrawText(str, rect2, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | align2);
    pDC->SetTextAlign(align1);
}

void DrawGlowingText(CDC* pDC, int x, int y, LPCTSTR str, COLORREF clrGlow)
{
    COLORREF clrSaved = pDC->GetTextColor();

    pDC->SetTextColor(clrGlow == -1 ? GetColorAutoBg(pDC) : clrGlow);

    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) pDC->TextOut(x + dx, y + dy, str);

    pDC->SetTextColor(clrSaved);
    pDC->TextOut(x, y, str);
}

int GetTextWidthW(CDC* pDC, LPCWSTR s, int length)
{
    CSize sz(0, 0);
    ::GetTextExtentPoint32W(pDC->m_hAttribDC, s, length < 0 ? wcslen(s) : length, &sz);
    return sz.cx;
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Color Manipulation

namespace AFLibGui {
bool ColorsOk(CWnd* pWnd)
{
    CDC* pDC = pWnd->GetDC();
    int colors = pDC->GetDeviceCaps(NUMCOLORS);
    pWnd->ReleaseDC(pDC);
    return colors < 0 || colors > 256;
}

COLORREF GetColorAutoFg(CDC* pDC)
{
    return pDC->IsPrinting() ? colorBlack : colorText();
}

COLORREF GetColorAutoBg(CDC* pDC)
{
    return pDC->IsPrinting() ? colorWhite : colorWindow();
}

COLORREF GetColorLighter(COLORREF color, int index)
{
    if (index == 0) return color;

    COLORREF color2 = 0;
    int kMax = 0;

    for (int i = 0; i < 24; i += 8) kMax = max(kMax, int((color >> i) & 0xFF));
    kMax = kMax == 0 ? 96 : Rint(kMax * 0.66);
    if (index == 1) kMax = Rint(kMax * 0.66);

    for (int i = 0; i < 24; i += 8)
    {
        int k = (color >> i) & 0xFF;
        if (k == 0) k = kMax;
        color2 |= k << i;
    }

    return color2;
}

COLORREF GetColorMean(COLORREF c1, COLORREF c2, double coefficient)
{
    COLORREF c = 0;

    for (int i = 0; i < 3; ++i)
    {
        int mask = 0xFF << (i * 8);
        c += Rint((c1 & mask) * (1 - coefficient) + (c2 & mask) * coefficient) & mask;
    }

    return c;
}

int GetColorGray(COLORREF color)
{
    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);
    int mx = max(r, max(g, b));
    return (2 * r + 2 * g + b + 2 * mx) / 7;
}

int GetColorDiff(COLORREF c1, COLORREF c2)
{
    int s = 0;

    for (int i = 0; i < 24; i += 8) s += abs(int((c1 >> i) & 0xFF) - int((c2 >> i) & 0xFF));

    return s;
}

void CreateHalftoneBrush(CBrush& brush, COLORREF colorL, COLORREF colorD)
{
    CImageAF image;
    image.CreateHalftone(colorL, colorD);
    image.MakeBrush(brush);
}

void AddBitmapToRichEdit(CRichEditCtrl& wndRich, HBITMAP hBitmap, const CSize& sz)
{
    tagSTGMEDIUM data = {0};
    data.hBitmap = hBitmap;
    DWORD tymed = TYMED_GDI;
    CLIPFORMAT cfFormat = CF_BITMAP;

    data.tymed = tymed;
    COleDataSource* pDataSource = new COleDataSource;
    pDataSource->CacheData(cfFormat, &data);

    LPDATAOBJECT pDataObject = NULL;
    IOleClientSite* pOleClientSite = NULL;
    LPLOCKBYTES pLockBytes = NULL;
    IStorage* pStorage = NULL;
    IOleObject* pOleObject = NULL;
    CException* pExcThrown = NULL;

    try
    {
        pDataObject = LPDATAOBJECT(pDataSource->GetInterface(&IID_IDataObject));
        SCODE sc = GetScode(wndRich.GetIRichEditOle()->GetClientSite(&pOleClientSite));
        if (sc != S_OK) AfxThrowOleException(sc);

        sc = ::CreateILockBytesOnHGlobal(NULL, true, &pLockBytes);
        if (sc != S_OK) AfxThrowOleException(sc);
        ASSERT(pLockBytes != NULL);

        sc = ::StgCreateDocfileOnILockBytes(
            pLockBytes, STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage);
        if (sc != S_OK) AfxThrowOleException(sc);
        ASSERT(pStorage != NULL);

        FORMATETC fm = {0};
        fm.cfFormat = cfFormat;
        fm.ptd = NULL;
        fm.dwAspect = DVASPECT_CONTENT;
        fm.lindex = -1;
        fm.tymed = tymed;

        sc = ::OleCreateStaticFromData(pDataObject, IID_IOleObject, OLERENDER_FORMAT, &fm, pOleClientSite,
            pStorage, reinterpret_cast<LPVOID*>(&pOleObject));
        if (sc != S_OK) AfxThrowOleException(sc);

        CLSID clsid;
        sc = pOleObject->GetUserClassID(&clsid);
        if (sc != S_OK) AfxThrowOleException(sc);

        REOBJECT reobject = {0};
        reobject.cbStruct = sizeof(REOBJECT);
        reobject.clsid = clsid;
        reobject.cp = REO_CP_SELECTION;
        reobject.dvaspect = DVASPECT_CONTENT;
        reobject.poleobj = pOleObject;
        reobject.polesite = pOleClientSite;
        reobject.sizel = sz;
        reobject.dwFlags = REO_BELOWBASELINE;
        reobject.pstg = pStorage;

        sc = GetScode(wndRich.GetIRichEditOle()->InsertObject(&reobject));
        if (sc != S_OK) AfxThrowOleException(sc);
    }
    catch (CException* pExc)
    {
        pExcThrown = pExc;
    }

    if (pOleObject != NULL) pOleObject->Release();
    if (pStorage != NULL) pStorage->Release();
    if (pLockBytes != NULL) pLockBytes->Release();
    if (pOleClientSite != NULL) pOleClientSite->Release();
    if (pDataObject != NULL) pDataObject->Release();

    if (pExcThrown != NULL)
    {
#ifdef _DEBUG
        CString s = GetExceptionMsg(pExcThrown);
        TRACE(s);
#endif
        throw pExcThrown;
    }
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Font

namespace AFLibGui {
void CreateAbsoluteFont(CFont& font, LPCTSTR face, int pixelSize, bool bBold, bool bItalic)
{
    LOGFONT logFont = {0};
    logFont.lfWeight = bBold ? 700 : 400;
    logFont.lfItalic = bItalic;
    _tcscpy_s(logFont.lfFaceName, LF_FACESIZE, face);

    logFont.lfCharSet = DEFAULT_CHARSET;  // ANSI_CHARSET;
    logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality = PROOF_QUALITY;
    logFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    logFont.lfHeight = pixelSize;
    font.CreateFontIndirect(&logFont);
}

void CreateLogicalFont(CFont& font, CDC* pDC, LPCTSTR face, double pointSize, bool bBold, bool bItalic)
{
    CWnd* pWnd = NULL;
    if (pDC == NULL)
    {
        pWnd = CWnd::GetDesktopWindow();
        pDC = pWnd->GetDC();
    }

    int pixelSize = -Rint(pointSize * pDC->GetDeviceCaps(LOGPIXELSY) / 72);
    if (pWnd != NULL) pWnd->ReleaseDC(pDC);
    CreateAbsoluteFont(font, face, pixelSize, bBold, bItalic);
}

void CreateCourierFont(CFont& font, CDC* pDC)
{
    CreateLogicalFont(font, pDC, _T("Courier New"), 8, false, false);
}

CSize GetFontSize(CDC* pDC, CFont* pFont, LPCTSTR str)
{
    HWND hWnd = NULL;
    HDC hDC = NULL;

    if (pDC == NULL)
    {
        hWnd = CWnd::GetDesktopWindow()->m_hWnd;
        hDC = ::GetDC(hWnd);
    }
    else
        hDC = pDC->m_hAttribDC;

    ASSERT(hDC != NULL);

    HGDIOBJ hFontOld = NULL;
    if (pFont != NULL) hFontOld = ::SelectObject(hDC, pFont->m_hObject);

    CSize sz;
    ::GetTextExtentPoint32(hDC, str, _tcslen(str), &sz);

    if (hFontOld != NULL) ::SelectObject(hDC, hFontOld);

    if (hWnd != NULL) ::ReleaseDC(hWnd, hDC);

    return sz;
}

int GetFontHeight(CDC* pDC, CFont* pFont)
{
    return GetFontSize(pDC, pFont, strFontTest).cy;
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Lines

namespace AFLibGui {
void CreateDottedPen(CPen& pen, COLORREF color)
{
    LOGBRUSH logBrush = {0};
    logBrush.lbStyle = BS_SOLID;
    logBrush.lbColor = color;
    pen.CreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &logBrush);
}

void Draw3dLine(CDC* pDC, int x1, int x2, int y)
{
    CPen penLight(PS_SOLID, 1, colorLight());
    CPen penDark(PS_SOLID, 1, colorDark());
    CPen* pPenOld = pDC->SelectObject(&penDark);

    pDC->MoveTo(x1, y);
    pDC->LineTo(x2, y);
    pDC->SelectObject(&penLight);
    pDC->MoveTo(x1, y + 1);
    pDC->LineTo(x2, y + 1);
    pDC->SelectObject(pPenOld);
}

void DrawFocusRect(CDC* pDC, const CRect& rect, int thickness, COLORREF color)
{
    CPen pen;
    CreateDottedPen(pen, color == colorAuto ? GetColorAutoFg(pDC) : color);

    CBrush brush;
    brush.CreateStockObject(NULL_BRUSH);
    TDCStateSaver dcss(pDC);

    pDC->SelectObject(&pen);
    pDC->SelectObject(&brush);
    pDC->Rectangle(rect);
    if (thickness <= 1) return;

    CRect rect2 = rect;
    for (; thickness > 1; --thickness)
    {
        rect2.DeflateRect(1, 1);
        pDC->Rectangle(rect2);
    }
}

CSize AndrewGetOffset(const CPoint& p1, const CPoint& p2, int sz)
{
    CSize dp0 = p2 - p1;
    double temp = sqrt(double(dp0.cx * dp0.cx + dp0.cy * dp0.cy)) / sz;

    return CSize(Rint(dp0.cy / temp), Rint(-dp0.cx / temp));
}

void AndrewLineTo(CDC* pDC, int x, int y)
{
    AndrewLineTo(pDC, CPoint(x, y));
}

void AndrewLineTo(CDC* pDC, const CPoint& pt)
{
    CPen* pPenOld = pDC->GetCurrentPen();
    LOGPEN logPen;
    pPenOld->GetLogPen(&logPen);
    COLORREF color = logPen.lopnColor;

    if (pDC->GetNearestColor(color) == color)
    {
        pDC->LineTo(pt);
        return;
    }

    CPoint pt2 = pDC->GetCurrentPosition();
    int d1 = logPen.lopnWidth.x / 2;
    int d2 = logPen.lopnWidth.x - d1;
    CSize dpCross1 = AndrewGetOffset(pt, pt2, d1);
    CSize dpCross2 = d1 == d2 ? -dpCross1 : AndrewGetOffset(pt, pt2, -d2);
    CSize dpLengthwise(-dpCross2.cy, dpCross2.cx);

    // Check if physical width = 0.  d2 >= d1, so no need to check dpCross1
    CSize dpPhys(dpCross2);
    pDC->LPtoDP(&dpPhys);
    if (dpPhys.cx == 0 && dpPhys.cy == 0)
    {
        pDC->LineTo(pt);
        return;
    }

    CPen pen;
    CBrush brush;
    TDCStateSaver dcss(pDC);

    pen.CreateStockObject(NULL_PEN);
    pDC->SelectObject(&pen);
    brush.CreateSolidBrush(color);
    pDC->SelectObject(&brush);

    CPoint arrPoint[6] = {
        pt + dpCross1, pt + dpLengthwise, pt + dpCross2, pt2 + dpCross2, pt2 - dpLengthwise, pt2 + dpCross1};

    pDC->Polygon(arrPoint, 6);
    pDC->MoveTo(pt);
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Window

namespace AFLibGui {
bool AndrewMoveWindow(CWnd* pWnd, int x, int y, int cx, int cy, bool bRepaint)
{
    int flags = SWP_NOZORDER;

    if (!bRepaint) flags |= SWP_NOREDRAW;
    return pWnd->SetWindowPos(&CWnd::wndTop, x, y, cx, cy, flags) != 0;
}

bool AndrewMoveWindow(CWnd* pWnd, const CRect& rect, bool bRepaint)
{
    return AndrewMoveWindow(pWnd, rect.left, rect.top, rect.Width(), rect.Height(), bRepaint);
}

CString GetWindowClass(HWND hWnd)
{
    return GetUglyString(TWndClassFunc(hWnd));
}

HWND FindLiveChildFromPoint(HWND hWnd, POINT pt)
{
    ::ClientToScreen(hWnd, &pt);
    HWND hWndChild = ::GetWindow(hWnd, GW_CHILD);

    for (; hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
    {
        CRect rect;
        ::GetWindowRect(hWndChild, rect);
        if (!rect.PtInRect(pt)) continue;

        // ok, we are in rect, but need to check the window
        // if control ID is 0 or IDC_STATIC, no good
        UINT idc = ::GetDlgCtrlID(hWndChild);
        if (idc == 0 || (idc & 0xFFFF) == (IDC_STATIC & 0xFFFF)) continue;

        // if window class is "Static", no good either
        CString sClass = GetWindowClass(hWndChild);
        if (sClass.CompareNoCase(_T("Static")) == 0) continue;

        // if this is a groupbox, also ignore
        if (sClass.CompareNoCase(_T("Button")) == 0 &&
            (::GetWindowLong(hWndChild, GWL_STYLE) & 0x0F) == BS_GROUPBOX)
            continue;

        return hWndChild;
    }

    return NULL;
}

void CenterRectOnRect(CRect& rect, const CRect& rectMain)
{
    rect.MoveToXY(rectMain.left + (rectMain.Width() - rect.Width()) / 2,
        rectMain.top + (rectMain.Height() - rect.Height()) / 2);
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Menu

namespace AFLibGui {
int FindMenuItem(HMENU hMenu, LPCTSTR what)
{
    int i = GetMenuItemCount(hMenu);
    while (--i >= 0)
    {
        CString s = GetNiceString(TMenuItemFunc(hMenu, i));
        s.Replace(_T("&"), strEmpty);
        if (s.CompareNoCase(what) == 0) break;
    }
    return i;
}

void CopyMenu(HMENU hSource, HMENU hDest, int index)
{
    if (hSource == NULL) return;
    if (index < 0) index = GetMenuItemCount(hDest);

    for (int i = 0; i < GetMenuItemCount(hSource); ++i)
    {
        try
        {
            TMenuItemInfo mii(hSource, i, TMenuItemInfo::tByPos);

            if (mii.GetSubmenu() != NULL)
            {
                HMENU hOld = mii.GetSubmenu();
                HMENU hNew = CreateMenu();
                CopyMenu(hOld, hNew, 0);
                mii.SetSubmenu(hNew);
            }

            mii.Insert(hDest, i + index, TMenuItemInfo::tByPos);
        }
        catch (CExcMessage* pExc)
        {
            pExc->Delete();
            break;
        }
    }
}

void CopyMenu(int idSource, HMENU hDest, int index)
{
    CMenu menuSource;
    menuSource.LoadMenu(idSource);
    CopyMenu(menuSource.m_hMenu, hDest, index);
}

bool IsKeyHotInMenu(HMENU hMenu, TCHAR ch)
{
    for (int i = 0; i < GetMenuItemCount(hMenu); ++i)
    {
        try
        {
            CString s = GetNiceString(TMenuItemFunc(hMenu, i));
            if (IsKeyHotInString(s, ch)) return true;
        }
        catch (CExcMessage* pExc)
        {
            pExc->Delete();
            break;
        }
    }
    return false;
}

void RightClickMenu(CWnd* pWnd, HMENU hMenu, CPoint point)
{
    pWnd->ClientToScreen(&point);
    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, *pWnd, NULL);
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Log Display

namespace AFLibGui {
void DisplayLog(LPCTSTR message, LPCTSTR log)
{
    if (IsEmptyStr(log)) return;
    CDlgLog dlg;
    dlg.m_alert = message;
    dlg.m_log = log;
    dlg.DoModal();
}

void DisplayLog(UINT idsMessage, LPCTSTR log)
{
    CString message(MAKEINTRESOURCE(idsMessage));
    DisplayLog(message, log);
}
}  // namespace AFLibGui

/////////////////////////////////////////////////////////////////////////////
// Other

namespace {
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg != BFFM_INITIALIZED) return 0;
    ::SendMessage(hWnd, BFFM_SETSELECTION, WPARAM(true), LPARAM(lpData));
    return 0;
}
}  // namespace

namespace AFLibGui {
bool ChangeTemplateFont(LPDLGTEMPLATE pTemplate, LPCTSTR face, int size)
{
    CDialogTemplate dt(pTemplate);
    DWORD oldSize = dt.m_dwTemplateSize;

    dt.SetFont(face, size);
    if (dt.m_dwTemplateSize > oldSize) return false;

    LPVOID pTemplateNew = GlobalLock(dt.m_hTemplate);
    memcpy(pTemplate, pTemplateNew, dt.m_dwTemplateSize);
    GlobalUnlock(dt.m_hTemplate);
    return true;
}

bool BrowseForDir(CString& dir, LPCTSTR prompt)
{
    CString prompt2 = IsEmptyStr(prompt) ? LocalAfxString(IDS_SELECT_DIR) : prompt;
    BROWSEINFO binfo = {NULL};

    // check dir, do NOT create
    if (!CheckDirectory(dir, false)) dir = _T("C:\\");

    binfo.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
    binfo.pszDisplayName = NULL;
    binfo.lpszTitle = prompt2;
    binfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
    binfo.lpfn = BrowseCallbackProc;
    binfo.lParam = LPARAM(LPCTSTR(dir));

    // a modal dialog pops up a here...
    TModalCounter modalCounter;
    LPITEMIDLIST itemDist = SHBrowseForFolder(&binfo);
    if (itemDist == NULL) return false;

    CString dirNew;
    bool bOk = SHGetPathFromIDList(itemDist, dirNew.GetBuffer(MAX_PATH + 1)) != 0;
    dirNew.ReleaseBuffer();
    if (!bOk) return false;

    dir = dirNew;
    AppendSlash(dir);
    return true;
}

HCURSOR GetArrowPlusCursor()
{
    return LoadCursor(AFLibDLL.hModule, MAKEINTRESOURCE(IDU_ARROW_PLUS));
}
}  // namespace AFLibGui
