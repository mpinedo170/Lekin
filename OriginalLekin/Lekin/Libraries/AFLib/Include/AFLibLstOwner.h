#pragma once

#include "AFLibCursorSet.h"
#include "AFLibLCBDraw.h"

namespace AFLibPrivate {
class CDlgListHelper;
}

/////////////////////////////////////////////////////////////////////////////
// CLstOwner list box
// Base class for owner draw list boxes; also implements drag-and-drop editing

namespace AFLibGui {
class AFLIB CLstOwner : public CListBox
{
private:
    typedef CListBox super;
    DEFINE_COPY_AND_ASSIGN(CLstOwner);

    bool m_bMultipleSelection;  // is multiple selection?
    bool m_bMultipleColumn;     // is multiple column?

    // small window for editing list box items
    AFLibPrivate::CDlgListHelper* m_pDlgEdit;

    // drag state
    enum TDrag  // drNone -- not dragging;
    {
        drgNone,
        drgTrig,
        drgDrag
    };            // drgTrig -- LButton down, have not moved yet;
    TDrag m_drg;  // drgDrag -- dragging

    bool m_bMayEdit;     // edit may be triggered by Mouse Up
    bool m_bEditKilled;  // do not invoke edit when timer comes up!

    int m_iFrom;       // this item is being dragged
    int m_iDrop;       // and here is where it was dragged to
    CRect m_rectDrop;  // drop position (y = m_rectDrop.top = m_rectDrop.bottom)
    int m_dirScroll;   // -1, 0, or 1 -- where the list box is being scrolled
    bool m_bCopy;      // ctrl-dragging

    // change and m_iDrop m_yDrop
    void ChangeDrop(int iDrop);

    // draw drop line (XOR drawing mode)
    void XorDrop();

    // make sure the drop is NOT drawn; draw it
    void DrawDrop();

    // make sure the drop is drawn; erase it
    void EraseDrop();

    // end dragging
    void EscapeHelper(bool bDrop);

    // end dragging and drop
    void Drop();

    // end dragging, do NOT drop
    void Escape()
    {
        EscapeHelper(false);
    }

    // destroy edit dialog
    void DestroyDlgEdit();

    // helper for StartEdit()
    bool StartEditHelper();

    // kill edit timer, set flag
    void KillStartEdit();

protected:
    TLCBDraw m_draw;      // keeper of the drawing information
    CUIntArray m_arrTab;  // array of tabulations
    TCursorSet m_curSet;  // cursor set

    // send notification to parent about selection change
    void SendSelchange();

public:
    CLstOwner();

    // fill the array with titles from the list; possibly exclude one index
    void FillBadNames(CStringArray& arrTitle, int indexExclude);

    // modify title to create a unique one
    void CreateUniqueTitle(CString& title);

    // check item height, reset it
    void ResetItemHeight();

    // get number of selected items
    int GetSelCount();

    // get the first (or only) selected item
    int GetFirstSel();

    // get the only selected item; if multile items selected, -1
    int GetSingleSel();

    // select only one item
    void SetSingleSel(int index);

    // add an empty string with a data pointer
    int AddData(LPVOID data);

    // add text with a data pointer
    int AddItemData(LPCTSTR s, int data);

    // get single selection's data pointer
    LPVOID GetSingleData();

    // get single selection's text
    CString GetSingleString();

    int m_firstTabIDC;      // IDC of the label for the first tab
    bool m_bFocusDetected;  // does drawing depend of whether the list box is in focus?
    bool m_bDragEnabled;    // is dragging enabled? set from the derived classes
    bool m_bCopyEnabled;    // is ctrl-dragging enabled?

    // supported types of user label edit
    enum TEdit
    {
        edtNone,
        edtClick,
        edtDblClick
    };
    TEdit m_edt;  // is user label edit enabled, and how?

    // is editing now?
    bool InEdit();

    // is dragging now?
    bool InDrag()
    {
        return m_drg == drgDrag;
    }

    // find iDrop given mouse position
    int ItemFromPointAF(const CPoint& pt, int& dirScroll);
    int ItemFromPointAF(const CPoint& pt);

    // is multiple selection?
    bool IsMultipleSelection()
    {
        return m_bMultipleSelection;
    }

    // is multiple column?
    bool IsMultipleColumn()
    {
        return m_bMultipleColumn;
    }

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void PreSubclassWindow();
    afx_msg void OnPaint();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnDestroy();

    // start editing
    bool StartEdit();

    // cancel edit, if it under way
    void CancelEdit();

    // cancel dragging, if it is under way
    void CancelDrag();

    // called in Subclass() and Create()
    virtual void OnInit();

    // start dragging
    virtual bool StartDrag(const CPoint& pt);

    // get item text for the item being drawn
    virtual void GetItemText();

    // DrawItem wrapper
    virtual void Draw(CDC* pDC);

    // MeasureItem wrapper
    virtual int Measure(int index);

    // begin drag: return true to start dragging the item
    virtual bool OnBeginDrag(int iFrom);

    // called when drag has ended -- with either "drop" or "cancel"
    virtual void OnEndDrag(int iFrom, bool bDrop);

    // Dragged from iFrom to iTo, not dropped yet
    virtual bool OnDrag(int iFrom, int iDrop, bool bCopy);

    // Dragged and droped from iFrom to iTo
    virtual bool OnDrop(int iFrom, int iDrop, bool bCopy);

    // Starting edit; returns true if allowed to edit
    virtual bool OnStartEdit(int index, CString& item);

    // Ending edit; returns true if allowed to change item name
    virtual bool OnEndEdit(int index, CString& item);

    // Finalize edit changes
    virtual void CommitEditChanges(int index, LPCTSTR item);

    // called on double click
    virtual void OnDoubleClick(int index, UINT nFlags, const CPoint& point);

    // called on right-click
    virtual void OnRightClick(int index, UINT nFlags, const CPoint& point);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CLstOwner)
};
}  // namespace AFLibGui
