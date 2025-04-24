#include "StdAfx.h"

#include "AFLibRichEditCtrlW.h"

#include "AFLibGuiGlobal.h"
#include "AFLibResource.h"
#include "Local.h"

using AFLibGui::CRichEditCtrlW;
using namespace AFLibPrivate;

CRichEditCtrlW::CRichEditCtrlW()
{
    m_type = tRegular;
}

bool CRichEditCtrlW::IsAllSelected()
{
    long nStartChar = 0;
    long nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    return nStartChar == 0 && nEndChar >= GetTextLengthEx(GTL_DEFAULT);
}

bool CRichEditCtrlW::IsAllUnselected()
{
    long nStartChar = 0;
    long nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    return nStartChar == nEndChar;
}

void CRichEditCtrlW::SetGray()
{
    SetBackgroundColor(false, colorNormal());
}

void CRichEditCtrlW::DisablePopupMenu()
{
    m_type = tNone;
}

void CRichEditCtrlW::EnablePopupMenu(bool bEnablePar)
{
    m_type = bEnablePar ? tParagraph : tRegular;
}

bool CRichEditCtrlW::ModifyPopupMenu(CMenu* pMenu)
{
    return true;
}

BEGIN_MESSAGE_MAP(CRichEditCtrlW, super)
ON_WM_RBUTTONUP()
ON_COMMAND(ID_EDIT_BOTTOM, OnEditTop)
ON_COMMAND(ID_EDIT_BOTTOM, OnEditBottom)
ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
ON_COMMAND(ID_EDIT_SELECT_PAR, OnEditSelectPar)
ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
END_MESSAGE_MAP()

void CRichEditCtrlW::OnRButtonUp(UINT nFlags, CPoint point)
{
    super::OnRButtonUp(nFlags, point);
    if (m_type == tNone) return;

    CMenu menuPopup;
    menuPopup.Attach(::LoadMenu(AFLibDLL.hModule, MAKEINTRESOURCE(IDM_POPUP)));
    CMenu* pMenu = menuPopup.GetSubMenu(1);

    if (m_type == tParagraph)
    {
        if (IsAllSelected() || GetLineCount() < 3) pMenu->EnableMenuItem(ID_EDIT_SELECT_PAR, MFS_GRAYED);
    }
    else
    {
        pMenu->DeleteMenu(ID_EDIT_TOP, MF_BYCOMMAND);
        pMenu->DeleteMenu(ID_EDIT_BOTTOM, MF_BYCOMMAND);
        pMenu->DeleteMenu(ID_EDIT_SELECT_PAR, MF_BYCOMMAND);
    }

    if (IsAllSelected()) pMenu->EnableMenuItem(ID_EDIT_SELECT_ALL, MFS_GRAYED);

    if (IsAllUnselected()) pMenu->EnableMenuItem(ID_EDIT_COPY, MFS_GRAYED);

    if (!ModifyPopupMenu(pMenu)) return;
    RightClickMenu(this, *pMenu, point);
}

void CRichEditCtrlW::OnEditTop()
{
    SetSel(0, 0);
}

void CRichEditCtrlW::OnEditBottom()
{
    int length = GetTextLengthEx(GTL_DEFAULT);
    SetSel(length, length);
}

void CRichEditCtrlW::OnEditSelectAll()
{
    SetSel(0, -1);
}

void CRichEditCtrlW::OnEditSelectPar()
{
    long nStartChar = 0;
    long nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    int lineStart = LineFromChar(nStartChar);
    int lineEnd = LineFromChar(nEndChar);

    for (; lineStart > 0; --lineStart)
        if (LineLength(LineIndex(lineStart - 1)) <= 0) break;

    for (; lineEnd < GetLineCount(); ++lineEnd)
        if (LineLength(LineIndex(lineEnd)) <= 0) break;

    nStartChar = LineIndex(lineStart);
    nEndChar = lineEnd >= GetLineCount() ? GetTextLengthEx(GTL_DEFAULT) : LineIndex(lineEnd);
    SetSel(nStartChar, nEndChar);
}

void CRichEditCtrlW::OnEditCopy()
{
    if (!IsAllUnselected()) Copy();
}

/////////////////////////////////////////////////////////////////////////////
// Special functions to establish Unicode contents in Ascii environment

#ifndef _UNICODE
bool CRichEditCtrlW::TestUnicode()
{
    if (::IsWindowUnicode(m_hWnd)) return true;
    INT_PTR temp = ::SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, (INT_PTR)0);
    ::SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, temp);
    return false;
}

BOOL CRichEditCtrlW::SubclassWindow(HWND hWnd)
{
    if (!super::SubclassWindow(hWnd)) return false;
    TestUnicode();
    return true;
}

BOOL CRichEditCtrlW::SubclassDlgItem(UINT nID, CWnd* pWndParent)
{
    if (!super::SubclassDlgItem(nID, pWndParent)) return false;
    TestUnicode();
    return true;
}

BOOL CRichEditCtrlW::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    return CWnd::Create(_T("RICHEDIT20W"), NULL, dwStyle, rect, pParentWnd, nID);
}

LRESULT CRichEditCtrlW::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    if (!TestUnicode()) return super::DefWindowProc(nMsg, wParam, lParam);

    if (m_pfnSuper != NULL) return ::CallWindowProcW(m_pfnSuper, m_hWnd, nMsg, wParam, lParam);

    WNDPROC pfnWndProc;
    if ((pfnWndProc = *GetSuperWndProcAddr()) == NULL)
        return ::DefWindowProcW(m_hWnd, nMsg, wParam, lParam);
    else
        return ::CallWindowProcW(pfnWndProc, m_hWnd, nMsg, wParam, lParam);
}
#endif
