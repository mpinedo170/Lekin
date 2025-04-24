#include "StdAfx.h"

#include "AppAsm.h"

#include "Dlg3Button.h"
#include "DlgAddJob.h"
#include "DlgAddMch.h"
#include "DlgAddWkc.h"
#include "DlgConfirm.h"
#include "DlgRunCon.h"
#include "DlgRunWin.h"
#include "DocSmart.h"
#include "DocTmpSmart.h"
#include "FrmMain.h"
#include "FrmSmart.h"
#include "ViewGantt.h"
#include "ViewObj.h"

#include "Job.h"
#include "Misc.h"
#include "Plugin.h"
#include "Schedule.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// window

void CAppAsm::OpenMainFrame()
{
    if (m_pMainWnd != NULL) return;

    m_pMainWnd = new CFrmMain;
    GetMainFrame()->LoadFrame(IDR_MAINFRAME);
    GetMainFrame()->ShowWindow(SW_HIDE);
}

bool CAppAsm::PrivateOpenConsole()
{
    if (m_hWndCon) return true;

    if (!AllocConsole()) return false;
    CString conTitle;
    conTitle.LoadString(IDS_CON_TITLE);
    CString conTitle2 = conTitle + IntToStr(int(GetMainFrame()->m_hWnd));
    if (!SetConsoleTitle(conTitle2)) return false;

    COORD sz = {80, 24};
    if (!SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT))
        return false;
    if (!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT))
        return false;

    for (int i = 0; m_hWndCon == NULL; ++i)
    {
        if (i == 10) return false;
        Sleep(1);
        m_hWndCon = ::FindWindow(NULL, conTitle2);
    }

    ::ShowWindow(m_hWndCon, SW_HIDE);
#ifdef _DEBUG
    ::ShowWindow(m_hWndCon, SW_MINIMIZE);
#endif
    SetConsoleTitle(conTitle);
    return true;
}

bool CAppAsm::OpenConsole()
{
    if (m_hWndCon) return true;
    FreeConsole();
    GetMainFrame()->SetWindowPos(&CWnd::wndTopMost, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE);
    bool bResult = PrivateOpenConsole();

    GetMainFrame()->SetForegroundWindow();
    GetMainFrame()->SetWindowPos(&CWnd::wndNoTopMost, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE);
    return bResult;
}

bool CAppAsm::CloseConsole()
{
    if (!FreeConsole())
    {
        m_hWndCon = NULL;
        return true;
    }

    if (!m_hWndCon) return true;
    ::ShowWindow(m_hWndCon, SW_MINIMIZE);

    Sleep(10);
    if (IsWindow(m_hWndCon))
    {
        ::ShowWindow(m_hWndCon, SW_HIDE);
        ::PostMessage(m_hWndCon, WM_CLOSE, 0, 0);
    }

    m_hWndCon = NULL;
    return true;
}

int CAppAsm::FillWindows(bool bIconic, bool arrBOk[])
{
    int total = 0;

    for (int i = 0; i < tvwN; ++i)
    {
        arrBOk[i] = false;
        CFrmSmart* pFrame = GetFrm(i);
        if (!pFrame->IsWindowVisible()) continue;

        bool bIc = pFrame->IsIconic() != 0;
        if (bIc != bIconic) continue;
        arrBOk[i] = true;
        ++total;
        if (pFrame->IsZoomed()) pFrame->ShowWindow(SW_RESTORE);
    }
    return total;
}

void CAppAsm::TileHor(const CRect& rect, bool arrBOk[])
{
    CRect cliRect(rect);
    static int weightRow[tvwN] = {tvwMch, tvwGantt, tvwSeq, tvwLog, tvwJob, tvwObj};
    int windows[2];

    int total = 0;
    for (int i = 0; i < tvwN; ++i)
    {
        int id = weightRow[i];
        if (arrBOk[id])
        {
            windows[total] = id;
            if (++total == 2) break;
        }
    }

    switch (total)
    {
        case 2:
        {
            CRect rect2(cliRect);
            rect2.top += int(rect.Height() * 0.45);
            GetFrm(windows[1])->MoveWindow(rect2);
            cliRect.bottom = rect2.top;
        }
        case 1:
            GetFrm(windows[0])->MoveWindow(cliRect);
    }
}

