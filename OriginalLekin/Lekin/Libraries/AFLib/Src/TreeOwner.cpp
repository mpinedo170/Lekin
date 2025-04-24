#include "StdAfx.h"

#include "AFLibTreeOwner.h"

#include "AFLibArrays.h"
#include "AFLibDCStateSaver.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibStringTokenizerStr.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibGui::CTreeOwner;

/////////////////////////////////////////////////////////////////////////////
// CTreeOwner::TCallOnSelect

CTreeOwner::TCallOnSelect::TCallOnSelect(CTreeOwner* pHost, bool bCallOnSelect)
{
    m_pHost = pHost;
    m_bCallOnSelect = m_pHost->m_bCallOnSelect;
    m_pHost->m_bCallOnSelect = bCallOnSelect;
}

CTreeOwner::TCallOnSelect::~TCallOnSelect()
{
    m_pHost->m_bCallOnSelect = m_bCallOnSelect;
}

/////////////////////////////////////////////////////////////////////////////
// CTreeOwner

IMPLEMENT_DYNAMIC(CTreeOwner, super)

CTreeOwner::CTreeOwner()
{
    m_type = dNone;
    m_bCopyEnabled = false;
    m_hItemFrom = m_hItemDrop = m_hItemPlus = NULL;
    m_bOwnerDraw = m_bOwnerDrop = false;
    m_bSpecialDrop = m_bCopy = false;
    m_bCanEdit = m_bEditCancelled = false;
    m_timerExpMS = 1000;
    m_bCallOnSelect = true;
}

void CTreeOwner::OnInit()
{
    InitTimer();
}

void CTreeOwner::InitTimer()
{
    if (m_type == dNone)
        KillTimer(timerUpdate);
    else
        SetTimer(timerUpdate, m_curSet.m_timerMS, NULL);
}

HTREEITEM CTreeOwner::PrivateGetLastItem(bool bVisible)
{
    int i = bVisible ? GetVisibleCount() : MXI;
    HTREEITEM hItem = GetFirstVisibleItem();

    while (--i >= 0)
    {
        HTREEITEM hItem2 = GetNextVisibleItem(hItem);
        if (hItem2 == NULL) break;
        hItem = hItem2;
    }

    return hItem;
}

bool CTreeOwner::IsItemFullyVisible(HTREEITEM hItem)
{
    CRect rectClient;
    CRect rectItem;

    if (!GetItemRect(hItem, rectItem, false) || rectItem.top < 0) return false;

    GetClientRect(rectClient);
    return rectItem.bottom <= rectClient.Height();
}

void CTreeOwner::SetDragType(TTypeDrag type, bool bCopyEnabled)
{
    if (InDrag()) return;

    m_type = type;
    m_bCopyEnabled = m_type == dNone ? false : bCopyEnabled;
    if (m_hWnd != NULL) InitTimer();
}

void CTreeOwner::CallOnSelect(HTREEITEM hItem)
{
    if (m_bCallOnSelect) OnSelect(hItem);
}

bool CTreeOwner::SelectItemNoFeedback(HTREEITEM hItem)
{
    TCallOnSelect sz(this, false);
    return SelectItem(hItem) != 0;
}

