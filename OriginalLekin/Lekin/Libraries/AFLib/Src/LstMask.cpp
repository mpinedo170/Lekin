#include "StdAfx.h"

#include "AFLibLstMask.h"

#include "AFLibMathGlobal.h"

using AFLibGui::CLstMask;

IMPLEMENT_DYNAMIC(CLstMask, super)

CLstMask::CLstMask()
{
    m_bAlwaysSelect = true;
}

int CLstMask::Search(int iFrom, int iTo, int d)
{
    iFrom = max(0, min(iFrom, GetCount() - 1));
    int sign = AFLibMath::sgn(iFrom - iTo);

    if (sign == 0) return IsEnabled(iFrom) ? iFrom : -1;

    while (true)
    {
        if (IsEnabled(iFrom)) return iFrom;
        iFrom += d;
        if ((iFrom - iTo) * sign < 0) return -1;
    }
}

void CLstMask::PrivateSetCurSel(int index)
{
    SetCurSel(index);
    GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), LPARAM(GetSafeHwnd()));
}

void CLstMask::FixSelection()
{
    if (GetCount() <= 0) return;
    int index = GetCurSel();

    if (!m_bAlwaysSelect)
    {
        if (index >= 0 && !IsEnabled(index)) SetCurSel(-1);
        return;
    }

    if (index == -1)
        index = Search(0, GetCount() - 1, 1);
    else
    {
        int newIndex = Search(index, GetCount() - 1, 1);
        if (newIndex == -1) newIndex = Search(index, 0, -1);
        index = newIndex;
    }

    SetCurSel(index);
}

BEGIN_MESSAGE_MAP(CLstMask, super)
ON_WM_LBUTTONDOWN()
ON_WM_KEYDOWN()
ON_MESSAGE(LB_SETCURSEL, OnLbSetCurSel)
END_MESSAGE_MAP()

void CLstMask::OnInit()
{
    super::OnInit();
    ASSERT(!IsMultipleSelection());
}

void CLstMask::Draw(CDC* pDC)
{
    if (!IsWindowEnabled() || !IsEnabled(m_draw.m_index)) pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
    super::Draw(pDC);
}

bool CLstMask::IsEnabled(int index)
{
    return true;
}

void CLstMask::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
    BOOL bOutside = true;
    int index = ItemFromPoint(point, bOutside);
    if (bOutside || index < 0 || index >= GetCount()) return;
    PrivateSetCurSel(index);
}

void CLstMask::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    int curSel = GetCurSel();
    int index = 0;

    CRect rect;
    GetClientRect(rect);
    int page = max(rect.Height() / GetItemHeight(0), 1);
    int pageMinusOne = max(page - 1, 1);

    if (GetCount() <= 0) return;
    if (curSel < 0)
        index = Search(0, GetCount() - 1, 1);
    else
        switch (nChar)
        {
            case VK_LEFT:
                if (!IsMultipleColumn() || curSel - page < 0) return;
                index = Search(curSel - page, 0, -page);
                break;

            case VK_RIGHT:
                if (!IsMultipleColumn() || curSel + page >= GetCount()) return;
                index = Search(curSel + page, GetCount() - 1, page);
                break;

            case VK_NEXT:
                index = Search(curSel + pageMinusOne, GetCount() - 1, 1);
                if (index == -1) index = Search(GetCount() - 1, curSel, -1);
                break;

            case VK_PRIOR:
                index = Search(curSel - pageMinusOne, 0, -1);
                if (index == -1) index = Search(0, curSel, 1);
                break;

            case VK_DOWN:
                index = Search(curSel + 1, GetCount() - 1, 1);
                break;

            case VK_UP:
                index = Search(curSel - 1, 0, -1);
                break;

            default:
                return;
        }

    if (index >= 0 && index != curSel) PrivateSetCurSel(index);
}

LRESULT CLstMask::OnLbSetCurSel(WPARAM wParam, LPARAM lParam)
{
    if (wParam == GetCurSel()) return 0;
    if (wParam >= 0 && !IsEnabled(wParam)) return 0;
    return Default();
}
