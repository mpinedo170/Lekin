#pragma once

/////////////////////////////////////////////////////////////////////////////
// TMenuItemInfo
// Wrapper for MENUITEMINFO

namespace AFLibGui {
class AFLIB TMenuItemInfo
{
private:
    MENUITEMINFO m_info;  // underlying menu item info
    CString m_itemText;   // if menu item is a string, the the text of the item

public:
    // how to get or set menu item info -- by ID or position
    enum TType
    {
        tByPos,
        tByID
    };

    TMenuItemInfo();
    TMenuItemInfo(const TMenuItemInfo& mii);
    TMenuItemInfo(HMENU hMenu, int item, TType t);
    TMenuItemInfo& operator=(const TMenuItemInfo& mii);

    // apply m_info to an item of the menu
    void Apply(HMENU hMenu, int item, TType t) const;

    // insert the item into menu
    void Insert(HMENU hMenu, int item, TType t) const;

    // is menu item a string?
    bool IsText() const
    {
        return (m_info.fType & (MFT_SEPARATOR | MFT_BITMAP)) == 0;
    }

    // is menu item a separator?
    bool IsSeparator() const
    {
        return (m_info.fType & MFT_SEPARATOR) != 0;
    }

    // is menu item a bitmap?
    bool IsBitmap() const
    {
        return (m_info.fType & MFT_BITMAP) != 0;
    }

    // get item state
    UINT GetState() const
    {
        return m_info.fState;
    }

    // set item state
    void SetState(UINT state)
    {
        m_info.fState = state;
    }

    // get item ID
    UINT GetID() const
    {
        return m_info.wID;
    }

    // set item ID
    void SetID(UINT ID)
    {
        m_info.wID = ID;
    }

    // get submenu handle
    HMENU GetSubmenu() const
    {
        return m_info.hSubMenu;
    }

    // set submenu handle
    void SetSubmenu(HMENU hMenu)
    {
        m_info.hSubMenu = hMenu;
    }

    // get item data
    ULONG_PTR GetItemData() const
    {
        return m_info.dwItemData;
    }

    // set item data
    void SetItemData(ULONG_PTR itemData)
    {
        m_info.dwItemData = itemData;
    }

    // get check bitmap (checked or unchecked)
    HBITMAP GetChkBitmap(bool bChecked) const
    {
        return bChecked ? m_info.hbmpChecked : m_info.hbmpUnchecked;
    }

    // set check bitmap (checked or unchecked)
    void GetChkBitmap(bool bChecked, HBITMAP hbmp)
    {
        (bChecked ? m_info.hbmpChecked : m_info.hbmpUnchecked) = hbmp;
    }

    // get item text
    const CString& GetText() const
    {
        return m_itemText;
    }

    // set item text
    void SetText(LPCTSTR text);

    // get item bitmap
    HBITMAP GetBitmap() const
    {
        return m_info.hbmpItem;
    }

    // set item bitmap
    void SetBitmap(HBITMAP hbmpItem);

    // make the item be a separator
    void SetSeparator();
};
}  // namespace AFLibGui
