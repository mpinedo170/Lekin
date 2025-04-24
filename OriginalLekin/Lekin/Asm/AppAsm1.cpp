#include "StdAfx.h"

#include "AppAsm.h"

#include "Dlg3Button.h"
#include "DlgInit.h"
#include "DlgManual.h"
#include "DlgMenu.h"
#include "DlgObj2.h"
#include "DlgOptions.h"
#include "DlgParameter.h"
#include "DlgPerform.h"
#include "DlgPluginMan.h"
#include "DlgPrintMan.h"
#include "DlgStyle.h"
#include "DlgWorkspace.h"
#include "DocJob.h"
#include "DocMch.h"
#include "DocSeq.h"
#include "DocTmpSmart.h"
#include "FrmMain.h"
#include "FrmSmart.h"
#include "ViewGantt.h"
#include "ViewObj.h"

#include "Job.h"
#include "Misc.h"
#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CAppAsm

BEGIN_MESSAGE_MAP(CAppAsm, super)
ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
ON_COMMAND(ID_WKT_NEW, OnWktNew)
ON_COMMAND(ID_WKT_SAVE, OnWktSave)
ON_COMMAND(ID_WKT_SAVE_AS, OnWktSaveAs)
ON_COMMAND(ID_WKT_PRINT, OnWktPrint)
ON_COMMAND(ID_WKT_SETTINGS, OnWktSettings)
ON_COMMAND(ID_TOOL_OPTIONS, OnToolOptions)
ON_COMMAND(ID_WIN_CASCADE, OnWinCascade)
ON_COMMAND(ID_WIN_TILE, OnWinTile)
ON_COMMAND(ID_TOOL_PRF, OnToolPrf)
ON_COMMAND(ID_RULE_MANUAL, OnRuleManual)
ON_COMMAND(ID_TOOL_OBJ, OnToolObj)
ON_COMMAND(ID_LOG_ADD, OnLogAdd)
ON_COMMAND(ID_LOG_DEL, OnLogDel)
ON_COMMAND(ID_LOG_EDIT, OnLogEdit)
ON_UPDATE_COMMAND_UI(ID_TOOL_PRF, OnUpdateLogs)
ON_UPDATE_COMMAND_UI(ID_LOG_DEL, OnUpdateLogDel)
ON_UPDATE_COMMAND_UI(ID_WKT_SAVE, OnUpdateWktSave)
ON_COMMAND(ID_TOOL_PLUGIN, OnToolPlugin)
ON_COMMAND(ID_TOOL_GENERIC, OnToolGeneric)
ON_COMMAND(ID_TOOL_CONVERT, OnToolConvert)
ON_UPDATE_COMMAND_UI(ID_TOOL_OBJ, OnUpdateLogs)
ON_UPDATE_COMMAND_UI(ID_LOG_ADD, OnUpdateLogs)
ON_UPDATE_COMMAND_UI(ID_LOG_EDIT, OnUpdateLogs)
ON_UPDATE_COMMAND_UI(ID_WKT_SAVE_AS, OnUpdateWktSave)

ON_COMMAND(ID_HELP, OnHelp)
ON_COMMAND_RANGE(ID_WIN0, ID_WIN5, OnWin)
ON_COMMAND_RANGE(ID_WIN_SHOW0, ID_WIN_SHOW3, OnWinShow)

ON_COMMAND_RANGE(ID_RULE0, ID_RULE0 + ruleN - 1, OnRule)
ON_COMMAND_RANGE(ID_PLUGIN0, ID_PLUGIN0 + 999, OnPlugin)
ON_UPDATE_COMMAND_UI_RANGE(ID_PLUGIN0, ID_PLUGIN0 + 999, OnUpdatePlugin)
ON_COMMAND_RANGE(ID_LOG0, ID_LOG0 + 999, OnLogEntry)

