#include "StdAfx.h"

#include "AppAsm.h"
#include "FrmMain.h"

#include "DocSmart.h"
#include "DocTmpSmart.h"
#include "FrmSmart.h"

#include "Plugin.h"
#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CFrmMain

IMPLEMENT_DYNAMIC(CFrmMain, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CFrmMain, super)
ON_WM_CREATE()
ON_WM_INITMENU()
ON_WM_CLOSE()
ON_WM_GETMINMAXINFO()
ON_WM_DESTROY()
ON_WM_SIZE()
ON_WM_MOVE()
ON_WM_INITMENUPOPUP()

ON_COMMAND(ID_HELP_FINDER, OnHelpFinder)
ON_COMMAND(ID_HELP, OnHelp)
ON_COMMAND(ID_CONTEXT_HELP, OnContextHelp)
ON_COMMAND(ID_DEFAULT_HELP, OnHelpFinder)
END_MESSAGE_MAP()

static const UINT indicators[] = {
    ID_SEPARATOR,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CFrmMain construction/destruction

CFrmMain::CFrmMain()
{
    m_winRect.SetRectEmpty();
    m_cliRect.SetRectEmpty();
    m_oldRect.SetRectEmpty();
}

BOOL CFrmMain::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style &= ~FWS_ADDTOTITLE;
    return super::PreCreateWindow(cs);
}

int CFrmMain::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;

    VERIFY(m_wndToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS));
    VERIFY(m_wndToolBar.LoadToolBar(IDR_MAINFRAME));

    VERIFY(m_wndStatusBar.Create(this));
    VERIFY(m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)));

    InitBitmaps(m_wndToolBar);
    return 0;
}

void CFrmMain::InitBitmaps(CToolBar& wndToolBar)
{
    m_menuBitmapper.Add(wndToolBar);
}

void CFrmMain::InitPrintBitmap(CToolBar& wndToolBar)
{
    m_menuBitmapper.Add(ID_FILE_PRINT, wndToolBar, ID_FILE_PRINT_DIRECT, true);
}

/////////////////////////////////////////////////////////////////////////////
// CFrmMain message handlers

static void DelAll(CMenu* pMenu)
{
    int count = pMenu->GetMenuItemCount();
    for (int i = count; --i >= 0;)
    {
        TMenuItemInfo mii(pMenu->m_hMenu, i, TMenuItemInfo::tByPos);
        if (mii.IsSeparator()) break;
        pMenu->DeleteMenu(i, MF_BYPOSITION);
    }
}

HMENU CFrmMain::GetWindowMenuPopup(HMENU hMenuBar)
{  // suppressing warning trace message in CMDIFrameWnd::GetWindowMenuPopup()
    return NULL;
}

void CFrmMain::OnInitMenu(CMenu* pMenu)
{
    super::OnInitMenu(pMenu);

    // find "Schedule" menu

    if (pMenu != GetMenu()) return;
    int index = theApp.ScheduleMenu();
    if (index == -1) return;

    CMenu* pSub = pMenu->GetSubMenu(index);
    ASSERT(pSub);
    CMenu* pRule = pSub->GetSubMenu(0);
    DelAll(pRule);

    for (int i = ID_RULE0; i < ID_RULE0 + ruleN; ++i)
    {
        CString buffer(MAKEINTRESOURCE(i));
        TStringTokenizerStr sp(buffer, strNewLine);
        sp.ReadStr();

        CString s = sp.ReadStr();
        if (s.IsEmpty()) continue;

        int prefix = i - ID_RULE0 + 1;
        if (prefix == 10) prefix = 0;

        if (prefix < 10) s = _T("&") + IntToStr(prefix) + strSpace + s;
        pRule->AppendMenu(MF_STRING, i, s);
    }

    CMenu* pUser = pSub->GetSubMenu(1);
    DelAll(pUser);

    TPlugin plug;
    for (int i = 0;; ++i)
    {
        if (!plug.ReadFromReg(i)) break;
        plug.AddToMenu(*pUser, i + ID_PLUGIN0);
    }

    CMenu* pLog = pSub->GetSubMenu(2);
    DelAll(pLog);

    if (pSchActive == NULL)
    {
        pSub->EnableMenuItem(2, MF_DISABLED | MF_GRAYED | MF_BYPOSITION);
        return;
    }
    pSub->EnableMenuItem(2, MF_ENABLED | MF_BYPOSITION);

    EnumerateSch UINT flags = MF_STRING;
    if (pSchActive == pSch) flags |= MF_CHECKED;
    pLog->AppendMenu(flags, ID_LOG0 + si, pSch->m_id);
    EnumerateEnd
}

