#include "StdAfx.h"

#include "AFLibTreeDitem.h"

#include "AFLibDlgDitemDetails.h"
#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibMenuItemInfo.h"
#include "AFLibResource.h"
#include "AFLibStringTokenizerStr.h"
#include "AFLibStrings.h"
#include "AFLibSuperText.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibGui;
using namespace AFLibIO;
using namespace AFLibPrivate;

IMPLEMENT_DYNAMIC(CTreeDitem, super)

CTreeDitem::CTreeDitem(TType type)
{
    m_type = type;
    m_strPathDelim = strBSlash;

    switch (m_type)
    {
        case tBlue:
            m_bOwnerDraw = true;
            break;

        case tEditable:
            m_bOwnerDrop = true;
            SetDragType(dDrop, true);
            break;
    }
}

CTreeDitem::~CTreeDitem()
{
    m_arrDitem.DestroyAll();
}

void CTreeDitem::KillAllItems()
{
    CancelDrag();
    EndEditLabelNow(true);

    TCallOnSelect cz(this, false);
    DeleteAllItems();
    m_arrDitem.DestroyAll();
}

HTREEITEM CTreeDitem::CreateFolder(HTREEITEM hItemParent, LPCTSTR title)
{
    HTREEITEM hItem = InsertItem(title, hItemParent);
    SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
    return hItem;
}

HTREEITEM CTreeDitem::PrivateAddDitem(const TDitem& ditem)
{
    HTREEITEM hItem = NULL;
    TStringTokenizerStr sp(ditem.GetDir(), m_strPathDelim);

    while (!sp.IsEof())
    {
        CString s = sp.ReadStr();
        HTREEITEM hItem2 = FindChildItem(hItem, s);

        if (hItem2 == NULL)
            hItem = CreateFolder(hItem, s);
        else if (GetItemDitem(hItem2) != NULL)
            return NULL;
        else
            hItem = hItem2;
    }

    if (FindChildItem(hItem, ditem.GetTitle()) != NULL) return NULL;

    TDitem* pDitem = ditem.Clone();
    m_arrDitem.Add(pDitem);
    hItem = InsertItem(ditem.GetTitle(), hItem);
    SetItemData(hItem, DWORD_PTR(pDitem));
    return hItem;
}

HTREEITEM CTreeDitem::AddDitem(const TDitem& ditem)
{
    TCallOnSelect cz(this, false);
    HTREEITEM hItem = PrivateAddDitem(ditem);
    return hItem;
}

HTREEITEM CTreeDitem::FindItem(LPCTSTR path)
{
    HTREEITEM hItem = NULL;
    TStringTokenizerStr sp(path, m_strPathDelim);

    while (!sp.IsEof())
    {
        hItem = FindChildItem(hItem, sp.ReadStr());
        if (hItem == NULL) break;
    }

    return hItem;
}

void CTreeDitem::DeleteDir(HTREEITEM hItem)
{
    TDitem* pDitem = GetItemDitem(hItem);

    if (pDitem != NULL)
    {
        DeleteItem(hItem);
        m_arrDitem.DestroyExact(pDitem);
    }
    else
    {
        CArray<HTREEITEM, HTREEITEM> arrItem;
        for (HTREEITEM hItem2 = GetChildItem(hItem); hItem2 != NULL; hItem2 = GetNextSiblingItem(hItem2))
            arrItem.Add(hItem2);

        for (int i = 0; i < arrItem.GetSize(); ++i) DeleteDir(arrItem[i]);
        DeleteItem(hItem);
    }
}

bool CTreeDitem::DelCurrentWithPrompt(LPCTSTR prompt)
{
    HTREEITEM hItem = GetSelectedItem();
    if (hItem == NULL || IsItemReadOnly(hItem)) return false;

    if (ItemHasChildren(hItem))
    {
        CString prompt2 = IsEmptyStr(prompt) ? LocalAfxString(IDS_DITEM_DEL_DIR, GetItemText(hItem)) : prompt;
        if (AfxMB2(MB_YESNO, prompt2) != IDYES) return false;
    }

    DeleteDir(hItem);
    return true;
}

TDitem* CTreeDitem::GetItemDitem(HTREEITEM hItem)
{
    return reinterpret_cast<TDitem*>(GetItemData(hItem));
}

