#include "StdAfx.h"

#include "AppAsm.h"

#include "DlgAbout.h"
#include "DlgJob.h"
#include "DlgPerform.h"
#include "DlgTitle.h"
#include "DocGantt.h"
#include "DocJob.h"
#include "DocLog.h"
#include "DocMch.h"
#include "DocObj.h"
#include "DocSeq.h"
#include "DocTmpAlg.h"
#include "DocTmpSmart.h"
#include "FrmMain.h"
#include "FrmSmart.h"
#include "TreePlug.h"
#include "ViewGantt.h"
#include "ViewJob.h"
#include "ViewLog.h"
#include "ViewMch.h"
#include "ViewObj.h"
#include "ViewSeq.h"

#include "Job.h"
#include "Misc.h"
#include "Schedule.h"

namespace {
const CString keyExe = _T("Program");
const CString keyComp = _T("Company");
}  // namespace

extern const CString secVit = _T("Vital");
extern const CString secHeu = _T("Heuristics");
extern const CString secView = _T("View");
extern const CString secSettings = _T("Settings");

extern const CString keyLicense = _T("License");
extern const CString keyFace = _T("Face");
extern const CString keySize = _T("Size");
extern const CString keySortJob = _T("Job Sort");
extern const CString keySortSch = _T("Sch Sort");

extern const CString keyID = _T("ID");
extern const CString keyCount = _T("Count");
extern const CString keyConfirm = _T("Confirm");
extern const CString keyObj = _T("Objective");

extern const CString strFntTimes = _T("Times New Roman");
extern const CString strFntFancy = _T("Symbol");

extern const CString keyX = _T("X");
extern const CString keyY = _T("Y");
extern const CString keyCX = _T("CX");
extern const CString keyCY = _T("CY");
extern const CString keyState = _T("State");

CAppAsm theApp;
CString ExePath;
CString AlgPath;
CString Company;
CString License = _T("Demo");

/////////////////////////////////////////////////////////////////////////////
// CAppAsm initialization

CAppAsm::CAppAsm()
{
    m_pDlgJob = NULL;
    m_pDlgPerform = NULL;
    m_bNormalRun = false;
    m_pArrJobSorted = NULL;
    m_pArrSchSorted = NULL;
}