void CTreeOwner::DrawDrop(const CPoint& pt, bool bScroll)
{
    if (!InDrag()) return;

    UINT flags = 0;
    HTREEITEM hItemDrop = HitTest(pt, &flags);
    bool bSpecialDrop = false;
    bool bCopy = m_bCopyEnabled && GetKeyState(VK_CONTROL) < 0;

    if (bScroll)
    {
        if (flags & TVHT_TOLEFT)
            SendMessage(WM_HSCROLL, SB_LINEUP, NULL);
        else if (flags & TVHT_TORIGHT)
            SendMessage(WM_HSCROLL, SB_LINEDOWN, NULL);
    }

    if (flags & TVHT_ABOVE)
    {
        if (bScroll) SendMessage(WM_VSCROLL, SB_LINEUP, NULL);
        hItemDrop = GetFirstVisibleItem();
    }
    else if (flags & TVHT_BELOW)
    {
        if (bScroll) SendMessage(WM_VSCROLL, SB_LINEDOWN, NULL);
        hItemDrop = GetLastVisibleItem();

        // for an insert mark, trigger "special position" if cursor is below the view
        //   and there's nowhere to scroll
        if (m_type == dInsert && GetNextVisibleItem(hItemDrop) == NULL && IsItemFullyVisible(hItemDrop))
            bSpecialDrop = true;
    }
    else if (flags & TVHT_NOWHERE)
        bSpecialDrop = true;
    else
    {  // for the "drop" mark, cursor on empty space means "drop to root"
        if (m_type == dDrop && !(flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT))) bSpecialDrop = true;
    }

    if (flags & TVHT_ONITEMBUTTON)
    {
        if (hItemDrop != m_hItemPlus)
        {
            KillTimer(timerMouseUpdate);
            if (ItemHasChildren(hItemDrop))
            {
                m_hItemPlus = hItemDrop;
                SetTimer(timerMouseUpdate, m_timerExpMS, NULL);
            }
        }
    }
    else
    {
        KillTimer(timerMouseUpdate);
        m_hItemPlus = NULL;
    }

    if (bSpecialDrop) hItemDrop = m_type == dDrop ? NULL : GetLastUnhiddenItem();

    if (m_hItemDrop == hItemDrop && m_bSpecialDrop == bSpecialDrop && m_bCopy == bCopy) return;

    m_hItemDrop = hItemDrop;
    m_bSpecialDrop = bSpecialDrop;
    m_bCopy = bCopy;

    if (m_type == dDrop)
        SelectDropTarget(m_hItemDrop);
    else
        SetInsertMark(m_hItemDrop, m_bSpecialDrop);

    bool bOk = OnDrag(m_hItemFrom, m_hItemDrop, m_bSpecialDrop, m_bCopy);
    m_curSet.SetCursor(bOk, m_bCopy);
}

void CTreeOwner::CancelDrag()
{
    if (InDrag()) Escape();
}

HTREEITEM CTreeOwner::HitTestAF(const CPoint& point)
{
    UINT flags = 0;
    HTREEITEM hItem = HitTest(point, &flags);

    return flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT) ? hItem : NULL;
}

void CTreeOwner::EscapeHelper(bool bDrop)
{
    HTREEITEM hItemFrom = m_hItemFrom;
    KillTimer(timerMouseUpdate);

    m_hItemFrom = m_hItemPlus = m_hItemDrop = NULL;
    m_bSpecialDrop = m_bCopy = false;
    SelectDropTarget(NULL);
    SetInsertMark(NULL);

    m_curSet.Clear();
    m_curSet.RestoreCursor();
    ReleaseCapture();
    OnEndDrag(hItemFrom, bDrop);
}

void CTreeOwner::Drop()
{
    HTREEITEM hItemFrom = m_hItemFrom;
    HTREEITEM hItemDrop = m_hItemDrop;
    bool bSpecialDrop = m_bSpecialDrop;
    bool bCopy = m_bCopy;
    EscapeHelper(true);
    OnDrop(hItemFrom, hItemDrop, bSpecialDrop, bCopy);
}

bool CTreeOwner::PrivatePreDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw)
{
    if (m_bOwnerDrop)
    {
        UINT state = pDraw->nmcd.uItemState;
        COLORREF& colorText = pDraw->clrText;
        COLORREF& colorBack = pDraw->clrTextBk;

        if (!IsWindowEnabled())
        {
            colorText = colorDark();
            colorBack = colorNormal();
        }
        else if (state & CDIS_SELECTED)
        {
            if (state & CDIS_FOCUS)
            {
                colorText = colorHighlightText();
                colorBack = colorHighlight();
            }
            else
            {
                colorText = GetTextColor();
                if (colorText == colorAuto) colorText = colorWindowText();
                colorBack = colorNormal();
            }
        }
        else
        {
            colorText = GetTextColor();
            if (colorText == colorAuto) colorText = colorWindowText();

            colorBack = GetBkColor();
            if (colorBack == colorAuto) colorBack = colorWindow();
        }
    }

    return m_bOwnerDraw ? PreDraw(pDC, hItem, rect, pDraw) : false;
}

void CTreeOwner::PrivatePostDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw)
{
    pDC->SetTextColor(pDraw->clrText);
    pDC->SetTextAlign(TA_LEFT | TA_TOP);

    if (m_bOwnerDraw) PostDraw(pDC, hItem, rect, pDraw);

    if (m_bOwnerDrop && hItem == GetDropHilightItem())
    {
        CBrush brush;
        brush.CreateStockObject(NULL_BRUSH);
        TDCStateSaver dcss(pDC);
        pDC->SelectObject(&brush);
        OnDrawOwnerDrop(pDC, hItem, rect);
    }
}

HTREEITEM CTreeOwner::FindChildItem(HTREEITEM hItem, LPCTSTR title)
{
    for (HTREEITEM hItem2 = GetChildItem(hItem); hItem2 != NULL; hItem2 = GetNextSiblingItem(hItem2))
        if (GetItemText(hItem2).CompareNoCase(title) == 0) return hItem2;

    return NULL;
}