void CFrmMain::GetMessageString(UINT nID, CString& rMessage) const
{
    if (nID >= ID_FILE_MRU_FIRST && nID <= ID_FILE_MRU_LAST)
        nID = ID_FILE_MRU_FIRST;
    else if (nID >= ID_LOG0 && nID <= ID_LOG0 + 999)
        nID = ID_LOG0;
    else if (nID >= ID_PLUGIN0 && nID <= ID_PLUGIN0 + 999)
    {
        TPlugin plug;
        plug.ReadFromReg2(nID);
        rMessage = plug.m_Desc;
        if (!rMessage.IsEmpty()) return;
        nID = ID_PLUGIN0;
    }
    else if (nID >= ID_S_SORT_NAME && nID <= ID_S_SORT_OBJ5)
    {
        CString s = nID == ID_S_SORT_NAME ? _T("Name") : TSchedule::GetLabel(nID - ID_S_SORT_OBJ0);
        AfxFormatString1(rMessage, IDS_S_SORT, s);
        return;
    }

    super::GetMessageString(nID, rMessage);
}

void CFrmMain::OnClose()
{
    if (m_lpfnCloseProc != NULL && !(*m_lpfnCloseProc)(this)) return;

    for (int i = tvwSeq; i >= tvwMch; --i)
    {
        CDocTmpSmart* pTmp = theApp.GetTmp(i);
        CDocSmart* pDoc = pTmp->GetDoc();
        if (!pDoc->SaveModified()) return;
        pDoc->SetModifiedFlag(false);
        pTmp->OpenDocumentFile(NULL, false);
    }

    super::OnClose();
}

void CFrmMain::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    super::OnGetMinMaxInfo(lpMMI);
    lpMMI->ptMinTrackSize = CPoint(630, 470);
}

void CFrmMain::OnDestroy()
{
    theApp.SavePosition();
    theApp.CloseConsole();
    super::OnDestroy();
}

void CFrmMain::OnSize(UINT nType, int cx, int cy)
{
    super::OnSize(nType, cx, cy);

    switch (nType)
    {
        case SIZE_RESTORED:
            GetWindowRect(m_winRect);

        case SIZE_MAXIMIZED:
            ::GetClientRect(m_hWndMDIClient, m_cliRect);
            if (!m_oldRect.IsRectEmpty() && memcmp(&m_oldRect, &m_cliRect, sizeof(CRect)))
            {
                for (int i = 0; i < tvwN; ++i)
                {
                    CFrmSmart* pFrm = theApp.GetFrm(i);
                    if (!pFrm || pFrm->IsZoomed()) break;
                    if (!pFrm->IsWindowVisible() || pFrm->IsIconic()) continue;

                    CRect rect;
                    pFrm->GetWindowRect(rect);
                    for (int j = 0; j < 2; ++j) ::ScreenToClient(m_hWndMDIClient, LPPOINT(LPRECT(rect)) + j);

                    rect.left = rect.left * m_cliRect.Width() / m_oldRect.Width();
                    rect.right = rect.right * m_cliRect.Width() / m_oldRect.Width();
                    rect.top = rect.top * m_cliRect.Height() / m_oldRect.Height();
                    rect.bottom = rect.bottom * m_cliRect.Height() / m_oldRect.Height();
                    pFrm->MoveWindow(rect);
                }
                theApp.ArrangeIcons();
            }
            m_oldRect = m_cliRect;
    }
}

void CFrmMain::OnMove(int x, int y)
{
    super::OnMove(x, y);
    if (!IsIconic() && !IsZoomed()) GetWindowRect(m_winRect);
}

void CFrmMain::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    super::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
    if (!bSysMenu) m_menuBitmapper.UpdateMenu(pPopupMenu);
}