ON_COMMAND_RANGE(ID_J_SORT_NAME, ID_J_SORT_RELEASE, OnJobSort)
ON_UPDATE_COMMAND_UI_RANGE(ID_J_SORT_NAME, ID_J_SORT_RELEASE, OnUpdateJobSort)
ON_COMMAND_RANGE(ID_S_SORT_NAME, ID_S_SORT_OBJ0 + objN - 1, OnSchSort)
ON_UPDATE_COMMAND_UI_RANGE(ID_S_SORT_NAME, ID_S_SORT_OBJ0 + objN - 1, OnUpdateSchSort)

ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnUpdateRecentFileMenu)
ON_COMMAND_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppAsm commands

void CAppAsm::OnAppAbout()
{
    ShowAboutDlg(true);
}

// workspace menu

void CAppAsm::OnWktNew()
{
    if (!ClearWorkspace()) return;

    CDlgMenu dlgMenu;
    if (dlgMenu.DoModal() == IDCANCEL) return;
    TWorkspace wkt = dlgMenu.m_wkt;

    switch (wkt)
    {
        case wktLoad:
            LoadWorkspace(NULL);
            return;
        case wktExit:
            GetMainWnd()->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
            return;
    }

    TWorkcenter::SetFlexible(WktIsFlexible(wkt));
    TJob::SetShop(WktGetShop(wkt));

    CDlgInit dlg;
    if (dlg.DoModal() != IDOK) return;

    int jobCount = dlg.m_jobCount;
    int mchCount = dlg.m_mchCount;

    if (wkt <= wktParallel)
    {
        TID style(tyWkc, _T("Uno"), colorWhite);
        style.RandomColor();
        arrWorkcenter.Add(new TWorkcenter(style, mchCount, 0, 'A'));
        m_pTmpMch->Modify();
    }
    else
    {
        GetFrm(tvwMch)->ShowWindow(SW_SHOWMAXIMIZED);
        while (true)
        {
            int k = arrWorkcenter.GetSize();
            if (k >= mchCount) break;
            GetView(tvwMch)->SendMessage(WM_COMMAND, ID_WKC_ADD, 0);
            if (arrWorkcenter.GetSize() == k) break;
        }
    }

    if (GotWkc())
    {
        GetFrm(tvwJob)->ShowWindow(SW_SHOWMAXIMIZED);
        while (true)
        {
            int k = arrJob.GetSize();
            if (k >= jobCount) break;
            GetView(tvwJob)->SendMessage(WM_COMMAND, ID_JOB_ADD, 0);
            if (arrJob.GetSize() == k) break;
        }
    }
    ShowSomething();
}

void CAppAsm::OnWktSettings()
{
    CDlgWorkspace dlg;
    dlg.DoModal();
}

void CAppAsm::OnWktSave()
{
    if (!GetDoc(tvwMch)->DoFileSave()) return;
    FinishSaving(false);
}

void CAppAsm::OnWktSaveAs()
{
    if (!GetDoc(tvwMch)->DoSave(NULL)) return;
    FinishSaving(true);
}

void CAppAsm::OnWktPrint()
{
    CDlgPrintMan dlg;
    if (dlg.DoModal() != IDOK) return;
    dlg.DoPrint();
}

// schedule menu

void CAppAsm::OnRule(UINT nID)
{
    if (!InvalidateAll()) return;

    CString buffer;
    buffer.LoadString(nID);
    TStringTokenizerStr sp(buffer, strNewLine);
    sp.ReadStr();

    CString s = sp.ReadStr();
    TRule rule = TRule(nID - ID_RULE0);

    if (rule == ruleATCS)
    {
        EnumerateWkc pWkc->ComputeAv();
        EnumerateEnd CDlgParameter dlg;
        if (dlg.DoModal() != IDOK) return;

        s += _T(" (") + DoubleToStr(dlg.m_k1) + strComma + DoubleToStr(dlg.m_k2) + _T(")");

        TSchedule::m_k1 = dlg.m_k1;
        TSchedule::m_k2 = dlg.m_k2;
    }

    TSchedule* pSch = new TSchedule;
    pSch->Create(rule);

    TSchedule* pSch2 = arrSchedule.FindKey(s);
    if (pSch2)
    {
        int res = IDCANCEL;
        if (*pSch2 != *pSch)
        {
            CDlg3Button dlg(IDD_SCH_CONFLICT);
            res = dlg.DoModal();
        }
        switch (res)
        {
            case IDOK:
                arrSchedule.AlterName(s);
                break;
            case IDNO:
                arrSchedule.DestroyExact(pSch2);
                pSch2 = NULL;
                break;
            case IDCANCEL:
                delete pSch2;
                SetSchActive(pSch2, false);
                return;
        }
    }
    pSch->m_id = s;
    arrSchedule.Add(pSch);
    SetSchActive(pSch, true);
}

