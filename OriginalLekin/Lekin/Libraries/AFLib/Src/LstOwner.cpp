#include "StdAfx.h"

#include "AFLibLstOwner.h"

#include "AFLibArrays.h"
#include "AFLibDCStateSaver.h"
#include "AFLibGuiGlobal.h"
#include "DlgListHelper.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibGui::CLstOwner;

IMPLEMENT_DYNAMIC(CLstOwner, super)

CLstOwner::CLstOwner() : m_draw(ODT_LISTBOX), m_rectDrop(0, 0, 0, 0)
{
    m_bMultipleSelection = m_bMultipleColumn = m_bMayEdit = false;
    m_bFocusDetected = m_bDragEnabled = m_bCopyEnabled = false;
    m_edt = edtNone;
    m_pDlgEdit = NULL;
    m_iFrom = m_iDrop = -1;
    m_dirScroll = 0;
    m_drg = drgNone;
    m_bCopy = false;
    m_bEditKilled = false;
    m_firstTabIDC = 0;
}

void CLstOwner::FillBadNames(CStringArray& arrTitle, int indexExclude)
{
    for (int i = 0; i < GetCount(); ++i)
        if (i != indexExclude)
        {
            CString s;
            GetText(i, s);
            arrTitle.Add(s);
        }

    SortStrings(arrTitle);
}

void CLstOwner::CreateUniqueTitle(CString& title)
{
    CStringArray arrTitle;
    FillBadNames(arrTitle, -1);
    AFLib::CreateUniqueTitle(title, arrTitle);
}

void CLstOwner::ResetItemHeight()
{
    LPMEASUREITEMSTRUCT pMIS = m_draw.GetMIS();
    MeasureItem(pMIS);
    SetItemHeight(0, pMIS->itemHeight);
}

int CLstOwner::GetSelCount()
{
    return IsMultipleSelection() ? super::GetSelCount() : GetCurSel() >= 0 ? 1 : 0;
}

int CLstOwner::GetFirstSel()
{
    if (!IsMultipleSelection()) return GetCurSel();

    for (int i = 0; i < GetCount(); ++i)
        if (GetSel(i)) return i;

    return -1;
}

int CLstOwner::GetSingleSel()
{
    return GetSelCount() == 1 ? GetFirstSel() : -1;
}

void CLstOwner::SetSingleSel(int index)
{
    if (!IsMultipleSelection())
    {
        SetCurSel(index);
        return;
    }

    SetSel(-1, false);
    if (index >= 0) SetSel(index, true);
}

int CLstOwner::AddItemData(LPCTSTR s, int data)
{
    int index = AddString(s);
    SetItemData(index, data);
    return index;
}

int CLstOwner::AddData(LPVOID data)
{
    int index = AddString(strEmpty);
    SetItemDataPtr(index, data);
    return index;
}

LPVOID CLstOwner::GetSingleData()
{
    int i = GetSingleSel();
    if (i < 0) return NULL;
    return GetItemDataPtr(i);
}

CString CLstOwner::GetSingleString()
{
    CString s;
    int i = GetSingleSel();
    if (i >= 0) GetText(i, s);
    return s;
}

int CLstOwner::ItemFromPointAF(const CPoint& pt, int& dirScroll)
{
    dirScroll = 0;
    if (GetCount() == 0) return -1;

    int top = GetTopIndex();
    CRect rectItem;
    GetItemRect(top, rectItem);
    CRect rectCli;
    GetClientRect(rectCli);
    int index = -1;

    if (m_bMultipleColumn)
    {
        if (pt.y < rectCli.top || pt.y >= rectCli.bottom) return -1;

        int nHor = rectCli.Width() / rectItem.Width();
        int nVert = (rectCli.Height() / rectItem.Height());
        int col = min(int(pt.x / rectItem.Width()), nHor);

        if (pt.x < rectCli.left)
        {
            dirScroll = -nVert;
            col = 0;
        }
        else if (pt.x >= rectCli.right)
            dirScroll = nVert;

        index = top + col * nVert + (pt.y - rectItem.top) / rectItem.Height();

        int maxCol = (GetCount() - top - 1) / nVert;

        if (col > maxCol)
        {
            index -= (col - maxCol) * nVert;
            col = maxCol;
        }
    }
    else
    {
        if (pt.x < rectCli.left || pt.x >= rectCli.right) return -1;

        if (pt.y < rectCli.top)
        {
            dirScroll = -1;
            return top;
        }

        if (pt.y >= rectCli.bottom)
        {
            dirScroll = 1;
            index = top + rectCli.Height() / rectItem.Height();
        }
        else
            index = top + (pt.y - rectItem.top) / rectItem.Height();
    }

    return min(index, GetCount());
}

