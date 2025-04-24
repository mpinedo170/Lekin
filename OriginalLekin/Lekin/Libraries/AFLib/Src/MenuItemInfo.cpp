#include "StdAfx.h"

#include "AFLibMenuItemInfo.h"

#include "AFLibGlobal.h"
#include "Local.h"

using namespace AFLib;
using AFLibGui::TMenuItemInfo;
using AFLibPrivate::LocalAfxString;

TMenuItemInfo::TMenuItemInfo()
{
    memset(&m_info, 0, sizeof(MENUITEMINFO));
    m_info.cbSize = sizeof(MENUITEMINFO);

    m_info.fMask = MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STATE | MIIM_ID | MIIM_SUBMENU;
    m_info.fType = MFT_SEPARATOR;
}

TMenuItemInfo::TMenuItemInfo(const TMenuItemInfo& mii)
{
    *this = mii;
}

TMenuItemInfo::TMenuItemInfo(HMENU hMenu, int item, TType t)
{
    bool bByPos = t == tByPos;

    memset(&m_info, 0, sizeof(MENUITEMINFO));
    m_info.cbSize = sizeof(MENUITEMINFO);
    m_info.fMask = MIIM_FTYPE;

    if (!GetMenuItemInfo(hMenu, item, bByPos, &m_info))
        ThrowMessage(LocalAfxString(IDS_NO_SUCH_MENU, IntToStr(item)));

    int mask = MIIM_CHECKMARKS | MIIM_STATE | MIIM_ID | MIIM_SUBMENU;

    if (IsText())
    {
        m_info.fMask = MIIM_STRING;
        GetMenuItemInfo(hMenu, item, bByPos, &m_info);
        m_itemText = CString(chrSpace, m_info.cch);
        ++m_info.cch;
        m_info.dwTypeData = const_cast<LPTSTR>(LPCTSTR(m_itemText));
        mask |= MIIM_STRING;
    }
    else if (IsBitmap())
        mask |= MIIM_BITMAP;

    m_info.fMask = mask;
    GetMenuItemInfo(hMenu, item, bByPos, &m_info);

    m_info.fMask |= MIIM_FTYPE;
}

TMenuItemInfo& TMenuItemInfo::operator=(const TMenuItemInfo& mii)
{
    m_info = mii.m_info;
    if (!mii.IsText()) return *this;

    m_itemText = mii.m_itemText;
    m_info.dwTypeData = const_cast<LPTSTR>(LPCTSTR(m_itemText));
    return *this;
}

void TMenuItemInfo::Apply(HMENU hMenu, int item, TType t) const
{
    SetMenuItemInfo(hMenu, item, t == tByPos, &m_info);
}

void TMenuItemInfo::Insert(HMENU hMenu, int item, TType t) const
{
    InsertMenuItem(hMenu, item, t == tByPos, &m_info);
}

void TMenuItemInfo::SetText(LPCTSTR text)
{
    m_info.fMask &= ~MIIM_BITMAP;
    m_info.fMask |= MIIM_STRING;

    m_info.fType &= ~(MFT_BITMAP | MFT_SEPARATOR);
    m_info.fType |= MFT_STRING;

    m_itemText = text;
    m_info.dwTypeData = const_cast<LPTSTR>(LPCTSTR(m_itemText));
    m_info.cch = m_itemText.GetLength() + 1;
    m_info.hbmpItem = NULL;
}

void TMenuItemInfo::SetBitmap(HBITMAP hbmpItem)
{
    m_info.fMask &= ~MIIM_STRING;
    m_info.fMask |= MIIM_BITMAP;

    m_info.fType &= ~(MFT_STRING | MFT_SEPARATOR);
    m_info.fType |= MFT_BITMAP;

    m_itemText.Empty();
    m_info.dwTypeData = NULL;
    m_info.cch = 0;
    m_info.hbmpItem = hbmpItem;
}

void TMenuItemInfo::SetSeparator()
{
    m_info.fMask &= ~(MIIM_STRING | MIIM_BITMAP);

    m_info.fType &= ~(MFT_STRING | MFT_BITMAP);
    m_info.fType |= MFT_SEPARATOR;

    m_itemText.Empty();
    m_info.dwTypeData = NULL;
    m_info.cch = 0;
    m_info.hbmpItem = NULL;
}