BOOL CAppAsm::InitInstance()
{
    SetRegistryKey(_T("Feldman"));

    m_pArrJobSorted = new TJobArray;
    m_pArrSchSorted = new TScheduleArray;

    ExePath = GetExePath();
    AlgPath = GetMachineString(secVit, keyAlg, ExePath);
    AppendSlash(AlgPath);

#ifndef _DEBUG
    {
        CString ExePath2 = GetMachineString(secVit, keyExe, strEmpty);
        AppendSlash(ExePath2);
        if (ExePath.CompareNoCase(ExePath2) != 0) ExePath.Empty();
    }
#endif

    Company = GetMachineString(secVit, keyComp, _T("A"));

    if (!CheckHack() || ExePath.IsEmpty() || AlgPath.IsEmpty() || Company.IsEmpty())
    {
        AfxMB(IDP_REINSTALL);
        return false;
    }

    AfxEnableControlContainer();
    LoadStdProfileSettings(0);

    m_pTmpMch =
        new CDocTmpSmart(IDR_MCH, RUNTIME_CLASS(CDocMch), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewMch));
    AddDocTemplate(m_pTmpMch);

    m_pTmpJob =
        new CDocTmpSmart(IDR_JOB, RUNTIME_CLASS(CDocJob), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewJob));
    AddDocTemplate(m_pTmpJob);

    m_pTmpSeq =
        new CDocTmpSmart(IDR_SEQ, RUNTIME_CLASS(CDocSeq), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewSeq));
    AddDocTemplate(m_pTmpSeq);

    m_pTmpGantt = new CDocTmpSmart(
        IDR_GANTT, RUNTIME_CLASS(CDocGantt), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewGantt));
    AddDocTemplate(m_pTmpGantt);

    m_pTmpObj =
        new CDocTmpSmart(IDR_OBJ, RUNTIME_CLASS(CDocObj), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewObj));
    AddDocTemplate(m_pTmpObj);

    m_pTmpLog =
        new CDocTmpSmart(IDR_LOG, RUNTIME_CLASS(CDocLog), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewLog));
    AddDocTemplate(m_pTmpLog);

    m_pTmpAlg = new CDocTmpAlg(
        IDR_ALG, RUNTIME_CLASS(CDocSmart), RUNTIME_CLASS(CFrmSmart), RUNTIME_CLASS(CViewSmart));
    AddDocTemplate(m_pTmpAlg);

    if (equals(m_lpCmdLine, _T("/Register")))
    {
        OpenMainFrame();
        try
        {
            RegisterLekin();
        }
        catch (CException* pExc)
        {
            UINT idsHelp;
            CString s = GetExceptionMsg(pExc, &idsHelp);
            pExc->Delete();
            s += strEol + AfxString(IDS_CONTACT_ME);
            AfxMessageBox(s, MB_OK, idsHelp);
        }
        return false;
    }

    CCommandLineInfo info;
    ParseCommandLine(info);

    switch (info.m_nShellCommand)
    {
        case CCommandLineInfo::FileNew:
        case CCommandLineInfo::FileOpen:
            m_bNormalRun = true;
            break;

        case CCommandLineInfo::AppUnregister:
            OpenMainFrame();
            ProcessShellCommand(info);
            UnregisterLekin();
            return false;
    }

    OpenMainFrame();
    m_faceFont = GetUserString(secView, keyFace, strFntTimes);
    m_sizeFont = GetUserInt(secView, keySize, 7);
    m_sortJob = min(int(GetUserInt(secView, keySortJob, 0)), 4);
    m_sortSch = min(int(GetUserInt(secView, keySortSch, 0)), int(objN));
    m_maskObj = GetUserInt(secView, keyObj, 2);

    for (int i = 3; i < tvwN; ++i) GetTmp(i)->OpenDocumentFile(NULL, false);

    for (int i = 0; i < 3; ++i) GetTmp(i)->OpenDocumentFile(NULL, false);
    ResetObjMask();

    for (int i = 0; i < tvwN; ++i)
    {
        CFrmSmart* pFrame = GetFrm(i);
        pFrame->ShowWindow(SW_SHOWNORMAL);
        pFrame->MoveWindow(0, 0, 400, 300);
        pFrame->ShowWindow(SW_HIDE);
    }

    if (m_bNormalRun)
    {
        CDlgTitle dlg;
        dlg.DoModal();
    }

    int szX = GetSystemMetrics(SM_CXSCREEN);
    int szY = GetSystemMetrics(SM_CYSCREEN);

    int x = min(GetUserInt(secSettings, keyX, 0), szX * 4 / 5);
    int y = min(GetUserInt(secSettings, keyY, 0), szY * 4 / 5);
    int cx = min(GetUserInt(secSettings, keyCX, szX * 2 / 3), szX);
    int cy = min(GetUserInt(secSettings, keyCY, szY * 2 / 3), szY);
    GetMainFrame()->MoveWindow(x, y, cx, cy);

    switch (GetUserInt(secSettings, keyState, 1))
    {
        case 0:
            m_nCmdShow = SW_MINIMIZE;
            break;
        case 2:
            m_nCmdShow = SW_MAXIMIZE;
            break;
        default:
            m_nCmdShow = SW_SHOWNORMAL;
            break;
    }

    GetMainFrame()->ShowWindow(m_nCmdShow);
    GetMainFrame()->SetForegroundWindow();

    CPrintDialog dlg(true);
    dlg.GetDefaults();
    m_hDevMode = dlg.m_pd.hDevMode;
    m_hDevNames = dlg.m_pd.hDevNames;

    if (m_bNormalRun) ShowAboutDlg(false);

    if (info.m_nShellCommand == CCommandLineInfo::FileNew)
        OnWktNew();
    else
    {
        if (!info.m_strFileName.IsEmpty()) LoadWorkspace(info.m_strFileName, true);
        if (!ProcessShellCommand(info)) return false;
    }

    m_bNormalRun = true;
    return true;
}

int CAppAsm::ExitInstance()
{
    delete m_pArrJobSorted;
    delete m_pArrSchSorted;
    return super::ExitInstance();
}

void CAppAsm::ShowAboutDlg(bool bAnyway)
{
    CDlgAbout dlg;
    dlg.m_bShowAnyway = bAnyway;
    dlg.DoModal();
}

void CAppAsm::RegisterLekin()
{
    for (int i = 0; i <= tvwN; ++i)
    {
        CString ext = GetTmp(i)->GetExt();
        if (ext.IsEmpty()) continue;
        DelRegTree(HKEY_CLASSES_ROOT, ext);
    }

    CreateUnsecureRegKey(HKEY_LOCAL_MACHINE, GetProfilePath() + strBSlash + keyAlg);
    CreateUnsecureDir(AlgPath);

    EnableShellOpen();
    RegisterShellFileTypes(true);
    WriteUserInt(secSettings, keyState, 2);

    static const int arrObj[] = {0xF6, 0x80, 0x04, 0x80, 0xF2, 0};

    static const int arrWkt[] = {0x3F, 0x15, 0x14, 0x0C, 0x14, 0};

    CTreePlug tree;
    tree.CreateDummy();
    tree.Init(false);

    for (int i = 0; arrObj[i] != NULL; ++i)
    {
        TPlugin plug;
        plug.Default();
        plug.m_Name.LoadString(IDS_ALG_NAME0 + i);
        plug.m_Desc.LoadString(IDS_ALG_DESC0 + i);
        plug.m_Exe.LoadString(IDS_ALG_EXE0 + i);
        plug.m_Obj = arrObj[i];
        plug.m_Wkt = arrWkt[i];
        if (i == 0) plug.m_bConsole = false;
        if (i > 0) plug.m_Advanced = 0;
        if (i == 2) plug.m_Files.LoadString(IDS_ALG_FILES2);
        if (i == 4) plug.m_bLimAsk = true;
        plug.AddToTree(tree);
    }
    tree.Save();
    WriteMachineString(secVit, keyLicense, License);
}