int CLstOwner::ItemFromPointAF(const CPoint& pt)
{
    int temp = 0;
    int index = ItemFromPointAF(pt, temp);
    return temp == 0 && index < GetCount() ? index : -1;
}

void CLstOwner::ChangeDrop(int iDrop)
{
    bool bCopy = m_bCopyEnabled && GetKeyState(VK_CONTROL) < 0;
    if (iDrop == m_iDrop && bCopy == m_bCopy) return;

    m_iDrop = iDrop;
    m_bCopy = bCopy;
    bool bOk = OnDrag(m_iFrom, m_iDrop, m_bCopy);
    m_curSet.SetCursor(bOk, m_bCopy);
}

void CLstOwner::SendSelchange()
{
    UINT idc = GetDlgCtrlID();
    GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(idc, LBN_SELCHANGE), LPARAM(m_hWnd));
}

bool CLstOwner::StartDrag(const CPoint& pt)
{
    int index = ItemFromPointAF(pt);
    if (index < 0 || index != GetSingleSel() || !OnBeginDrag(index)) return false;

    SendSelchange();
    m_curSet.Init();
    m_iFrom = index;
    SetCapture();
    m_drg = drgDrag;
    return true;
}

void CLstOwner::XorDrop()
{
    if (m_rectDrop.IsRectNull()) return;

    CDC* pDC = GetDC();
    CPen pen;

    TDCStateSaver dcss(pDC);
    pen.CreatePen(PS_SOLID, 3, colorWhite);
    pDC->SelectObject(&pen);
    pDC->SetROP2(R2_XORPEN);

    pDC->MoveTo(m_rectDrop.TopLeft());
    pDC->LineTo(m_rectDrop.BottomRight());
    ReleaseDC(pDC);
}

void CLstOwner::DrawDrop()
{
    if (!m_rectDrop.IsRectNull()) return;

    if (m_iDrop == -1)
        m_rectDrop.SetRectEmpty();
    else
    {
        if (m_iDrop == GetCount())
        {
            GetItemRect(m_iDrop - 1, m_rectDrop);
            m_rectDrop.top = m_rectDrop.bottom;
        }
        else
        {
            GetItemRect(m_iDrop, m_rectDrop);
            m_rectDrop.bottom = m_rectDrop.top;
        }

        CRect rectCli;
        GetClientRect(rectCli);
        m_rectDrop.right = min(m_rectDrop.right, rectCli.right);
    }

    XorDrop();
}

void CLstOwner::EraseDrop()
{
    XorDrop();
    m_rectDrop.SetRectEmpty();
}

void CLstOwner::EscapeHelper(bool bDrop)
{
    int iFrom = m_iFrom;

    EraseDrop();
    m_drg = drgNone;
    m_iFrom = m_iDrop = -1;
    m_bCopy = false;
    m_curSet.Clear();
    m_curSet.RestoreCursor();
    ReleaseCapture();
    OnEndDrag(iFrom, bDrop);
}

void CLstOwner::Drop()
{
    bool bCopy = m_bCopy;
    int iFrom = m_iFrom;
    int iDrop = m_iDrop;

    EscapeHelper(true);
    OnDrop(iFrom, iDrop, bCopy);
}

bool CLstOwner::StartEditHelper()
{
    m_draw.m_index = GetSingleSel();
    ASSERT(m_draw.m_index >= 0);

    GetItemText();
    if (!OnStartEdit(m_draw.m_index, m_draw.m_line)) return false;

    if (m_pDlgEdit == NULL)
        m_pDlgEdit = new CDlgListHelper(this);
    else if (m_pDlgEdit->m_hWnd != NULL)
        m_pDlgEdit->DestroyWindow();

    CRect rect;
    GetItemRect(m_draw.m_index, rect);
    ClientToScreen(rect);

    m_pDlgEdit->m_index = m_draw.m_index;
    m_pDlgEdit->m_data = m_draw.m_line;
    m_pDlgEdit->Create(rect);
    return true;
}

bool CLstOwner::StartEdit()
{
    if (GetSelCount() != 1) return false;

    m_draw.MiniStart(m_hWnd);
    bool bOk = StartEditHelper();
    m_draw.Finish();
    return bOk;
}

bool CLstOwner::InEdit()
{
    return m_pDlgEdit != NULL && m_pDlgEdit->m_hWnd != NULL;
}

void CLstOwner::KillStartEdit()
{
    KillTimer(timerStartEdit);
    m_bEditKilled = true;
}

void CLstOwner::CancelEdit()
{
    if (InEdit()) m_pDlgEdit->DestroyWindow();
}