CRect CAppAsm::ArrangeIcons()
{
    bool arrBOk[tvwN];
    CRect cliRect = GetMainFrame()->m_cliRect;
    CSize icSize(GetSystemMetrics(SM_CXMINIMIZED), GetSystemMetrics(SM_CYMINIMIZED));

    int total = FillWindows(true, arrBOk);
    if (total == 0) return cliRect;

    int cols = cliRect.Width() / icSize.cx;
    int rows = (total - 1) / cols + 1;

    cliRect.bottom -= rows * icSize.cy;
    CPoint pt(0, cliRect.bottom);

    for (int i = 0; i < tvwN; ++i)
    {
        if (!arrBOk[i]) continue;
        CFrmSmart* pFrame = GetFrm(i);
        CRect rect(pt, icSize);
        pFrame->MoveWindow(rect);

        pt.x += icSize.cx;
        if (pt.x + icSize.cx > cliRect.Width())
        {
            pt.x = 0;
            pt.y += icSize.cy;
        }
    }
    return cliRect;
}

void CAppAsm::ShowSomething()
{
    OnWinShow(ID_WIN_SHOW0 + min(arrSchedule.GetSize(), 2));
}

void CAppAsm::ActivateSomething()
{
    for (int iconic = 0; iconic < 2; ++iconic)
        for (int i = tvwN; --i >= 0;)
        {
            CDocTmpSmart* pTmp = theApp.GetTmp(i);
            CFrmSmart* pFrame = pTmp->GetFrm();
            if (pFrame->IsWindowVisible() && pFrame->IsIconic() == iconic)
            {
                pTmp->ActivateFrm();
                return;
            }
        }
}

bool CAppAsm::ZoomFit()
{
    return ((CViewGantt*)GetView(tvwGantt))->ZoomFit();
}

void CAppAsm::SavePosition()
{
    if (!m_bNormalRun) return;

    int state = 1;
    if (GetMainFrame()->IsIconic())
        state = 0;
    else if (GetMainFrame()->IsZoomed())
        state = 2;

    CRect& rect = GetMainFrame()->m_winRect;
    WriteUserInt(secSettings, keyX, rect.left);
    WriteUserInt(secSettings, keyY, rect.top);
    WriteUserInt(secSettings, keyCX, rect.Width());
    WriteUserInt(secSettings, keyCY, rect.Height());
    WriteUserInt(secSettings, keyState, state);
}

/////////////////////////////////////////////////////////////////////////////
// font manipulation

void CAppAsm::CreateAppFont(CFont& font, CDC* pDC, double coef, bool bBold, bool bItalic)
{
    CreateLogicalFont(font, pDC, m_faceFont, m_sizeFont * coef, bBold, bItalic);
}

void CAppAsm::CreateSysFont(CFont& font, CDC* pDC)
{
    CreateLogicalFont(
        font, pDC, pDC != NULL && pDC->IsPrinting() ? _T("Arial") : _T("MS Shell Dlg"), 8, false, false);
}

/////////////////////////////////////////////////////////////////////////////
// confirmation

bool CAppAsm::AskConfirmation(UINT promptID, int level, LPCTSTR prompt)
{
    int confirm = theApp.GetUserInt(secView, keyConfirm, 2);
    if (level > confirm) return true;

    CDlgConfirm dlg;
    if (prompt)
        dlg.m_prompt = prompt;
    else
        dlg.m_prompt.LoadString(promptID);
    dlg.m_bConfirm = false;
    dlg.m_level = level;

    DWORD oldContext = m_dwPromptContext;
    m_dwPromptContext = promptID + HID_BASE_PROMPT;

    UINT id = dlg.DoModal();
    if (dlg.m_bConfirm && level > 0) theApp.WriteUserInt(secView, keyConfirm, level - 1);
    m_dwPromptContext = oldContext;
    return id == IDYES;
}

bool CAppAsm::AskConfirmDel(UINT ids2)
{
    CString s(MAKEINTRESOURCE(ids2));
    CString msg;
    AfxFormatString1(msg, IDP_CNF_DEL, s);
    return AskConfirmation(IDP_CNF_DEL, 2, msg);
}

/////////////////////////////////////////////////////////////////////////////
// edit tree views