void CAppAsm::OnRuleManual()
{
    if (!InvalidateAll()) return;
    TSchedule* pSch = NULL;

    if (pSchActive)
    {
        pSch = new TSchedule(*pSchActive);
        pSch->RandomColor();
    }
    else
    {
        pSch = new TSchedule();
        pSch->m_id = "Manual";
        pSch->CreateSequences();
    }

    arrSchedule.AlterName(pSch->m_id);

    CDlgManual dlg(pSch);
    if (dlg.DoModal() != IDOK)
    {
        delete pSch;
        return;
    }

    arrSchedule.Replace(pSch);
    SetSchActive(pSch, true);
}

// Tool menu

void CAppAsm::OnToolOptions()
{
    CDlgOptions dlg;
    dlg.m_faceFont = m_faceFont;
    dlg.m_sizeFont = m_sizeFont;
    dlg.m_confirmLevel = GetUserInt(secView, keyConfirm, 2);

    if (dlg.DoModal() != IDOK) return;

    WriteUserInt(secView, keyConfirm, dlg.m_confirmLevel);
    WriteUserString(secView, keyFace, m_faceFont = dlg.m_faceFont);
    WriteUserInt(secView, keySize, m_sizeFont = dlg.m_sizeFont);

    for (int i = 0; i < tvwN; ++i) GetFrm(i)->RecalcLayout();
}

void CAppAsm::OnToolPrf()
{
    if (pSchActive == NULL) return;

    if (m_pDlgPerform == NULL)
        theApp.m_pDlgPerform = new CDlgPerform;
    else
        m_pDlgPerform->SetFocus();
}

void CAppAsm::OnToolObj()
{
    CDlgObj2 dlg;
    dlg.m_Select = m_maskObj;
    if (dlg.DoModal() != IDOK) return;

    m_maskObj = dlg.m_Select;
    theApp.WriteUserInt(secView, keyObj, m_maskObj);
    ResetObjMask();
    m_pTmpObj->ActivateFrm();
}

void CAppAsm::OnToolPlugin()
{
    CDlgPluginMan dlg;
    dlg.DoModal();
}

void CAppAsm::OnToolGeneric()
{
    EnumerateWkc CString s;
    s.Format(_T("W%02d"), wi + 1);
    pWkc->m_idGen = s;

    if (TWorkcenter::IsFlexible())
    {
        for (int i = pWkc->GetMchCount(); --i >= 0;)
            pWkc->m_arrMch[i]->m_id.Format(_T("%s.%02d"), LPCTSTR(s), i + 1);
    }
    else
        pWkc->m_arrMch[0]->m_id = s;
    EnumerateEnd

        EnumerateJob pJob->m_id.Format(_T("J%02d"), ji + 1);
    for (int i = pJob->GetOpCount(); --i >= 0;)
    {
        TOperation* pOp = pJob->m_arrOp[i];
        pOp->m_idWkc = pOp->GetWkc()->m_idGen;
    }
    EnumerateEnd

        EnumerateWkc pWkc->m_id = pWkc->m_idGen;
    EnumerateEnd

        m_pArrJobSorted->RemoveAll();
    for (int i = tvwMch; i <= tvwSeq; ++i)
    {
        CDocSmart* pDoc = GetDoc(i);
        if (pDoc && !pDoc->IsEmpty()) pDoc->CDocSmart::SetModifiedFlag2(true);
    }
}