void CLstOwner::CancelDrag()
{
    if (InDrag()) Escape();
}

BEGIN_MESSAGE_MAP(CLstOwner, super)
ON_WM_CREATE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_LBUTTONDBLCLK()
ON_WM_RBUTTONDOWN()
ON_WM_RBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_TIMER()
ON_WM_CAPTURECHANGED()
ON_WM_PAINT()
ON_WM_KILLFOCUS()
ON_WM_SETFOCUS()
ON_WM_DESTROY()
END_MESSAGE_MAP()

void CLstOwner::OnPaint()
{
    EraseDrop();
    super::OnPaint();
    DrawDrop();
}

void CLstOwner::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    m_draw.StartDraw(lpDrawItemStruct, m_bFocusDetected);

    try
    {
        GetItemText();
        m_draw.StartRegion();
        Draw(&m_draw.m_dc);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    m_draw.Finish();
}

void CLstOwner::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    m_draw.StartMeasure(m_hWnd, lpMeasureItemStruct);

    try
    {
        GetItemText();
        lpMeasureItemStruct->itemHeight = Measure(lpMeasureItemStruct->itemID);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    m_draw.Finish();
}

int CLstOwner::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
    return m_draw.CompareItems(lpCompareItemStruct);
}

void CLstOwner::PreSubclassWindow()
{
    super::PreSubclassWindow();
    OnInit();
}

int CLstOwner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;
    OnInit();
    return 0;
}

void CLstOwner::OnLButtonDown(UINT nFlags, CPoint point)
{
    KillStartEdit();
    int preClick = m_edt == edtClick ? GetSingleSel() : -1;

    super::OnLButtonDown(nFlags, point);
    if (!InEdit() && m_bDragEnabled) m_drg = drgTrig;

    if (preClick != -1 && GetSingleSel() == preClick && ItemFromPointAF(point) == preClick) m_bMayEdit = true;
}

void CLstOwner::OnLButtonUp(UINT nFlags, CPoint point)
{
    KillStartEdit();
    if (InDrag())
    {
        if (m_iDrop != -1)
            Drop();
        else
            Escape();
    }
    else
    {
        super::OnLButtonUp(nFlags, point);
        if (m_bMayEdit)
        {
            SetTimer(timerStartEdit, 500, NULL);
            m_bEditKilled = false;
        }
    }

    m_bMayEdit = false;
    m_drg = drgNone;
}

void CLstOwner::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    KillStartEdit();
    if (m_edt == edtDblClick && StartEdit()) return;
    OnDoubleClick(ItemFromPointAF(point), nFlags, point);
}

void CLstOwner::OnRButtonDown(UINT nFlags, CPoint point)
{
    KillStartEdit();
    CancelDrag();
    super::OnRButtonDown(nFlags, point);
}

void CLstOwner::OnRButtonUp(UINT nFlags, CPoint point)
{
    KillStartEdit();
    OnRightClick(ItemFromPointAF(point), nFlags, point);
}

void CLstOwner::OnMouseMove(UINT nFlags, CPoint point)
{
    switch (m_drg)
    {
        case drgTrig:
            m_bMayEdit = false;
            if (StartDrag(point)) break;
            m_drg = drgNone;

        case drgNone:
            super::OnMouseMove(nFlags, point);
            return;
    }

    int iDrop = ItemFromPointAF(point, m_dirScroll);
    if (iDrop == m_iDrop) return;

    EraseDrop();
    ChangeDrop(iDrop);
    DrawDrop();
}

void CLstOwner::OnTimer(UINT nIDEvent)
{
    switch (nIDEvent)
    {
        case timerUpdate:
        {
            if (!InDrag() || m_dirScroll == 0) return;

            int iDrop = max(min(m_iDrop + m_dirScroll, GetCount()), 0);
            if (iDrop == m_iDrop) return;

            int newTop = max(min(GetTopIndex() + m_dirScroll, GetCount() - 1), 0);
            if (newTop == GetTopIndex()) return;

            EraseDrop();
            m_iDrop = -1;
            SetTopIndex(newTop);
            ChangeDrop(iDrop);
            DrawDrop();
            return;
        }

        case timerStartEdit:
            KillTimer(timerStartEdit);
            if (m_bEditKilled) return;
            StartEdit();
            return;

        case timerCommitEdit:
        {
            if (m_pDlgEdit == NULL) return;
            int index = m_pDlgEdit->m_index;
            CString line = m_pDlgEdit->m_data;

            if (OnEndEdit(index, line)) CommitEditChanges(index, line);
            return;
        }
    }

    super::OnTimer(nIDEvent);
}