void CTreeOwner::FillBadNames(HTREEITEM hItemParent, CStringArray& arrTitle, HTREEITEM hItemExclude)
{
    for (HTREEITEM hItem = GetChildItem(hItemParent); hItem != NULL; hItem = GetNextSiblingItem(hItem))
        if (hItem != hItemExclude) arrTitle.Add(GetItemText(hItem));
    SortStrings(arrTitle);
}

void CTreeOwner::CreateUniqueTitle(HTREEITEM hItemParent, CString& title)
{
    CStringArray arrTitle;
    FillBadNames(hItemParent, arrTitle, NULL);
    AFLib::CreateUniqueTitle(title, arrTitle);
}

HTREEITEM CTreeOwner::PrivateFindPath(LPCTSTR path, bool bApprox)
{
    ASSERT(!m_strPathDelim.IsEmpty());

    HTREEITEM hItem = NULL;
    AFLibIO::TStringTokenizerStr sp(path, m_strPathDelim);

    while (!sp.IsEof())
    {
        CString s = sp.ReadStr();
        HTREEITEM hItem2 = FindChildItem(hItem, s);

        if (hItem2 == NULL) return bApprox ? hItem : NULL;
        hItem = hItem2;
    }

    return hItem;
}

CString CTreeOwner::GetPath(HTREEITEM hItem)
{
    ASSERT(!m_strPathDelim.IsEmpty());

    CString path;
    for (; hItem != NULL; hItem = GetParentItem(hItem)) path = GetItemText(hItem) + m_strPathDelim + path;
    return path;
}

BEGIN_MESSAGE_MAP(CTreeOwner, super)
ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnTvnBeginLabelEdit)
ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnTvnEndLabelEdit)
ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBeginDrag)
ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelChanged)
ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNmCustomDraw)
ON_MESSAGE(TVM_EDITLABEL, OnTvmEditLabel)
ON_WM_CREATE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_LBUTTONDBLCLK()
ON_WM_RBUTTONDOWN()
ON_WM_RBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_TIMER()
ON_WM_CAPTURECHANGED()
ON_WM_DESTROY()
END_MESSAGE_MAP()

void CTreeOwner::PreSubclassWindow()
{
    super::PreSubclassWindow();
    OnInit();
}

int CTreeOwner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;
    OnInit();
    return 0;
}

BOOL CTreeOwner::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        CEdit* pEdit = GetEditControl();

        switch (pMsg->wParam)
        {
            case VK_ESCAPE:
                if (InDrag())
                {
                    Escape();
                    return true;
                }

                if (pEdit != NULL)
                {
                    m_bEditCancelled = true;
                    pEdit->DestroyWindow();
                    return true;
                }
                break;

            case VK_RETURN:
                if (pEdit == NULL) break;
                pEdit->DestroyWindow();
                return true;
        }
    }

    return super::PreTranslateMessage(pMsg);
}

void CTreeOwner::OnTvnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
    *pResult = 0;
    m_bEditCancelled = false;

    HTREEITEM hItem = pTVDispInfo->item.hItem;
    if (hItem == NULL || !m_bCanEdit || !OnBeginEdit(hItem)) *pResult = 1;
}

void CTreeOwner::OnTvnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
    const TVITEM& item = pTVDispInfo->item;
    *pResult = 0;
    if (m_bEditCancelled || item.pszText == NULL) return;

    if (OnEndEdit(item.hItem, item.pszText)) CallOnSelect(item.hItem);
}

void CTreeOwner::OnTvnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    *pResult = 0;

    HTREEITEM hItem = pNMTreeView->itemNew.hItem;
    if (hItem == NULL) return;
    SelectItem(hItem);

    if (InDrag() || GetEditControl() != NULL || !OnBeginDrag(hItem)) return;
    m_hItemFrom = hItem;
    m_curSet.Init();
    SetCapture();
}