void CAppAsm::EditWkc(TWorkcenterArray& arrWkc)
{
    CDlgAddWkc dlg;

    for (int i = 0; i < arrWkc.GetSize(); ++i) dlg.AddWkc(*arrWkc[i]);

    if (dlg.DoModal() != IDOK) return;
    CString newName = dlg.WkcID();

    if (!newName.IsEmpty())
    {
        ASSERT(arrWkc.GetSize() == 1);
        TWorkcenter* pWkcOnly = arrWkc[0];

        if (newName != pWkcOnly->m_id)
        {
            if (equals(newName, pWkcOnly->m_id))
                pWkcOnly->ChangeName(newName, true);
            else if (!pWkcOnly->HasOpers())
                pWkcOnly->ChangeName(newName, false);
            else
            {
                CDlg3Button dlg(IDD_CHG_WKC_NAME);
                int res = dlg.DoModal();
                switch (res)
                {
                    case IDOK:
                        pWkcOnly->ChangeName(newName, true);
                        theApp.m_pTmpJob->Modify();
                        break;
                    case IDNO:
                        pWkcOnly->ChangeName(newName, false);
                        break;
                }
            }
        }
    }

    for (int i = 0; i < arrWkc.GetSize(); ++i) dlg.ReadWkc(*arrWkc[i]);

    TWorkcenter::SingleMachine();
    m_pTmpMch->Modify();
}

void CAppAsm::EditMch(TMachineArray& arrMch)
{
    CDlgAddMch dlg;
    if (arrMch.GetSize() == 0) return;

    for (int i = 0; i < arrMch.GetSize(); ++i) dlg.AddMch(*arrMch[i]);
    if (dlg.DoModal() != IDOK) return;

    for (int i = 0; i < arrMch.GetSize(); ++i) dlg.ReadMch(*arrMch[i]);

    m_pTmpMch->Modify();
    if (pSchActive && dlg.m_bIDChange) m_pTmpSeq->Modify();
}

void CAppAsm::EditJob(TJobArray& arrJob)
{
    TOperationArray arrOpOrphan;

    if (!InvalidateJobs(arrJob, arrOpOrphan))
    {
        DeleteEmptyJobs();
        arrOpOrphan.DestroyAll();
        return;
    }

    CDlgAddJob dlg;
    for (int i = 0; i < arrJob.GetSize(); ++i) dlg.AddJob(*arrJob[i]);

    if (dlg.DoModal() != IDOK)
    {
        DeleteEmptyJobs();
        arrOpOrphan.DestroyAll();
        return;
    }

    for (int i = 0; i < arrJob.GetSize(); ++i) dlg.ReadJob(*arrJob[i]);

    DeleteEmptyJobs();
    arrOpOrphan.DestroyAll();
    m_pTmpJob->Modify();
}

/////////////////////////////////////////////////////////////////////////////
// workspace save/load

void CAppAsm::FinishSaving(bool bSaveAs)
{
    CString newTitle = m_pTmpMch->GetTitle();

    for (int i = 1; i < 3; ++i)
    {
        CDocTmpSmart* pTmp = GetTmp(i);
        CDocSmart* pDoc = pTmp->GetDoc();

        CString title = pDoc->GetTitle();
        if (bSaveAs || title == pTmp->GetUntitled())
        {
            title = newTitle + pTmp->GetExt();
            if (GetFileAttributes(title) != -1)
                if (AfxMB2(MB_YESNO, IDP_OVERWRITE, title) != IDYES) return;
        }
        if (pDoc->IsEmpty())
            pDoc->SetTitle(title);
        else if (!pDoc->DoSave(title))
            return;
    }
}

bool CAppAsm::ClearWorkspace()
{
    for (int i = 3; --i >= 0;)
        if (!GetTmp(i)->OpenDocumentFile(NULL, false)) return false;
    return true;
}