void CLstOwner::GetItemText()
{
    m_draw.GetItemText();
}

void CLstOwner::Draw(CDC* pDC)
{
    m_draw.Draw(m_arrTab);
}

void CLstOwner::OnInit()
{
    DWORD style = GetStyle();
    m_bMultipleSelection = (style & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) != 0;
    m_bMultipleColumn = (style & LBS_MULTICOLUMN) != 0;

    if (style & LBS_OWNERDRAWFIXED)
        ResetItemHeight();
    else
        ASSERT(style & LBS_OWNERDRAWVARIABLE);

    if (m_firstTabIDC != 0)
    {
        int x = 0;

        for (int i = m_firstTabIDC + 1;; ++i)
        {
            CRect rect;
            CWnd* pWndLabel = GetParent()->GetDlgItem(i);
            if (pWndLabel == NULL) break;
            pWndLabel->GetWindowRect(rect);
            ScreenToClient(rect);
            m_arrTab.Add(rect.left - x);
            x = rect.left;
        }
    }

    if (m_bDragEnabled) SetTimer(timerUpdate, m_curSet.m_timerMS, NULL);
}

int CLstOwner::Measure(int index)
{
    return m_draw.Measure().cy;
}

bool CLstOwner::OnDrag(int iFrom, int iDrop, bool bCopy)
{
    return true;
}

bool CLstOwner::OnDrop(int iFrom, int iDrop, bool bCopy)
{
    if (!OnDrag(iFrom, iDrop, bCopy)) return false;

    CString s;
    GetText(iFrom, s);
    int iData = GetItemData(iFrom);

    if (!bCopy)
    {
        if (iDrop == iFrom || iDrop == iFrom + 1) return false;

        DeleteString(iFrom);
        if (iDrop > iFrom) --iDrop;
    }

    InsertString(iDrop, s);
    SetItemData(iDrop, iData);
    SetSingleSel(iDrop);
    return true;
}

bool CLstOwner::OnBeginDrag(int iFrom)
{
    return true;
}

void CLstOwner::OnEndDrag(int, bool)
{}

void CLstOwner::OnDoubleClick(int, UINT nFlags, const CPoint& point)
{
    super::OnLButtonDblClk(nFlags, point);
}

void CLstOwner::OnRightClick(int index, UINT nFlags, const CPoint& point)
{
    super::OnRButtonUp(nFlags, point);
    SetFocus();

    if (!IsMultipleSelection() && index >= 0 && index < GetCount() && index != GetSingleSel())
    {
        SetSingleSel(index);
        SendSelchange();
    }
}

bool CLstOwner::OnStartEdit(int, CString&)
{
    return true;
}

bool CLstOwner::OnEndEdit(int, CString&)
{
    return true;
}

void CLstOwner::CommitEditChanges(int index, LPCTSTR item)
{
    DWORD itemData = GetItemData(index);
    int topIndex = GetTopIndex();

    SetRedraw(false);
    DeleteString(index);
    InsertString(index, item);
    SetItemData(index, itemData);
    SetSingleSel(index);
    SetTopIndex(topIndex);
    SetRedraw(true);
}

BOOL CLstOwner::PreTranslateMessage(MSG* pMsg)
{
    if (InDrag())
    {
        switch (pMsg->message)
        {
            case WM_KEYDOWN:
                switch (pMsg->wParam)
                {
                    case VK_ESCAPE:
                    {
                        Escape();
                        return true;
                    }
                    case VK_CONTROL:
                        ChangeDrop(m_iDrop);
                }
                break;

            case WM_KEYUP:
                if (pMsg->wParam == VK_CONTROL) ChangeDrop(m_iDrop);
                break;
        }
    }

    return super::PreTranslateMessage(pMsg);
}

void CLstOwner::OnCaptureChanged(CWnd* pWnd)
{
    super::OnCaptureChanged(pWnd);
    CancelDrag();
}

void CLstOwner::OnKillFocus(CWnd* pNewWnd)
{
    super::OnKillFocus(pNewWnd);
    if (m_bFocusDetected) RedrawWindow();
}

void CLstOwner::OnSetFocus(CWnd* pOldWnd)
{
    super::OnSetFocus(pOldWnd);
    if (m_bFocusDetected) RedrawWindow();
}

void CLstOwner::OnDestroy()
{
    KillTimer(timerUpdate);

    if (m_pDlgEdit != NULL)
    {
        if (m_pDlgEdit->m_hWnd != NULL) m_pDlgEdit->DestroyWindow();
        delete m_pDlgEdit;
        m_pDlgEdit = NULL;
    }

    super::OnDestroy();
}