void CTreeDitem::ReadChildren(HTREEITEM hItemParent, TDitemArray& arrDitem, bool bCopy)
{
    TDitem* pDitem = GetItemDitem(hItemParent);

    if (pDitem != NULL)
    {
        if (bCopy) pDitem = pDitem->Clone();
        arrDitem.Add(pDitem);
        return;
    }

    for (HTREEITEM hItem = GetChildItem(hItemParent); hItem != NULL; hItem = GetNextSiblingItem(hItem))
        ReadChildren(hItem, arrDitem, bCopy);
}

bool CTreeDitem::PrivateProcessDragTarget(HTREEITEM hItemFrom, HTREEITEM& hItemDrop, bool bCopy)
{
    if (hItemDrop != NULL && GetItemDitem(hItemDrop) != NULL) hItemDrop = GetParentItem(hItemDrop);

    if (IsItemReadOnly(hItemDrop)) return false;

    if (hItemDrop == GetParentItem(hItemFrom) || IsItemReadOnly(hItemFrom)) return bCopy;

    for (HTREEITEM hItem = hItemDrop; hItem != NULL; hItem = GetParentItem(hItem))
        if (hItem == hItemFrom) return false;

    return true;
}

bool CTreeDitem::ProcessDragTarget(
    HTREEITEM hItemFrom, HTREEITEM& hItemDrop, bool bCopy, TSuperText* pSuperText)
{
    bool bOk = PrivateProcessDragTarget(hItemFrom, hItemDrop, bCopy);
    if (pSuperText == NULL) return bOk;

    CRichEditCtrl* pEditDesc = GetEditDesc();
    if (pEditDesc == NULL) return bOk;

    CreateDragPrompt(bOk, hItemFrom, hItemDrop, bCopy, *pSuperText);
    pSuperText->UploadClearRich(*pEditDesc);
    return bOk;
}

void CTreeDitem::SetItemSel(HTREEITEM hItem, int sel)
{
    if (sel == 0)
        m_mapSel.RemoveKey(hItem);
    else
        m_mapSel.SetAt(hItem, sel);
}

void CTreeDitem::SetChildrenSel(HTREEITEM hItem, int sel, TDitemArray& arrDitem)
{
    if (GetItemSel(hItem) == sel) return;
    SetItemSel(hItem, sel);

    TDitem* pDitem = GetItemDitem(hItem);

    if (pDitem != NULL)
    {
        arrDitem.Add(pDitem);
        return;
    }

    for (HTREEITEM hItem2 = GetChildItem(hItem); hItem2 != NULL; hItem2 = GetNextSiblingItem(hItem2))
        SetChildrenSel(hItem2, sel, arrDitem);
}

void CTreeDitem::SetParentsSel(HTREEITEM hItem, int sel)
{
    while (true)
    {
        hItem = GetParentItem(hItem);
        if (hItem == NULL) break;

        if (sel != 1)
        {
            for (HTREEITEM hItem2 = GetChildItem(hItem); hItem2 != NULL; hItem2 = GetNextSiblingItem(hItem2))
                if (GetItemSel(hItem2) != sel)
                {
                    sel = 1;
                    break;
                }
        }

        if (GetItemSel(hItem) == sel) break;
        SetItemSel(hItem, sel);
    }
}

void CTreeDitem::SetAllSel(HTREEITEM hItem, int sel, TDitemArray& arrDitem)
{
    SetChildrenSel(hItem, sel, arrDitem);
    SetParentsSel(hItem, sel);
}

void CTreeDitem::SetAllSel(HTREEITEM hItem, int sel)
{
    ASSERT(GetItemDitem(hItem) != NULL);
    TDitemArray arrDitem;
    SetAllSel(hItem, sel, arrDitem);
    ASSERT(arrDitem.GetSize() <= 1);
}

void CTreeDitem::GetTitleType(HTREEITEM hItem, CString& title, CString& type)
{
    const TDitem* pDitem = GetItemDitem(hItem);

    if (pDitem == NULL)
    {
        title = GetPath(hItem);
        type = LocalAfxString(IDS_FOLDER);
    }
    else
    {
        title = pDitem->GetTitle();
        type = pDitem->GetDitemType();
    }
}

