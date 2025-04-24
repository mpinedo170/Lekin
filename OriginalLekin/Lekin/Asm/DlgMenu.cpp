#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgMenu dialog

CDlgMenu::CDlgMenu() : super(CDlgMenu::IDD)
{
    m_wkt = wktSingle;
}

BEGIN_MESSAGE_MAP(CDlgMenu, super)
ON_WM_PAINT()
ON_COMMAND_RANGE(IDB_MM_SINGLE, IDB_MM_EXIT, OnClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMenu message handlers

BOOL CDlgMenu::OnInitDialog()
{
    for (int i = 0; i < 8; ++i) m_arrButton[i].SubclassDlgItem(IDB_MM_SINGLE + i, this);

    super::OnInitDialog();
    FillWorkspaces();
    return true;
}

void CDlgMenu::OnClick(UINT id)
{
    m_wkt = TWorkspace(id - IDB_MM_SINGLE);
    OnOK();
}

void CDlgMenu::OnPaint()
{
    CPaintDC dc(this);
    CFont font;
    CreateLogicalFont(font, &dc, strFntTimes, 20, true, false);
    CFont* pFontOld = dc.SelectObject(&font);
    dc.SetBkMode(TRANSPARENT);

    LPCTSTR s = _T("Main Menu");
    CRect rect;
    GetClientRect(rect);
    CSize sz = dc.GetTextExtent(s);
    int x = (rect.Width() - sz.cx) / 2;
    int y = sz.cy / 2;

    dc.SetTextColor(colorLight());
    dc.TextOut(x + 2, y + 2, s);

    dc.SetTextColor(colorText());
    dc.TextOut(x, y, s);
    dc.SelectObject(pFontOld);
}

BOOL CDlgMenu::PreTranslateMessage(MSG* pMsg)
{
    if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && pMsg->wParam == VK_RETURN)
        pMsg->wParam = VK_SPACE;
    return super::PreTranslateMessage(pMsg);
}
