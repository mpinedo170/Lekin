#include "StdAfx.h"

#include "AFLibWndAFHeader.h"

#include "AFLibDCStateSaver.h"
#include "AFLibFontDescriptor.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibImageAF.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibStringSpacer.h"
#include "Local.h"

using namespace AFLibGui;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;
using AFLibMath::Rint;

namespace {
const CString strInvisible = _T("U");
}

IMPLEMENT_DYNAMIC(CWndAFHeader, super)

/////////////////////////////////////////////////////////////////////////////
// CWndAFHeader::TColumn

CWndAFHeader::TColumn::TColumn()
{
    m_width = 0;
    m_bSort = m_bClose = m_bCenter = false;
    m_color = colorAuto;
}

/////////////////////////////////////////////////////////////////////////////
// CWndAFHeader::TColumnEx

bool CWndAFHeader::TColumnEx::DrawSquare(
    CDC* pDC, int& x, int side, TAction action, const CPoint& ptMouse, bool bDown)
{
    x -= side + 1;
    CRect rect(x, 0, x + side, side);

    bool bInside = rect.PtInRect(ptMouse) != 0;
    bool bSelected = m_sortOrder != 0 && action != aClose && (action == aSortAsc) == m_bAscending;

    bool bLower = bSelected || (bInside && bDown);
    bool bUpper = !bSelected && bInside && !bDown;
    bool bWhite = bSelected && !bDown;

    if (bWhite) pDC->FillSolidRect(rect, colorLight());

    if (bUpper || bLower)
    {
        pDC->MoveTo(x, side);
        pDC->SelectObject(bUpper ? m_pWnd->m_pPenLight : m_pWnd->m_pPenDark);
        pDC->LineTo(x, 0);
        pDC->LineTo(x + side, 0);
        pDC->SelectObject(bUpper ? m_pWnd->m_pPenDark : m_pWnd->m_pPenLight);
        pDC->LineTo(x + side, side);
        pDC->LineTo(x, side);
    }

    pDC->SelectObject(m_pWnd->m_pPenBlack);
    if (action == aClose)
    {
        pDC->MoveTo(x + 2, 2);
        pDC->LineTo(x + side - 1, side - 1);
        pDC->MoveTo(x + 2, side - 2);
        pDC->LineTo(x + side - 1, 1);
    }
    else
        m_pWnd->DrawTriangle(pDC, x + side / 2, 2, side - 4, action == aSortAsc, false);

    if (bInside && bDown)
    {
        m_pWnd->m_action = action;
        return true;
    }

    return false;
}

bool CWndAFHeader::TColumnEx::Paint(CDC* pDC, const CSize& sz, const CPoint& ptMouse, bool bButtons)
{
    int hFont = pDC->GetTextExtent(m_col.m_title).cy;
    int x = sz.cx / 2 + 1;

    pDC->SetTextAlign(TA_CENTER | TA_BOTTOM);
    pDC->SetTextColor(m_col.m_color != colorAuto ? m_col.m_color : colorText());

    bool bInside = ptMouse.x >= 0;
    bool bDown = bInside && m_pWnd->m_bLButtonDown;
    bool bTotalPressed = !m_bVisible && bDown;

    pDC->SelectObject(bTotalPressed ? m_pWnd->m_pPenDark : m_pWnd->m_pPenLight);
    pDC->MoveTo(0, 0);
    pDC->LineTo(0, sz.cy - 1);

    pDC->SelectObject(bTotalPressed ? m_pWnd->m_pPenLight : m_pWnd->m_pPenDark);
    pDC->MoveTo(sz.cx - 1, 0);
    pDC->LineTo(sz.cx - 1, sz.cy - 1);

    if (!m_bVisible)
    {
        if (!bTotalPressed) return false;
        m_pWnd->m_action = aClose;
        return true;
    }

    pDC->TextOut(x, sz.cy, m_col.m_title);
    if (!bButtons) return false;

    if (!bInside)
    {
        if (m_sortOrder == 0) return false;
        int totSort = m_pWnd->m_arrSort.GetSize();
        int h = sz.cy - hFont - 2;
        int hTri = (h - 3) * (totSort + 1 - m_sortOrder) / totSort + 3;
        int yTri = 1 + (h - hTri) / 2;
        m_pWnd->DrawTriangle(pDC, x, yTri, hTri, m_bAscending, true);
        return false;
    }

    bool bAction = false;
    int side = Rint(hFont * m_pWnd->m_coefButton);
    x = sz.cx - 1;

    if (m_col.m_bClose) bAction |= DrawSquare(pDC, x, side, aClose, ptMouse, bDown);

    if (m_col.m_bSort)
    {
        bAction |= DrawSquare(pDC, x, side, aSortDesc, ptMouse, bDown);
        bAction |= DrawSquare(pDC, x, side, aSortAsc, ptMouse, bDown);
    }

    return bAction;
}

