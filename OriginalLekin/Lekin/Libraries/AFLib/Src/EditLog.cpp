#include "StdAfx.h"

#include "AFLibEditLog.h"

#include "AFLibGuiGlobal.h"
#include "AFLibResource.h"
#include "AFLibStringTokenizerText.h"
#include "AFLibStrings.h"
#include "Local.h"

using AFLibGui::CEditLog;
using namespace AFLib;
using namespace AFLibPrivate;

IMPLEMENT_DYNAMIC(CEditLog, super)

CEditLog::CEditLog()
{
    ClearSavedSel();
    m_bInSetFocus = false;
}

void CEditLog::Flush(int startPos)
{
    if (startPos >= m_logMsg.GetLength()) return;

    int nStartChar = 0;
    int nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    if (nStartChar < startPos)
    {
        int x = GetScrollPos(SB_HORZ);
        int y = GetScrollPos(SB_VERT);

        SetRedraw(false);
        SetWindowText(m_logMsg);
        SetSel(nStartChar, nEndChar, true);
        SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, y));
        SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, x));
        SetRedraw(true);
    }
    else
    {
        SetWindowText(m_logMsg);
        SetSel(m_logMsg.GetLength(), m_logMsg.GetLength());
    }
}

void CEditLog::ClearLog()
{
    m_logMsg.Clear();
    if (m_hWnd != NULL) SetWindowText(strEmpty);
}

void CEditLog::AppendLog(LPCTSTR msg)
{
    int len = m_logMsg.GetLength();
    AFLibIO::TStringTokenizerText sp(msg);

    while (!sp.IsEof())
    {
        CString s = sp.ReadStr();
        m_logMsg += s;
        if (!sp.GetLastSeparator().IsEmpty()) m_logMsg += strEol;
    }

    if (m_hWnd != NULL) Flush(len);
}

void CEditLog::OnInit()
{
    ASSERT((GetStyle() & ES_MULTILINE) != 0);
    Flush(0);
}

BEGIN_MESSAGE_MAP(CEditLog, super)
ON_WM_CREATE()
ON_MESSAGE(EM_SETSEL, OnEmSetSel)
ON_WM_SETFOCUS()
ON_WM_RBUTTONUP()
ON_COMMAND(ID_EDIT_TOP, OnEditTop)
ON_COMMAND(ID_EDIT_BOTTOM, OnEditBottom)
ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
END_MESSAGE_MAP()

void CEditLog::PreSubclassWindow()
{
    super::PreSubclassWindow();
    OnInit();
}

int CEditLog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;
    OnInit();
    return 0;
}

void CEditLog::OnSetFocus(CWnd* pOldWnd)
{
    if (m_savedStartChar >= 0)
    {
        SetSel(m_savedStartChar, m_savedEndChar, true);
        ClearSavedSel();
    }

    m_bInSetFocus = true;
    super::OnSetFocus(pOldWnd);
    m_bInSetFocus = false;
}

LRESULT CEditLog::OnEmSetSel(WPARAM wParam, LPARAM lParam)
{
    if (wParam == 0 && (lParam < 0 || lParam >= m_logMsg.GetLength()))
    {
        if (m_bInSetFocus) return 0;
        GetSel(m_savedStartChar, m_savedEndChar);
    }
    else
        ClearSavedSel();

    return Default();
}

void CEditLog::OnRButtonUp(UINT nFlags, CPoint point)
{
    SetFocus();

    int nStartChar = 0;
    int nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    CMenu menuPopup;
    menuPopup.Attach(::LoadMenu(AFLibDLL.hModule, MAKEINTRESOURCE(IDM_POPUP)));
    CMenu* pMenu = menuPopup.GetSubMenu(1);
    pMenu->DeleteMenu(ID_EDIT_SELECT_PAR, MF_BYCOMMAND);

    if (nEndChar <= 0) pMenu->EnableMenuItem(ID_EDIT_TOP, MFS_GRAYED);

    if (nStartChar >= m_logMsg.GetLength()) pMenu->EnableMenuItem(ID_EDIT_BOTTOM, MFS_GRAYED);

    if (nStartChar == 0 && nEndChar == m_logMsg.GetLength())
        pMenu->EnableMenuItem(ID_EDIT_SELECT_ALL, MFS_GRAYED);

    if (nStartChar == nEndChar) pMenu->EnableMenuItem(ID_EDIT_COPY, MFS_GRAYED);

    RightClickMenu(this, *pMenu, point);
}

void CEditLog::OnEditTop()
{
    SetSel(0, 0);
}

void CEditLog::OnEditBottom()
{
    SetSel(m_logMsg.GetLength(), m_logMsg.GetLength());
}

void CEditLog::OnEditSelectAll()
{
    SetSel(0, m_logMsg.GetLength());
}

void CEditLog::OnEditCopy()
{
    SendMessage(WM_COPY);
}