void CAppAsm::OnToolConvert()
{
    if (!ClearWorkspace()) return;
    TWorkcenter::SetFlexible(false);
    TJob::SetShop(JobShop);

    CString filter;
    filter.LoadString(IDS_TEXTFILTER);
    CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, filter);
    if (dlg.DoModal() == IDCANCEL) return;

    ifstream F(CStringA(dlg.GetPathName()));
    if (!F)
    {
        AfxMB(IDP_CANT_CREATE);
        return;
    }

    BeginWaitCursor();

    while (true)
    {
        char filename[_MAX_PATH];
        F >> filename;
        F >> filename;
        F.ignore(1024, chrNewLine);
        F.ignore(1024, chrNewLine);

        if (!F || !filename[0] || !_stricmp(filename, "END")) break;
        CString path = GetFullPath(filename);

        TCHAR drive[_MAX_DRIVE];
        TCHAR dir[_MAX_DIR];
        TCHAR fname[_MAX_FNAME];
        TCHAR ext[_MAX_EXT];

        _tsplitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        CString dir2(drive);
        dir2 += dir;
        CreateDirectory(dir2, NULL);
        dir2 += fname;

        int nJob, nMch;
        F >> nJob >> nMch;
        if (!F) break;

        for (int i = 0; i < nMch; ++i)
        {
            TWorkcenter* pWkc = new TWorkcenter();
            pWkc->m_id.Format(_T("W%02d"), i + 1);
            pWkc->RandomColor();
            pWkc->SingleMachine();
            arrWorkcenter.Add(pWkc);
        }

        for (int j = 0; j < nJob; ++j)
        {
            TJob* pJob = new TJob();
            pJob->m_id.Format(_T("J%02d"), j + 1);
            pJob->RandomColor();

            for (int i = 0; i < nMch; ++i)
            {
                int m, p;
                F >> m >> p;
                if (m < 0 || p <= 0) break;

                CString idWkc;
                idWkc.Format(_T("W%02d"), m + 1);
                TOperation* pOp = new TOperation(pJob, idWkc, p, _T('A'));
                pJob->m_arrOp.Add(pOp);
            }
            arrJob.Add(pJob);
        }

        bool bOk = m_pTmpMch->SaveFile(dir2 + _T(".mch")) && m_pTmpJob->SaveFile(dir2 + _T(".job"));

        arrWorkcenter.DestroyAll();
        arrJob.DestroyAll();

        if (!bOk)
        {
            AfxMB(IDP_CANT_CREATE);
            break;
        }
    }

    EndWaitCursor();
}

// Window menu

void CAppAsm::OnWin(UINT nID)
{
    GetTmp(nID - ID_WIN0)->ActivateFrm();
}

void CAppAsm::OnWinShow(UINT nID)
{
    static int arrWindow[4][tvwN] = {{tvwMch, tvwJob, -1}, {tvwGantt, tvwSeq, tvwJob, -1},
        {tvwGantt, tvwSeq, tvwLog, tvwJob, -1}, {tvwLog, tvwObj, -1}};

    bool arrBOk[tvwN] = {false};
    for (int i = 0; i < tvwN; ++i)
    {
        int id = arrWindow[nID - ID_WIN_SHOW0][i];
        if (id < 0) break;
        arrBOk[id] = true;
    }

    for (int i = 0; i < tvwN; ++i)
    {
        CFrmSmart* pFrame = GetFrm(i);
        int cmdShow = arrBOk[i] ? SW_SHOWNORMAL : pFrame->IsWindowVisible() ? SW_MINIMIZE : SW_HIDE;
        pFrame->ShowWindow(SW_HIDE);
        pFrame->ShowWindow(cmdShow);
    }
    OnWinTile();
}