int CWndAFHeader::TColumnEx::GetMinWidth(CDC* pDC)
{
    CSize sz = pDC->GetTextExtent(m_col.m_title);
    int wButton = Rint(sz.cy * m_pWnd->m_coefButton);

    int w = 2;
    if (m_col.m_bClose) w += wButton;
    if (m_col.m_bSort) w += wButton * 2;
    return max(w, int(sz.cx)) + 2;
}

/////////////////////////////////////////////////////////////////////////////
// CWndAFHeader

CWndAFHeader::CWndAFHeader()
{
    m_bMouseInside = m_bLButtonDown = false;
    m_action = aClose;
    m_actColumn = -1;
    m_pPenLight = m_pPenDark = m_pPenBlack = NULL;
    m_wClosed = 4;
    m_coefButton = 0.55;
}

CWndAFHeader::~CWndAFHeader()
{
    m_arrColumn.DestroyAll();
    m_arrSort.RemoveAll();
}

void CWndAFHeader::Create(CWnd* pWndParent, CWnd* pWndScroll)
{
    m_pWndScroll = pWndScroll;
    CRect rect(0, 0, 4, 4);
    CreateEx(WS_EX_CLIENTEDGE, NULL, strEmpty, WS_CHILD | WS_VISIBLE, rect, pWndParent, -1);
    SetTimer(timerMouseUpdate, 30, NULL);
}

CString CWndAFHeader::SaveState()
{
    TStringBuffer buffer;

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        TColumnEx* pCol = m_arrColumn[i];
        if (i > 0) buffer += chrSpace;

        if (!pCol->m_bVisible)
            buffer += strInvisible;
        else
            buffer.AppendInt(pCol->m_bAscending ? pCol->m_sortOrder : -pCol->m_sortOrder);
    }

    return LPCTSTR(buffer);
}

void CWndAFHeader::LoadState(LPCTSTR sPtr)
{
    TStringSpacer sp(sPtr);
    CIntArray arr;

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        if (sp.IsEof()) ThrowMessage(LocalAfxString(IDS_TOO_FEW_CMN));
        if (sp.TestSymbol(strInvisible))
        {
            arr.Add(INT_MAX);
            continue;
        }

        int a = sp.ReadInt();
        if (abs(a) > GetColumnCount()) ThrowMessage(strEmpty);
        arr.Add(a);
    }
    sp.TestEofHard();

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        TColumnEx* pCol = m_arrColumn[i];
        pCol->m_bVisible = pCol->m_bAscending = false;
        pCol->m_sortOrder = 0;

        int a = arr[i];
        if (a == INT_MAX) continue;
        pCol->m_bVisible = true;
        pCol->m_bAscending = a > 0;
        pCol->m_sortOrder = abs(a);
    }

    ResetSortOrder();
}

void CWndAFHeader::InsertColumn(int index, const TColumn& column)
{
    TColumnEx* pCol = new TColumnEx;
    pCol->m_col = column;
    pCol->m_sortOrder = 0;
    pCol->m_bAscending = true;
    pCol->m_bVisible = true;
    pCol->m_pWnd = this;
    m_arrColumn.InsertAt(index, pCol);
    ResetSortOrder();
}

void CWndAFHeader::AddColumn(const TColumn& column)
{
    InsertColumn(GetColumnCount(), column);
}

void CWndAFHeader::DeleteColumn(int index)
{
    m_arrColumn.DestroyAt(index);
    ResetSortOrder();
}

void CWndAFHeader::DeleteAllColumns()
{
    m_arrColumn.DestroyAll();
    ResetSortOrder();
}

const CWndAFHeader::TColumn& CWndAFHeader::GetColumn(int index)
{
    return m_arrColumn[index]->m_col;
}

void CWndAFHeader::SetColumn(int index, const TColumn& column)
{
    TColumnEx* pCol = m_arrColumn[index];
    pCol->m_col = column;
    if (!pCol->m_col.m_bClose) pCol->m_bVisible = true;
    if (!pCol->m_col.m_bSort) SetSortOrder(index, 0);
}

void CWndAFHeader::ShowColumn(int index, bool bShow)
{
    m_arrColumn[index]->m_bVisible = bShow;
    if (!bShow) SetSortOrder(index, 0);
    if (GetSafeHwnd() != NULL) Reshow();
}