void CTreeDitem::CreateDragPrompt(
    bool bOk, HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bCopy, TSuperText& superText)
{
    CString title;
    CString type;

    GetTitleType(hItemFrom, title, type);
    superText.Add(LocalAfxString(bCopy ? IDS_COPY : IDS_MOVE, type));
    superText.SetBold(true);
    superText.Add(title);
    superText.SetBold(false);
    superText.Add(strEol);

    if (bOk)
    {
        if (hItemDrop == NULL)
            superText.Add(LocalAfxString(IDS_TO_ROOT));
        else
        {
            superText.Add(LocalAfxString(IDS_TO));
            superText.SetBold(true);
            superText.Add(GetPath(hItemDrop));
            superText.SetBold(false);
        }
    }
}

void CTreeDitem::CreateDragPrompt(
    bool bOk, HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bCopy, CString& text)
{
    CString title;
    CString type;

    GetTitleType(hItemFrom, title, type);
    text += LocalAfxString(bCopy ? IDS_COPY : IDS_MOVE, type) + title + strEol;

    if (bOk)
    {
        if (hItemDrop == NULL)
            text += LocalAfxString(IDS_TO_ROOT);
        else
            text += LocalAfxString(IDS_TO) + GetPath(hItemDrop);
    }
}

void CTreeDitem::NormalizeTitle(CString& title)
{
    for (int i = 0; i < title.GetLength(); ++i)
    {
        TCHAR ch = title[i];
        if (!IsValidFileChar(ch)) title.SetAt(i, chrSpace);
    }
    title.Trim();
}

void CTreeDitem::AddNewFolder(LPCTSTR initTitle)
{
    HTREEITEM hItemParent = GetSelectedItem();

    if (IsItemReadOnly(hItemParent)) return;

    // for a folder, create new item under the selected one;
    // for a scan, create a "brother"
    if (hItemParent != NULL && GetItemDitem(hItemParent) != NULL) hItemParent = GetParentItem(hItemParent);

    CString title = initTitle;
    CreateUniqueTitle(hItemParent, title);
    HTREEITEM hItem = CreateFolder(hItemParent, title);
    SelectItem(hItem);
    EditLabel(hItem);
}

void CTreeDitem::SetReadOnlyDir(LPCTSTR dir, bool bReadOnly)
{
    CString dir2 = dir;
    dir2.MakeLower();

    if (!StringEndsWith(dir2, m_strPathDelim)) dir2 += m_strPathDelim;

    if (dir2.GetLength() <= m_strPathDelim.GetLength()) return;

    bool bFound = false;
    int index = BSearchStrings(dir2, m_arrReadOnly, bFound);

    if (bReadOnly)
    {
        if (!bFound) m_arrReadOnly.InsertAt(index, dir2);
    }
    else
    {
        if (bFound) m_arrReadOnly.RemoveAt(index);
    }

    m_bOwnerDraw = IsBlue() || !m_arrReadOnly.IsEmpty();
    if (m_hWnd != NULL) RedrawWindow();
}

bool CTreeDitem::PrivateIsItemReadOnly(HTREEITEM hItem, TStringBuffer& path)
{
    if (hItem == NULL) return false;

    HTREEITEM hItem2 = GetParentItem(hItem);
    if (PrivateIsItemReadOnly(hItem2, path)) return true;

    CString title = GetItemText(hItem);
    title.MakeLower();
    path += title;
    path += m_strPathDelim;

    bool bFound = false;
    BSearchStrings(path, m_arrReadOnly, bFound);
    return bFound;
}

bool CTreeDitem::IsItemReadOnly(HTREEITEM hItem)
{
    SAFE_STATIC_SB(path);
    return !m_arrReadOnly.IsEmpty() && PrivateIsItemReadOnly(hItem, path);
}

void CTreeDitem::DisplayDitemDesc(HTREEITEM hItem, TSuperText& superText)
{
    CRichEditCtrl* pEditDesc = GetEditDesc();
    if (pEditDesc == NULL) return;

    const TDitem* pDitem = NULL;

    if (hItem != NULL) pDitem = GetItemDitem(hItem);
    if (pDitem != NULL) pDitem->Describe(superText);
    superText.UploadClearRich(*pEditDesc);
}

CDlgDitemDetails* CTreeDitem::CreateDlgEdit(TDitem* pDitem)
{
    return NULL;
}

CDlgDitemDetails* CTreeDitem::CreateDlgNew(TDitem* pDitemSaple)
{
    return NULL;
}