void CAppAsm::OnWinTile()
{
    bool arrBOk[tvwN];
    CRect cliRect = ArrangeIcons();
    int total = FillWindows(false, arrBOk);
    int topID = -1;
    int h = int(cliRect.Height() * 0.35);

    switch (total)
    {
        case 0:
            return;

        case 2:
            if (arrBOk[tvwLog])
            {
                topID = tvwLog;
                break;
            }

        case 3:
        case 4:
            if (arrBOk[tvwGantt])
                topID = tvwGantt;
            else if (arrBOk[tvwLog] && total < 4)
                topID = tvwLog;
            break;

        case 5:
        case 6:
            if (arrBOk[tvwGantt])
            {
                topID = tvwGantt;
                h = int(cliRect.Height() * 0.25);
            }
            break;
    }

    if (topID != -1)
    {
        GetFrm(topID)->MoveWindow(cliRect.left, cliRect.top, cliRect.Width(), h);
        cliRect.top += h;
        arrBOk[topID] = false;
        --total;
    }

    switch (total)
    {
        case 0:
        case 1:
            TileHor(cliRect, arrBOk);
            break;

        case 5:
        {
            ASSERT(arrBOk[tvwObj]);
            int w = int(cliRect.Width() * 0.25);
            int x = cliRect.right - w;
            GetFrm(tvwObj)->MoveWindow(x, cliRect.top, w, cliRect.Height());
            cliRect.right = x;
            arrBOk[tvwObj] = false;
            --total;
        }

        default:
        {
            static int arrWidRow[tvwN] = {tvwObj, tvwMch, tvwSeq, tvwGantt, tvwJob, tvwLog};
            int arrWidth[tvwN];
            bool arrBHasWidth[tvwN];

            for (int i = 0; i < tvwN; ++i)
            {
                int id = arrWidRow[i];
                arrWidth[id] = i;
                arrBHasWidth[i] = arrBOk[id];
            }

            int count = 0;
            int threshold = tvwN;
            for (int i = 0; i < tvwN; ++i)
                if (arrBHasWidth[i])
                {
                    ++count;
                    if (count >= total / 2)
                    {
                        threshold = i;
                        break;
                    }
                }

            bool arrBOk1[tvwN], arrBOk2[tvwN];
            for (int i = 0; i < tvwN; ++i)
            {
                bool bNarrow = arrWidth[i] <= threshold;
                arrBOk1[i] = arrBOk[i] & bNarrow;
                arrBOk2[i] = arrBOk[i] & !bNarrow;
            }

            CRect rect1(cliRect), rect2(cliRect);
            rect1.right = rect2.left = int(cliRect.Width() * 0.4);
            TileHor(rect1, arrBOk1);
            TileHor(rect2, arrBOk2);
        }
    }
    ZoomFit();
    ActivateSomething();
}

void CAppAsm::OnWinCascade()
{
    bool arrBOk[tvwN];
    CRect cliRect = ArrangeIcons();
    int total = FillWindows(false, arrBOk);
    if (total == 0) return;

    int xy = GetSystemMetrics(SM_CYMINIMIZED);
    int txy = xy * (total - 1);
    CSize size = cliRect.Size() - CSize(txy, txy);
    CPoint pt(0, 0);
    CSize dpt(xy, xy);

    for (int i = 0; i < tvwN; ++i)
        if (arrBOk[i])
        {
            CFrmSmart* pFrame = GetFrm(i);
            pFrame->ShowWindow(SW_SHOWNORMAL);
            CRect rect(pt, size);
            pFrame->MoveWindow(rect);
            pt += dpt;
        }
    ZoomFit();
    ActivateSomething();
}

// Log menu

void CAppAsm::OnLogAdd()
{
    int index = arrSchedule.FindExact(pSchActive);
    if (index < 0) return;

    TSchedule* pSch = new TSchedule(*pSchActive);
    pSch->RandomColor();
    arrSchedule.AlterName(pSch->m_id);
    arrSchedule.InsertAt(index + 1, pSch);
    SetSchActive(pSch, true);
}