void CWndAFHeader::SetSortOrder(int index, int sortOrder)
{
    SetSortOrder(index, sortOrder, m_arrColumn[index]->m_bAscending);
}

void CWndAFHeader::SetSortOrder(int index, int sortOrder, bool bAscending)
{
    TColumnEx* pCol = m_arrColumn[index];
    int oldOrder = pCol->m_sortOrder;
    if (oldOrder == sortOrder && pCol->m_bAscending == bAscending) return;

    pCol->m_sortOrder = sortOrder;
    pCol->m_bAscending = bAscending;

    if (oldOrder == sortOrder)
    {
        if (sortOrder != 0) ResetSortOrder();
        return;
    }

    for (int i = 0; i < GetColumnCount(); ++i)
    {
        if (i == index) continue;
        TColumnEx* pCol2 = m_arrColumn[i];
        if (pCol2->m_sortOrder == 0) continue;

        if (sortOrder != 0 && pCol2->m_sortOrder >= sortOrder) ++pCol2->m_sortOrder;
        if (oldOrder != 0 && pCol2->m_sortOrder > oldOrder) --pCol2->m_sortOrder;
    }
    ResetSortOrder();
}

void CWndAFHeader::GetSortOrder(int index, int& sortOrder, bool& bAscending)
{
    TColumnEx* pCol = m_arrColumn[index];
    sortOrder = pCol->m_sortOrder;
    bAscending = pCol->m_bAscending;
}

int CWndAFHeader::GetWidth(int index)
{
    TColumnEx* pCol = m_arrColumn[index];
    return pCol->m_bVisible ? pCol->m_col.m_width : m_wClosed;
}

void CWndAFHeader::GetUnscrolledInterval(int index, int& x1, int& x2)
{
    int s = 0;
    for (int i = 0; i < index; ++i) s += GetWidth(i);

    x1 = s;
    x2 = s + GetWidth(index);
}

void CWndAFHeader::GetInterval(int index, int& x1, int& x2)
{
    int shift = 0;
    if (m_pWndScroll != NULL)
        shift =
            m_pWndScroll->GetScrollPos(m_pWndScroll->IsKindOf(RUNTIME_CLASS(CScrollBar)) ? SB_CTL : SB_HORZ);
    GetUnscrolledInterval(index, x1, x2);
    x1 -= shift;
    x2 -= shift;
}

void CWndAFHeader::ResetSortOrder()
{
    m_arrSort.RemoveAll();

    for (int i = 1;; ++i)
    {
        int j = GetColumnCount();
        while (--j >= 0)
            if (m_arrColumn[j]->m_sortOrder == i) break;

        if (j < 0) break;
        TSort sort = {j, m_arrColumn[j]->m_bAscending};
        m_arrSort.Add(sort);
    }
    if (GetSafeHwnd() != NULL) Resort();
}

int CWndAFHeader::GetColumnMinWidth(int index, CDC* pDC)
{
    return m_arrColumn[index]->GetMinWidth(pDC);
}

void CWndAFHeader::DrawTriangle(CDC* pDC, int x, int y, int h, bool bAscending, bool b3D)
{
    int xShift = Rint(h * 0.577);
    if (!b3D) pDC->SelectObject(m_pPenBlack);

    if (bAscending)
    {
        pDC->MoveTo(x, y);
        if (b3D) pDC->SelectObject(m_pPenDark);
        pDC->LineTo(x - xShift, y + h);
        if (b3D) pDC->SelectObject(m_pPenLight);
        pDC->LineTo(x + xShift, y + h);
        pDC->LineTo(x, y);
        if (!b3D) pDC->SetPixel(x + xShift, y + h, colorDark());
    }
    else
    {
        pDC->MoveTo(x, y + h);
        if (b3D) pDC->SelectObject(m_pPenDark);
        pDC->LineTo(x - xShift, y);
        pDC->LineTo(x + xShift, y);
        if (b3D) pDC->SelectObject(m_pPenLight);
        pDC->LineTo(x, y + h);
        if (!b3D) pDC->SetPixel(x + xShift, y, colorDark());
    }
}

bool CWndAFHeader::HasButtons()
{
    for (int i = 0; i < GetColumnCount(); ++i)
    {
        const TColumn& col = m_arrColumn[i]->m_col;
        if (col.m_bClose || col.m_bSort) return true;
    }
    return false;
}

BEGIN_MESSAGE_MAP(CWndAFHeader, super)
ON_WM_PAINT()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_TIMER()
ON_MESSAGE(WM_SETFONT, OnSetFont)
ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()