CRichEditCtrl* CTreeDitem::GetEditDesc()
{
    return NULL;
}

BEGIN_MESSAGE_MAP(CTreeDitem, super)
ON_COMMAND(ID_DITEM_NEW, OnDitemNew)
ON_COMMAND(ID_DITEM_NEW_FOLDER, OnDitemNewFolder)
ON_COMMAND(ID_DITEM_EDIT, OnDitemEdit)
ON_COMMAND(ID_DITEM_RENAME, OnDitemRename)
ON_COMMAND(ID_DITEM_DELETE, OnDitemDelete)
END_MESSAGE_MAP()

BOOL CTreeDitem::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE && !IsReadOnly() &&
        GetEditControl() == NULL && !InDrag())
    {
        SendMessage(WM_COMMAND, ID_DITEM_DELETE);
        return true;
    }

    return super::PreTranslateMessage(pMsg);
}

void CTreeDitem::OnRightClick(HTREEITEM hItem, UINT, const CPoint& point)
{
    if (IsReadOnly() || IsItemReadOnly(hItem)) return;
    SelectItem(hItem);

    CMenu menuPopup;
    menuPopup.Attach(::LoadMenu(AFLibDLL.hModule, MAKEINTRESOURCE(IDM_POPUP)));

    CMenu* pMenu = menuPopup.GetSubMenu(0);
    pMenu->SetDefaultItem(ID_DITEM_EDIT);

    if (hItem == NULL || GetItemDitem(hItem) == NULL) pMenu->EnableMenuItem(ID_DITEM_EDIT, MFS_GRAYED);

    if (hItem == NULL)
    {
        pMenu->EnableMenuItem(ID_DITEM_RENAME, MFS_GRAYED);
        pMenu->EnableMenuItem(ID_DITEM_DELETE, MFS_GRAYED);
    }

    if (!m_arrDitem.IsEmpty())
    {
        TMenuItemInfo mii(*pMenu, ID_DITEM_NEW, TMenuItemInfo::tByID);
        mii.SetText(mii.GetText() + strSpace + m_arrDitem[0]->GetDitemType());
        mii.Apply(*pMenu, ID_DITEM_NEW, TMenuItemInfo::tByID);
    }

    RightClickMenu(this, *pMenu, point);
}

bool CTreeDitem::OnBeginDrag(HTREEITEM hItemFrom)
{
    TDitemArray arrDitem;
    ReadChildren(hItemFrom, arrDitem, false);
    return arrDitem.GetSize() > 0;
}

bool CTreeDitem::OnDrag(HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bSpecialDrop, bool bCopy)
{
    return ProcessDragTarget(hItemFrom, hItemDrop, bCopy);
}

bool CTreeDitem::OnDrop(HTREEITEM hItemFrom, HTREEITEM hItemDrop, bool bSpecialDrop, bool bCopy)
{
    bool bOk = ProcessDragTarget(hItemFrom, hItemDrop, bCopy);
    if (!bOk) return false;

    const TDitem* pDitem = GetItemDitem(hItemFrom);
    CString newPath = GetPath(hItemDrop);
    CString title = GetItemText(hItemFrom);

    if (pDitem == NULL)
    {
        TDitemArray arrDitem;
        ReadChildren(hItemFrom, arrDitem, true);

        if (arrDitem.IsEmpty()) return false;

        CString oldPath = GetPath(hItemFrom);
        if (!bCopy) DeleteDir(hItemFrom);

        CreateUniqueTitle(hItemDrop, title);
        arrDitem.ChangeDirs(oldPath, newPath + title + strBSlash);

        HTREEITEM hItem = CreateFolder(hItemDrop, title);
        for (int i = 0; i < arrDitem.GetSize(); ++i) AddDitem(*arrDitem[i]);

        arrDitem.DestroyAll();
        SelectItem(hItem);
    }
    else
    {
        CAutoPtr<TDitem> pDitem2(pDitem->Clone());
        pDitem2->SetDir(newPath);
        if (!bCopy) DeleteDir(hItemFrom);

        CreateUniqueTitle(hItemDrop, title);
        pDitem2->SetTitle(title);
        SelectItem(AddDitem(*pDitem2));
    }

    return true;
}

bool CTreeDitem::OnBeginEdit(HTREEITEM hItem)
{
    return !IsItemReadOnly(hItem);
}

