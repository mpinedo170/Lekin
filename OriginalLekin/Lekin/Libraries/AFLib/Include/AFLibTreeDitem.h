#pragma once

#include "AFLibDitemArray.h"
#include "AFLibStringBuffer.h"
#include "AFLibTestMaps.h"
#include "AFLibTreeOwner.h"

class CRichEditCtrl;

/////////////////////////////////////////////////////////////////////////////
// CTreeDitem tree control
// Tree of path-based items (ditems)

namespace AFLibGui {
class CDlgDitemDetails;

class AFLIB CTreeDitem : public CTreeOwner
{
private:
    typedef CTreeOwner super;
    DEFINE_COPY_AND_ASSIGN(CTreeDitem);

protected:
    // tree type
    enum TType
    {
        tSimple,
        tBlue,
        tEditable
    };

private:
    TType m_type;  // type of this tree

    // for blue trees: 1 - partially selected, 2 - fully selected
    AFLib::CIntMap<HTREEITEM, HTREEITEM> m_mapSel;

    // read-only directories
    CStringArray m_arrReadOnly;

    // set map value or remove item from the blue map
    void SetItemSel(HTREEITEM hItem, int sel);

    // change selection level for children
    void SetChildrenSel(HTREEITEM hItem, int sel, TDitemArray& arrDitem);

    // change selection level for parents
    void SetParentsSel(HTREEITEM hItem, int sel);

    // set sel for children and parents
    void SetAllSel(HTREEITEM hItem, int sel, TDitemArray& arrDitem);

    // set sel for children and parents; works only for individual scans
    void SetAllSel(HTREEITEM hItem, int sel);

    // helper for CreateDragPrompt()
    void GetTitleType(HTREEITEM hItem, CString& title, CString& type);

    // helper for AddDitem()
    HTREEITEM PrivateAddDitem(const TDitem& ditem);

    // helper for IsItemReadOnly()
    bool PrivateIsItemReadOnly(HTREEITEM hItem, AFLibIO::TStringBuffer& path);

    // helper for ProcessDragTarget()
    bool PrivateProcessDragTarget(HTREEITEM hItemFrom, HTREEITEM& hItemDrop, bool bCopy);

protected:
    TDitemArray m_arrDitem;  // list of ditems

    explicit CTreeDitem(TType type);

    // create a "folder" item
    virtual HTREEITEM CreateFolder(HTREEITEM hItemParent, LPCTSTR title);

    // replace invalid symbols with spaces, trim
    virtual void NormalizeTitle(CString& title);

    // create a dialog to edit a ditem
    virtual CDlgDitemDetails* CreateDlgEdit(TDitem* pDitem);

    // create a dialog to create a new ditem
    virtual CDlgDitemDetails* CreateDlgNew(TDitem* pDitemSample);

    // get the "Description" rich edit control (if used)
    virtual CRichEditCtrl* GetEditDesc();

    // add a ditem to the the tree
    HTREEITEM AddDitem(const TDitem& ditem);

    // delete the entire sub-tree
    void DeleteDir(HTREEITEM hItem);

    // delete currently selected item (prompt user if a directory)
    bool DelCurrentWithPrompt(LPCTSTR prompt = NULL);

    // for an editable tree, check if the drag is valid; display drag description if pSuperText != NULL
    bool ProcessDragTarget(
        HTREEITEM hItemFrom, HTREEITEM& hItemDrop, bool bCopy, TSuperText* pSuperText = NULL);

    // create a super-text prompt (what is moved where)
    void CreateDragPrompt(
        bool bOk, HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bCopy, TSuperText& superText);

    // create a text prompt (what is moved where)
    void CreateDragPrompt(bool bOk, HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bCopy, CString& text);

    // create a new folder according to the current selection
    void AddNewFolder(LPCTSTR initTitle);

    // call in OnSelect() to diplay the item description
    void DisplayDitemDesc(HTREEITEM hItem, TSuperText& superText);

    // is the item read-only?
    bool IsItemReadOnly(HTREEITEM hItem);

    // is this a "read-only" tree?
    bool IsReadOnly()
    {
        return m_type != tEditable;
    }

    // the tree allows selection / unselection of multiple items
    bool IsBlue()
    {
        return m_type == tBlue;
    }

public:
    ~CTreeDitem();

    // destroy all items
    void KillAllItems();

    // get ditem for a given hItem; null if it is a folder name
    TDitem* GetItemDitem(HTREEITEM hItem);

    // find item by path
    HTREEITEM FindItem(LPCTSTR path);

    // read children of all levels for given node
    void ReadChildren(HTREEITEM hItemParent, TDitemArray& arrDitem, bool bCopy);

    // set one of the top-level directories read-only (or not)
    void SetReadOnlyDir(LPCTSTR dir, bool bReadOnly = true);

    // get the text color for read-only items
    virtual COLORREF GetReadOnlyColor(COLORREF colorText, COLORREF colorBack);

    // select an item, update colors, fill array with selected ditems
    void PickItem(HTREEITEM hItem, TDitemArray& arrDitem)
    {
        ASSERT(IsBlue());
        SetAllSel(hItem, 2, arrDitem);
    }

    // pick a single ditem
    void PickDitem(HTREEITEM hItem)
    {
        ASSERT(IsBlue());
        SetAllSel(hItem, 2);
    }

    // unselect a single scan
    void UnpickDitem(HTREEITEM hItem)
    {
        ASSERT(IsBlue());
        SetAllSel(hItem, 0);
    }

    // 0 - unselected, 1 - partially selected, 2 - fully selected
    int GetItemSel(HTREEITEM hItem)
    {
        ASSERT(IsBlue());
        return m_mapSel.GetAt(hItem, 0);
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void OnRightClick(HTREEITEM hItem, UINT nFlags, const CPoint& point);
    virtual bool OnBeginDrag(HTREEITEM hItemFrom);
    virtual bool OnDrag(HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bSpecialDrop, bool bCopy);
    virtual bool OnDrop(HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bSpecialDrop, bool bCopy);
    virtual bool OnBeginEdit(HTREEITEM hItem);
    virtual bool OnEndEdit(HTREEITEM hItem, LPCTSTR label);
    virtual void OnGetColors(HTREEITEM hItem, UINT state, COLORREF& colorText, COLORREF& colorBack);

    afx_msg void OnDitemNew();
    afx_msg void OnDitemNewFolder();
    afx_msg void OnDitemEdit();
    afx_msg void OnDitemRename();
    afx_msg void OnDitemDelete();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CTreeDitem)
};
}  // namespace AFLibGui