void CAppAsm::OnLogEdit()
{
    if (pSchActive && CDlgStyle::EditSch(pSchActive)) m_pTmpSeq->Modify();
}

void CAppAsm::OnLogDel()
{
    if (arrSchedule.GetSize() < 1) return;
    if (!AskConfirmDel(IDS_SCH)) return;
    int index = arrSchedule.FindExact(pSchActive);

    TScheduleArray& arrSchTemp = GetSchList();
    int index2 = arrSchTemp.FindExact(pSchActive) + 1;
    if (index2 >= arrSchTemp.GetSize()) index2 -= 2;

    TSchedule* pSch = arrSchTemp[index2];
    arrSchedule.DestroyAt(index);
    SetSchActive(pSch, true);
}

/////////////////////////////////////////////////////////////////////////////
// MRU file list implementation

CRecentFileList* CAppAsm::GetFileList(UINT nID, int& index)
{
    int id = nID - ID_FILE_MRU_FILE1;
    ASSERT(id >= 0);
    int type = id / MRU_COUNT;
    if (type >= 3) type = 0;

    index = id % MRU_COUNT;
    return GetTmp(type)->m_pRecent;
}

void CAppAsm::OnUpdateRecentFileMenu(CCmdUI* pCmdUI)
{
    int index = 0;
    CRecentFileList* pRecent = GetFileList(pCmdUI->m_nID, index);
    pRecent->UpdateMenu(pCmdUI);
}

void CAppAsm::OnOpenRecentFile(UINT nID)
{
    int index = 0;
    CRecentFileList* pRecent = GetFileList(nID, index);
    CString& lpszPathName = (*pRecent)[index];
    if (nID <= ID_FILE_MRU_FILE12)
        OpenDocumentFile(lpszPathName);
    else
    {
        if (!ClearWorkspace()) return;
        LoadWorkspace(lpszPathName);
    }
}

void CAppAsm::OnLogEntry(UINT nID)
{
    SetSchActive(arrSchedule[nID - ID_LOG0], false);
}

void CAppAsm::OnUpdateLogs(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(pSchActive != NULL);
}

void CAppAsm::OnPlugin(UINT nID)
{
    TPlugin plug;
    if (!plug.ReadFromReg2(nID)) return;
    RunExtAlgor(plug);
}

void CAppAsm::OnUpdatePlugin(CCmdUI* pCmdUI)
{
    TPlugin plug;
    pCmdUI->Enable(plug.ReadFromReg2(pCmdUI->m_nID));
}

void CAppAsm::OnJobSort(UINT nID)
{
    m_sortJob = nID - ID_J_SORT_NAME;
    theApp.WriteUserInt(secView, keySortJob, m_sortJob);
    m_pArrJobSorted->RemoveAll();
    m_pTmpJob->UpdateView();
}

void CAppAsm::OnSchSort(UINT nID)
{
    m_sortSch = nID - ID_S_SORT_NAME;
    theApp.WriteUserInt(secView, keySortSch, m_sortSch);
    m_pArrSchSorted->RemoveAll();
    m_pTmpObj->UpdateView();
    m_pTmpLog->UpdateView();
}

void CAppAsm::OnUpdateJobSort(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(arrJob.GetSize() > 1);
    pCmdUI->SetCheck(int(pCmdUI->m_nID - ID_J_SORT_NAME) == m_sortJob);
}

void CAppAsm::OnUpdateSchSort(CCmdUI* pCmdUI)
{
    int nID = pCmdUI->m_nID;
    pCmdUI->SetCheck(int(nID - ID_S_SORT_NAME) == m_sortSch);
    pCmdUI->Enable(arrSchedule.GetSize() > 1);
    if (nID >= ID_S_SORT_OBJ0) pCmdUI->SetText(_T("by ") + TSchedule::GetLabel(nID - ID_S_SORT_OBJ0));
}

void CAppAsm::OnUpdateLogDel(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(arrSchedule.GetSize() > 1);
}

void CAppAsm::OnUpdateWktSave(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GotWkc());
}