void CWndAFHeader::OnPaint()
{
    int x1 = 0;
    int x2 = 0;
    bool bButtons = HasButtons();

    CPaintDC dc(this);
    CRect rectCli;
    GetClientRect(rectCli);
    CFont* pFont = GetFont();

    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);
    bool bMouse = rectCli.PtInRect(pt) != 0;
    m_actColumn = -1;

    m_pPenLight = new CPen(PS_SOLID, 0, colorLight());
    m_pPenDark = new CPen(PS_SOLID, 0, colorDark());
    m_pPenBlack = new CPen(PS_SOLID, 0, colorText());

    for (int index = 0; index < GetColumnCount(); ++index)
    {
        GetInterval(index, x1, x2);
        if (x2 < 0) continue;
        if (x1 > rectCli.right) break;

        CPoint ptMouse(-1, -1);
        if (bMouse && pt.x >= x1 && pt.x < x2)
        {
            ptMouse.x = pt.x - x1;
            ptMouse.y = pt.y;
        }

        CSize sz(x2 - x1, rectCli.Height());
        CImageAF img;
        img.CreateScreen(sz);
        img.Fill(colorNormal());

        CDC* pDCImg = img.GetCDC();
        TDCStateSaver dcss(pDCImg);
        pDCImg->SelectObject(pFont);

        if (m_arrColumn[index]->Paint(pDCImg, sz, ptMouse, bButtons)) m_actColumn = index;
        dcss.Close();
        CImageAF::ReleaseCDC(pDCImg);
        img.Draw(dc, x1, 0);
    }

    if (x2 < rectCli.right) dc.FillSolidRect(x2, 0, rectCli.right - x2, rectCli.Height(), colorNormal());
    delete m_pPenLight;
    delete m_pPenDark;
    delete m_pPenBlack;
    m_pPenLight = m_pPenDark = m_pPenBlack = NULL;
}

void CWndAFHeader::OnMouseMove(UINT nFlags, CPoint point)
{
    super::OnMouseMove(nFlags, point);
    m_bMouseInside = true;
    RedrawWindow();
}

void CWndAFHeader::OnLButtonDown(UINT nFlags, CPoint point)
{
    super::OnLButtonDown(nFlags, point);
    m_bLButtonDown = true;
    m_bMouseInside = true;
    RedrawWindow();
}

void CWndAFHeader::OnLButtonUp(UINT nFlags, CPoint point)
{
    super::OnLButtonUp(nFlags, point);
    if (!m_bLButtonDown) return;
    m_bLButtonDown = false;

    int actColumn = m_actColumn;
    m_actColumn = -1;

    if (actColumn == -1)
    {
        RedrawWindow();
        return;
    }

    if (m_action == aClose)
    {
        bool bShow = !IsVisible(actColumn);
        if (!OnShowColumn(actColumn, bShow)) return;
        ShowColumn(actColumn, bShow);
        OnChangeView();
    }
    else
    {
        bool bAscending = m_action == aSortAsc;
        if (!OnSortColumn(actColumn, bAscending)) return;

        int oldOrder;
        bool oldAsc;
        GetSortOrder(actColumn, oldOrder, oldAsc);
        SetSortOrder(actColumn, oldAsc == bAscending ? 1 : oldOrder, bAscending);
        OnChangeView();
    }

    RedrawWindow();
}

void CWndAFHeader::Reshow()
{}

void CWndAFHeader::Resort()
{}

bool CWndAFHeader::OnShowColumn(int column, bool bShow)
{
    return true;
}

bool CWndAFHeader::OnSortColumn(int column, bool bAscending)
{
    return true;
}

void CWndAFHeader::OnChangeView()
{}

void CWndAFHeader::OnTimer(UINT nIDEvent)
{
    super::OnTimer(nIDEvent);
    if (!m_bMouseInside) return;

    if (nIDEvent == timerMouseUpdate)
    {
        CRect rectCli;
        GetClientRect(rectCli);
        CPoint pt;
        GetCursorPos(&pt);
        ScreenToClient(&pt);
        bool bMouseInside = rectCli.PtInRect(pt) != 0;

        if (!bMouseInside && m_bMouseInside)
        {
            m_bMouseInside = false;
            RedrawWindow();
        }
    }
}

LRESULT CWndAFHeader::OnSetFont(WPARAM wParam, LPARAM lParam)
{
    m_hFont = HFONT(wParam);
    if (lParam) RedrawWindow();
    return 0;
}

LRESULT CWndAFHeader::OnGetFont(WPARAM wParam, LPARAM lParam)
{
    return LRESULT(m_hFont);
}