void CAppAsm::UnregisterLekin()
{
    UnregisterShellFileTypes();
}

/////////////////////////////////////////////////////////////////////////////
// access

CDocTmpSmart* CAppAsm::GetTmp(int ID)
{
    return (&m_pTmpMch)[ID];
}

CDocSmart* CAppAsm::GetDoc(int ID)
{
    return GetTmp(ID)->GetDoc();
}

CFrmSmart* CAppAsm::GetFrm(int ID)
{
    return GetTmp(ID)->GetFrm();
}

CViewSmart* CAppAsm::GetView(int ID)
{
    return GetTmp(ID)->GetView();
}

CFrmMain* CAppAsm::GetMainFrame()
{
    return (CFrmMain*)m_pMainWnd;
}

CFrmSmart* CAppAsm::GetActiveFrame()
{
    CFrameWnd* pActFrame = GetMainFrame();
    if (pActFrame != NULL)
    {
        pActFrame = pActFrame->GetActiveFrame();
        if (pActFrame != NULL && !pActFrame->IsWindowVisible()) pActFrame = NULL;
    }
    return (CFrmSmart*)pActFrame;
}

static int SortByName(const TID* style1, const TID* style2)
{
    return _tcsicmp(style1->m_id, style2->m_id);
}

static int Sort2(const TJob* pJob1, const TJob* pJob2)
{
    int a = pJob1->GetProcTime() - pJob2->GetProcTime();
    return a ? a : SortByName(pJob1, pJob2);
}

static int Sort3(const TJob* pJob1, const TJob* pJob2)
{
    int a = pJob1->m_due - pJob2->m_due;
    return a ? a : SortByName(pJob1, pJob2);
}

static int Sort4(const TJob* pJob1, const TJob* pJob2)
{
    int a = pJob2->m_weight - pJob1->m_weight;
    return a ? a : SortByName(pJob1, pJob2);
}

static int Sort5(const TJob* pJob1, const TJob* pJob2)
{
    int a = pJob1->m_release - pJob2->m_release;
    return a ? a : SortByName(pJob1, pJob2);
}

static int SchSortFlag;

static int SortSch(const TSchedule* sch1, const TSchedule* sch2)
{
    int a = sch1->m_arrObj[SchSortFlag] - sch2->m_arrObj[SchSortFlag];
    return a ? a : SortByName(sch1, sch2);
}

typedef int (*TJobSortFunc)(const TJob*, const TJob*);
typedef int (*TSchSortFunc)(const TSchedule*, const TSchedule*);

TJobArray& CAppAsm::GetJobList()
{
    if (m_pArrJobSorted->GetSize() == arrJob.GetSize()) return *m_pArrJobSorted;
    m_pArrJobSorted->Copy(arrJob);
    if (m_sortJob == 0) return *m_pArrJobSorted;

    TJobSortFunc funcs[] = {TJobSortFunc(SortByName), Sort2, Sort3, Sort4, Sort5};
    m_pArrJobSorted->SortByElement(funcs[m_sortJob]);
    return *m_pArrJobSorted;
}

TScheduleArray& CAppAsm::GetSchList()
{
    if (m_pArrSchSorted->GetSize() == arrSchedule.GetSize()) return *m_pArrSchSorted;
    m_pArrSchSorted->Copy(arrSchedule);

    switch (m_sortSch)
    {
        case 0:
            m_pArrSchSorted->SortByElement(TSchSortFunc(SortByName));
            break;

        default:
            SchSortFlag = m_sortSch - 1;
            m_pArrSchSorted->SortByElement(SortSch);
    }

    return *m_pArrSchSorted;
}

/////////////////////////////////////////////////////////////////////////////
// floaters

void CAppAsm::OpenDlgJob()
{
    if (pSchActive == NULL) return;

    if (m_pDlgJob == NULL)
        m_pDlgJob = new CDlgJob;
    else
        m_pDlgJob->SetFocus();
}

void CAppAsm::CloseDlgJob()
{
    if (m_pDlgJob == NULL) return;

    m_pDlgJob->DestroyWindow();
    delete m_pDlgJob;
    m_pDlgJob = NULL;
    UpdateFloaters();
}

void CAppAsm::CloseDlgPerform()
{
    if (m_pDlgPerform == NULL) return;

    m_pDlgPerform->DestroyWindow();
    delete m_pDlgPerform;
    m_pDlgPerform = NULL;
    UpdateFloaters();
}

void CAppAsm::UpdateFloaters(TOperation* pOp)
{
    if (m_pDlgJob != NULL) m_pDlgJob->FillData(pOp);
    if (m_pDlgPerform != NULL) m_pDlgPerform->FillData();
    m_pTmpGantt->UpdateView();
}

void CAppAsm::CloseFloaters()
{
    CloseDlgJob();
    CloseDlgPerform();
}
