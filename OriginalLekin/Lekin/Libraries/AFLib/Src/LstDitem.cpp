#include "StdAfx.h"

#include "AFLibLstDitem.h"

#include "AFLibDitemArray.h"

using namespace AFLibGui;

CLstDitem::CLstDitem()
{
    m_bDragEnabled = false;
    m_bFocusDetected = true;
}

int CLstDitem::AddDitem(TDitem* pDitem)
{
    return AddItemData(pDitem->GetTitle(), reinterpret_cast<int>(pDitem));
}

void CLstDitem::AddDitemArray(TDitemArray& arrDitem, bool bSelect)
{
    SetRedraw(false);
    if (bSelect) SetSel(-1, false);

    for (int i = 0; i < arrDitem.GetSize(); ++i)
    {
        int index = AddDitem(arrDitem[i]);
        if (bSelect) SetSel(index, true);
    }
    SetRedraw(true);
}

TDitem* CLstDitem::GetDitem(int index)
{
    return reinterpret_cast<TDitem*>(GetItemDataPtr(index));
}

BEGIN_MESSAGE_MAP(CLstDitem, super)
END_MESSAGE_MAP()