bool CTreeDitem::OnEndEdit(HTREEITEM hItem, LPCTSTR label)
{
    CString title(label);

    NormalizeTitle(title);
    if (title.IsEmpty()) return false;

    // check for duplicates
    CStringArray arrTitle;
    FillBadNames(GetParentItem(hItem), arrTitle, hItem);

    bool bFound = false;
    BSearchStringsIC(title, arrTitle, bFound);
    if (bFound) return false;

    TDitem* pDitem = GetItemDitem(hItem);

    if (pDitem != NULL)
        pDitem->SetTitle(title);
    else
    {
        TDitemArray arrDitem;
        ReadChildren(hItem, arrDitem, false);
        CString path = GetPath(GetParentItem(hItem));
        arrDitem.ChangeDirs(path + GetItemText(hItem) + strBSlash, path + title + strBSlash);
    }

    SetItemText(hItem, title);
    return true;
}

COLORREF CTreeDitem::GetReadOnlyColor(COLORREF colorText, COLORREF colorBack)
{  // if the text is kinda red, make "read-only" green
    return GetRValue(colorText) > GetBValue(colorText) + GetGValue(colorText) + 20 ? RGB(0, 192, 0)
                                                                                   : RGB(255, 0, 0);
}

void CTreeDitem::OnGetColors(HTREEITEM hItem, UINT state, COLORREF& colorText, COLORREF& colorBack)
{
    if (IsItemReadOnly(hItem))
    {
        colorText = GetReadOnlyColor(colorText, colorBack);
        return;
    }

    if (!IsBlue()) return;

    switch (GetItemSel(hItem))
    {
        case 1:
            colorText = GetSysColor(COLOR_INACTIVECAPTION);
            break;
        case 2:
            colorText = GetSysColor(COLOR_ACTIVECAPTION);
            break;
    }
}

void CTreeDitem::OnDitemNew()
{
    HTREEITEM hItemParent = GetSelectedItem();
    if (IsItemReadOnly(hItemParent)) return;

    CString title = strNew;
    TDitem* pDitem = NULL;

    if (hItemParent != NULL) pDitem = GetItemDitem(hItemParent);

    if (pDitem != NULL)
    {
        hItemParent = GetParentItem(hItemParent);
        title = pDitem->GetTitle();
    }

    CAutoPtr<CDlgDitemDetails> pDlg(CreateDlgNew(pDitem));
    if (pDlg == NULL) return;
    ASSERT(pDlg->m_pDitem != NULL);

    CreateUniqueTitle(hItemParent, title);
    pDlg->m_pDitem->SetTitle(title);
    pDlg->m_pDitem->SetDir(GetPath(hItemParent));
    FillBadNames(hItemParent, pDlg->m_arrBadNames, NULL);
    if (pDlg->DoModal() != IDOK) return;

    SelectItem(AddDitem(*pDlg->m_pDitem));
}

void CTreeDitem::OnDitemNewFolder()
{
    AddNewFolder(strNew);
}

void CTreeDitem::OnDitemEdit()
{
    HTREEITEM hItem = GetSelectedItem();
    if (hItem == NULL || IsItemReadOnly(hItem)) return;

    TDitem* pDitem = GetItemDitem(hItem);
    if (pDitem == NULL) return;

    CAutoPtr<CDlgDitemDetails> pDlg(CreateDlgEdit(pDitem));
    if (pDlg == NULL) return;

    pDlg->m_pDitem = pDitem->Clone();
    FillBadNames(GetParentItem(hItem), pDlg->m_arrBadNames, hItem);
    if (pDlg->DoModal() != IDOK) return;

    m_arrDitem.DestroyExact(pDitem);
    pDitem = pDlg->m_pDitem;
    pDlg->m_pDitem = NULL;
    m_arrDitem.Add(pDitem);

    SetItemData(hItem, DWORD_PTR(pDitem));
    SetItemText(hItem, pDitem->GetTitle());
    OnSelect(hItem);
}

void CTreeDitem::OnDitemRename()
{
    HTREEITEM hItem = GetSelectedItem();
    if (hItem != NULL && !IsItemReadOnly(hItem)) EditLabel(hItem);
}

void CTreeDitem::OnDitemDelete()
{
    DelCurrentWithPrompt();
}