void CTreeOwner::OnNmCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTVCUSTOMDRAW pDraw = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);
    *pResult = CDRF_DODEFAULT;

    switch (pDraw->nmcd.dwDrawStage)
    {
        case CDDS_PREPAINT:
            if (m_bOwnerDraw || m_bOwnerDrop) *pResult = CDRF_NOTIFYITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT:
        case CDDS_ITEMPOSTPAINT:
        {
            CDC dc;
            dc.Attach(pDraw->nmcd.hdc);
            HTREEITEM hItem = HTREEITEM(pDraw->nmcd.dwItemSpec);
            CRect rect;
            GetItemRect(hItem, rect, true);
            TDCStateSaver dcss(&dc);

            try
            {
                dc.SetBkMode(TRANSPARENT);
                if (pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
                    *pResult =
                        PrivatePreDraw(&dc, hItem, rect, pDraw) ? CDRF_SKIPDEFAULT : CDRF_NOTIFYPOSTPAINT;
                else
                    PrivatePostDraw(&dc, hItem, rect, pDraw);
            }
            catch (CException* pExc)
            {
                pExc->Delete();
            }

            dcss.Close();
            dc.Detach();
            break;
        }
    }
}

LRESULT CTreeOwner::OnTvmEditLabel(WPARAM wParam, LPARAM lParam)
{
    m_bCanEdit = true;
    return Default();
}

void CTreeOwner::OnTimer(UINT nIDEvent)
{
    switch (nIDEvent)
    {
        case timerUpdate:
        {
            if (!InDrag()) return;
            CPoint pt;
            GetCursorPos(&pt);
            ScreenToClient(&pt);
            DrawDrop(pt, true);
            return;
        }

        case timerMouseUpdate:
            KillTimer(timerMouseUpdate);
            if (!InDrag() || m_hItemPlus == NULL) return;
            Expand(m_hItemPlus, TVE_TOGGLE);
    }

    super::OnTimer(nIDEvent);
}

void CTreeOwner::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_bCanEdit = true;
    super::OnLButtonDown(nFlags, point);
}

void CTreeOwner::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (InDrag())
    {
        if (m_hItemDrop != NULL || m_bSpecialDrop)
            Drop();
        else
            Escape();
    }
    else
        super::OnLButtonUp(nFlags, point);
}

void CTreeOwner::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CancelDrag();
    m_bCanEdit = false;
    OnDoubleClick(HitTestAF(point), nFlags, point);
}

void CTreeOwner::OnRButtonDown(UINT nFlags, CPoint point)
{
    CancelDrag();
}

void CTreeOwner::OnRButtonUp(UINT nFlags, CPoint point)
{
    OnRightClick(HitTestAF(point), nFlags, point);
}

void CTreeOwner::OnMouseMove(UINT nFlags, CPoint point)
{
    if (InDrag())
        DrawDrop(point, false);
    else
        super::OnMouseMove(nFlags, point);
}

void CTreeOwner::OnCaptureChanged(CWnd* pWnd)
{
    super::OnCaptureChanged(pWnd);
    CancelDrag();
}

void CTreeOwner::OnDestroy()
{
    KillTimer(timerUpdate);
    super::OnDestroy();
}

void CTreeOwner::OnTvnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    *pResult = 0;
    CallOnSelect(pNMTreeView->itemNew.hItem);
}

void CTreeOwner::OnGetColors(HTREEITEM hItem, UINT state, COLORREF& colorText, COLORREF& colorBack)
{}

bool CTreeOwner::PreDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw)
{
    OnGetColors(hItem, pDraw->nmcd.uItemState, pDraw->clrText, pDraw->clrTextBk);
    return false;
}

void CTreeOwner::PostDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw)
{}

void CTreeOwner::OnDrawOwnerDrop(CDC* pDC, HTREEITEM hItem, const CRect& rect)
{
    CPen pen(PS_SOLID, 0, colorText());
    CPen* pPenOld = pDC->SelectObject(&pen);
    pDC->Rectangle(rect);
    pDC->SelectObject(pPenOld);
}

bool CTreeOwner::OnDrag(HTREEITEM, HTREEITEM, bool, bool)
{
    return true;
}

bool CTreeOwner::OnDrop(HTREEITEM, HTREEITEM, bool, bool)
{
    return false;
}

void CTreeOwner::OnEndDrag(HTREEITEM hItemFrom, bool)
{
    CallOnSelect(hItemFrom);
}

void CTreeOwner::OnDoubleClick(HTREEITEM, UINT nFlags, const CPoint& point)
{
    super::OnLButtonDblClk(nFlags, point);
}

void CTreeOwner::OnRightClick(HTREEITEM, UINT nFlags, const CPoint& point)
{}

void CTreeOwner::OnSelect(HTREEITEM hItem)
{}

bool CTreeOwner::OnBeginEdit(HTREEITEM hItem)
{
    return true;
}

bool CTreeOwner::OnEndEdit(HTREEITEM hItem, LPCTSTR label)
{
    return false;
}

bool CTreeOwner::OnBeginDrag(HTREEITEM hItemFrom)
{
    return true;
}
