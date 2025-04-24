#pragma once

#include "AFLibCursorSet.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CTreeOwner tree control
// A tree control that helps implement drag-and-drop editing

namespace AFLibGui {
class AFLIB CTreeOwner : public CTreeCtrl
{
private:
    typedef CTreeCtrl super;
    DEFINE_COPY_AND_ASSIGN(CTreeOwner);

public:
    // type of the drag supported: none, drop hilight, insert
    enum TTypeDrag
    {
        dNone,
        dDrop,
        dInsert
    };

private:
    bool m_bCallOnSelect;  // call OnSelect()?

    TTypeDrag m_type;     // drag type
    bool m_bCopyEnabled;  // is ctrl-dragging enabled?

    HTREEITEM m_hItemFrom;  // item being dragged
    HTREEITEM m_hItemDrop;  // item where it was dragged to
    bool m_bSpecialDrop;    // special condition for "Drop":
                            //   dDrop -- not on an item; dInsert -- below last visible item
    bool m_bCopy;           // ctrl-dragged currently?
    HTREEITEM m_hItemPlus;  // item where mouse pointer is sitting on the "plus" (expand/collapse icon)

    bool m_bCanEdit;        // there was no double-click, so we can edit
    bool m_bEditCancelled;  // editing was cancelled

    // initialize the timer
    void InitTimer();

    // draw the drop indicator
    void DrawDrop(const CPoint& pt, bool bScroll);

    // end dragging
    void EscapeHelper(bool bDrop);

    // end dragging and drop
    void Drop();

    // end dragging, do NOT drop
    void Escape()
    {
        EscapeHelper(false);
    }

    // helper for GetLastVisibleItem() and GetLastUnhiddenItem()
    HTREEITEM PrivateGetLastItem(bool bVisible);

    // changes colors, then calls PreDraw() if m_bOwnerDraw is set
    bool PrivatePreDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw);

    // calls PostDraw() if m_bOwnerDraw is set, then OnDrawOwnerDrop() if m_bOwnerDrop is set
    void PrivatePostDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw);

    // helper for FindPath() and FindPathApprox()
    HTREEITEM PrivateFindPath(LPCTSTR path, bool bApprox);

    // call OnSelect() if necessary
    void CallOnSelect(HTREEITEM hItem);

protected:
    // class to save and restore the state of "call OnSelect()"
    class TCallOnSelect
    {
    private:
        CTreeOwner* m_pHost;   // host of this object
        bool m_bCallOnSelect;  // saved state of m_bCallOnSelect of the host

    public:
        TCallOnSelect(CTreeOwner* pHost, bool bCallOnSelect);
        ~TCallOnSelect();
    };

    TCursorSet m_curSet;     // cursor set
    int m_timerExpMS;        // number of ms before an item is expanded/collapsed
    bool m_bOwnerDraw;       // owner-draw?
    bool m_bOwnerDrop;       // "owner" drop target implemented
    CString m_strPathDelim;  // the path delimiter, if "path navigation" is implemented

    // cancel dragging, if it is under way
    void CancelDrag();

public:
    CTreeOwner();

    // get last visible item
    HTREEITEM GetLastVisibleItem()
    {
        return PrivateGetLastItem(true);
    }

    // get last item that's not in collapsed branches
    HTREEITEM GetLastUnhiddenItem()
    {
        return PrivateGetLastItem(false);
    }

    // is item completely visible in the tree view?
    bool IsItemFullyVisible(HTREEITEM hItem);

    // returns item containing the point; otherwise null
    HTREEITEM HitTestAF(const CPoint& point);

    // find a child item by title
    HTREEITEM FindChildItem(HTREEITEM hItem, LPCTSTR title);

    // select item, do NOT call OnSelect()
    bool SelectItemNoFeedback(HTREEITEM hItem);

    // set drag type
    void SetDragType(TTypeDrag type, bool bCopyEnabled);

    // fill the array of prohibited titles for an item (to avoid duplicate names)
    void FillBadNames(HTREEITEM hItemParent, CStringArray& arrTitle, HTREEITEM hItemExclude);

    // modify title to create a unique one
    void CreateUniqueTitle(HTREEITEM hItemParent, CString& title);

    // get the full path of a tree item
    CString GetPath(HTREEITEM hItem);

    // find the item for a path
    HTREEITEM FindPath(LPCTSTR path)
    {
        return PrivateFindPath(path, false);
    }

    // find the item for a path or a parent
    HTREEITEM FindApproxPath(LPCTSTR path)
    {
        return PrivateFindPath(path, true);
    }

    // return drag type
    TTypeDrag GetDragType()
    {
        return m_type;
    }

    // is ctrl-gragging enabled?
    bool IsCopyEnabled()
    {
        return m_bCopyEnabled;
    }

    // is dragging now?
    bool InDrag()
    {
        return m_hItemFrom != NULL;
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    // called in Subclass() and Create()
    virtual void OnInit();

    // called when selected scan changes
    virtual void OnSelect(HTREEITEM hItem);

    // begin edit: return true to start editing the label
    virtual bool OnBeginEdit(HTREEITEM hItem);

    // end edit: verify the new name and rename the label
    virtual bool OnEndEdit(HTREEITEM hItem, LPCTSTR label);

    // begin drag: return true to start dragging the item
    virtual bool OnBeginDrag(HTREEITEM hItemFrom);

    // item dragged from hItemFrom into hItemDrop, but not dropped yet
    virtual bool OnDrag(HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bSpecialDrop, bool bCopy);

    // drop hItemFrom into hItemDrop
    virtual bool OnDrop(HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bSpecialDrop, bool bCopy);

    // called when drag has ended -- with either "drop" or "cancel"
    virtual void OnEndDrag(HTREEITEM hItemFrom, bool bDrop);

    // called on double click
    virtual void OnDoubleClick(HTREEITEM hItem, UINT nFlags, const CPoint& point);

    // called on right-click
    virtual void OnRightClick(HTREEITEM hItem, UINT nFlags, const CPoint& point);

    // get colors for the item
    virtual void OnGetColors(HTREEITEM hItem, UINT state, COLORREF& colorText, COLORREF& colorBack);

    // called before an item is drawn (calls OnGetColors()); return true to skip the default draw routine
    virtual bool PreDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw);

    // called after the item has been drawn
    virtual void PostDraw(CDC* pDC, HTREEITEM hItem, const CRect& rect, LPNMTVCUSTOMDRAW pDraw);

    // called to draw the "drop" item if owner-drop is on
    virtual void OnDrawOwnerDrop(CDC* pDC, HTREEITEM hItem, const CRect& rect);

    virtual void PreSubclassWindow();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnTvnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTvnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTvnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTvnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNmCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnTvmEditLabel(WPARAM wParam, LPARAM lParam);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CTreeOwner)
};
}  // namespace AFLibGui