void CAppAsm::LoadWorkspace(LPCTSTR lpszPathName, bool bSpecial)
{
    CString path;
    CString ext;

    if (lpszPathName == NULL)
    {
        path = m_pTmpMch->Prompt();
        if (path.IsEmpty()) return;
    }
    else
    {
        path = lpszPathName;
        if (bSpecial) ext = path.Right(4);
    }

    int dot = path.ReverseFind('.');
    if (dot == -1) return;
    path = path.Left(dot);

    for (int i = 0; i < 3; ++i)
    {
        CDocTmpSmart* pTmp = GetTmp(i);
        CString s = pTmp->GetExt();
        CString path2 = path + s;
        CFileStatus fs;
        if (!CFile::GetStatus(path2, fs) || fs.m_attribute == (BYTE)(-1)) break;
        CDocSmart* pDoc = (CDocSmart*)pTmp->OpenDocumentFile(path2, false);
        if (!pDoc || pDoc->IsEmpty()) break;
        if (s == ext) break;
    }

    if (!pSchActive) ShowSomething();
}

/////////////////////////////////////////////////////////////////////////////
// misc

void CAppAsm::ResetObjMask()
{
    ((CViewObj*)GetView(tvwObj))->SetMask(m_maskObj);
}

int CAppAsm::ScheduleMenu()
{
    CMenu* pMenu = GetMainFrame()->GetMenu();
    if (!pMenu) return -1;

    int i = FindMenuItem(pMenu->GetSafeHmenu(), _T("Schedule"));
    if (i == -1) return -1;

    if (!GotWkc() || arrJob.GetSize() <= 1)
    {
        pMenu->EnableMenuItem(i, MF_DISABLED | MF_GRAYED | MF_BYPOSITION);
        i = -1;
    }
    else
        pMenu->EnableMenuItem(i, MF_ENABLED | MF_BYPOSITION);

    GetMainFrame()->DrawMenuBar();
    return i;
}

void CAppAsm::SetSchActive(TSchedule* pSch, bool bModify)
{
    bool bWasEmpty = pSchActive == NULL;
    pSchActive = pSch;
    if (!pSchActive) return;
    pSchActive->Recompute(true);

    m_pTmpSeq->Modify(bModify);
    if (bWasEmpty)
    {
        if (!ZoomFit()) ShowSomething();
    }
}

bool CAppAsm::CheckSequences(bool bCancel)
{
    if (!pSchActive) return true;
    CDocSmart* pDoc = GetDoc(tvwSeq);

    if (pDoc)
    {
        int repl = IDNO;
        if (bCancel)
        {
            CDlg3Button dlg(IDD_CLEAR_SEQ);
            dlg.m_bNormal = pDoc->IsModified() != 0;
            repl = dlg.DoModal();
        }
        switch (repl)
        {
            case IDCANCEL:
                return false;
            case IDOK:
                if (!pDoc->DoFileSave()) return false;
            case IDNO:
                pDoc->SetModifiedFlag(false);
        }
    }
    m_pTmpSeq->OpenDocumentFile(NULL, false);
    return true;
}

void CAppAsm::RunExtAlgor(TPlugin& plug)
{
    if (!InvalidateAll()) return;
    if (!plug.PrepareProcess()) return;
    TObjective objData = plug.m_bShortData ? plug.m_Obj2 : objTime;

    BeginWaitCursor();
    CString sMch = GetTmpPath() + _T("_user.mch");
    CString sJob = GetTmpPath() + _T("_user.job");
    CString sSeq = GetTmpPath() + _T("_user.seq");
    DeleteFile(sSeq);

    TID::SetIOFlag(objData);
    bool bOk = m_pTmpMch->SaveFile(sMch) && m_pTmpJob->SaveFile(sJob);
    TID::SetIOFlag(objTime);
    if (!bOk) return;

    EndWaitCursor();

    CDlgRunWin* pDlg = plug.IsConsole() ? (CDlgRunWin*)new CDlgRunCon : new CDlgRunWin;
    pDlg->m_pPlug = &plug;
    int result = pDlg->DoModal();
    int sec = pDlg->m_sec;
    delete pDlg;
    pDlg = NULL;

    BeginWaitCursor();

    int schN = arrSchedule.GetSize();
    TID::SetIOFlag(objData);
    TSchedule::m_newTime = sec;
    m_pTmpSeq->AppendFile(sSeq);
    TSchedule::m_newTime = 0;
    TID::SetIOFlag(objTime);
    EndWaitCursor();

    if (arrSchedule.GetSize() <= schN)
    {
        if (result == IDOK) AfxMB(IDP_NO_SCH);
    }
    else
    {
        SetSchActive(arrSchedule[schN], true);
    }
}
