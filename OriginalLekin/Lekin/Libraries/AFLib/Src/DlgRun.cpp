#include "StdAfx.h"

#include "AFLibDlgRun.h"

#include "AFLibExcBreak.h"
#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibThreadSafeStatic.h"
#include "DlgConfirm.h"
#include "Local.h"

using AFLibGui::CDlgRun;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibThread;
using namespace AFLibMath;
using namespace AFLibPrivate;

namespace {
const int PROGRESS_MAX = 1 << 30;
const CString strDashes = _T(" -- ");
}  // namespace

CDlgRun::CDlgRun(UINT nIDTemplate) : super(nIDTemplate)
{
    m_bRunParallel = true;
    m_bRethrow = false;
    m_bSuppressMB = false;
    m_bCancellable = true;
    m_slg = slgNone;
    m_status = staRunning;
    m_bTimed = false;
    m_changed = chgCancellable;
    m_clicks = m_current = -1;

    m_pWndProgress = new CProgressCtrl;
    m_pSync = new CCriticalSection;
    m_pThread = new CThread;
    m_pOdtStart = new COleDateTime;
    m_pOdtTimedStart = new COleDateTime;
    m_pOdtsBusy = new COleDateTimeSpan;
}

CDlgRun::~CDlgRun()
{
    delete m_pWndProgress;
    delete m_pSync;
    delete m_pThread;
    delete m_pOdtStart;
    delete m_pOdtTimedStart;
    delete m_pOdtsBusy;
}

UINT CDlgRun::Run2(LPVOID _pDlg)
{
    CDlgRun* pDlg = reinterpret_cast<CDlgRun*>(_pDlg);
    TStatus status = staFailed;
    CString errMsg;

    try
    {
        pDlg->RunAndCatch(errMsg, status);
    }
    catch (CExcMessage* pExc)
    {
        UINT idsHelp = 0;
        errMsg = GetExceptionMsg(pExc, &idsHelp);
        pExc->Delete();
    }
    catch (CException* pExc)
    {
        errMsg = GetExceptionMsg(pExc);
        pExc->Delete();
    }
    catch (...)
    {
        errMsg = LocalAfxString(IDS_UNCLASSIFIED);
    }

    Capitalize(errMsg);
    pDlg->Cleanup();
    pDlg->m_errMsg = errMsg;
    pDlg->m_status = status;
    return 0;
}

void CDlgRun::RunAndCatch(CString& errMsg, TStatus& status)
{
    try
    {
        Run();
        status = staSuccess;
    }
    catch (CExcBreak* pExc)
    {
        errMsg = LocalAfxString(m_bSuppressMB ? IDS_NO_MESSAGE : IDS_CANCELLED);
        pExc->Delete();
        status = staCancelled;
    }
}

int CDlgRun::GetFinishID()
{
    switch (m_status)
    {
        case staSuccess:
        case staSuccPlus:
            return IDOK;
        case staCancelled:
            return IDCANCEL;
    }
    return IDNO;
}

ATL::COleDateTime CDlgRun::GetTimeStart()
{
    return *m_pOdtStart;
}

COleDateTimeSpan CDlgRun::GetTimeBusy()
{
    return *m_pOdtsBusy;
}

void CDlgRun::TimedMsgBox(LPCTSTR s)
{
    CString msg = s + strNewLine + FormatTimeSpent(*m_pOdtsBusy);
    AfxMB2(MB_OK | MB_ICONINFORMATION, msg);
}

void CDlgRun::StartStep(LPCTSTR label, double clicks)
{
    CThread::Break();

    CSingleLock lock(m_pSync, true);
    *m_pOdtTimedStart = COleDateTime::GetCurrentTime();
    m_action.Empty();
    m_action += label;
    m_clicks = max(clicks, 1.);
    m_comment.Empty();
    m_current = 0;
    m_changed |= chgAction | chgProgress | chgComment;
}

void CDlgRun::Step(LPCTSTR label, double clicks)
{
    StartStep(label, clicks);
    m_bTimed = false;
}

double CDlgRun::TimedStep(LPCTSTR label, double seconds)
{
    StartStep(label, seconds);
    m_bTimed = true;
    COleDateTime odt = COleDateTime::GetCurrentTime();
    WaitStep();

    double sec = (COleDateTime::GetCurrentTime() - odt).GetTotalSeconds();

    if (sec >= 0)
    {
        TStringBuffer line;
        line += m_action;
        line.AppendCh(chrSpace, max(1, 40 - m_action.GetLength()));
        line += LocalAfxString(IDS_ESTIMATED, DoubleToStr(sec, 2), DoubleToStr(m_clicks, 2));
        Log(line);
    }

    CThread::Break();
    return sec;
}

void CDlgRun::ProcessSQLMsg(LPCTSTR msg)
{
    AppendLog(msg + strEol);
}

void CDlgRun::SetCurrent(double current)
{
    CThread::Break();
    CSingleLock lock(m_pSync, true);
    current = min(m_clicks, max(0., current));
    if (current == m_current) return;

    m_current = current;
    m_changed |= chgProgress;
}

void CDlgRun::AddClick(double clicks)
{
    SetCurrent(m_current + clicks);
}

void CDlgRun::SetTotalClicks(double clicks)
{
    CThread::Break();
    CSingleLock lock(m_pSync, true);

    if (clicks <= m_current) return;
    m_clicks = clicks;
    m_changed |= chgProgress;
}

bool CDlgRun::IsCancellable()
{
    return m_bCancellable;
}

bool CDlgRun::IsFinished()
{
    return m_status != staRunning;
}

void CDlgRun::SetComment(LPCTSTR comment)
{
    CThread::Break();
    CSingleLock lock(m_pSync, true);
    m_comment.Empty();
    m_comment += comment;
    m_changed |= chgComment;
}

void CDlgRun::SetDlgTitle(LPCTSTR dlgTitle)
{
    CThread::Break();
    CSingleLock lock(m_pSync, true);
    m_dlgTitle = dlgTitle;
    m_changed |= chgTitle;
}

void CDlgRun::ClearLog()
{
    CThread::Break();
    CSingleLock lock(m_pSync, true);

    m_changed &= ~chgLog;
    m_changed |= chgKillLog;
    m_strAddLog.Empty();
}

void CDlgRun::AppendLog(LPCTSTR msg)
{
    CThread::Break();
    CSingleLock lock(m_pSync, true);
    m_strAddLog += msg;
    m_changed |= chgLog;
}

void CDlgRun::SetCancellable(bool bCancellable)
{
    if (m_bCancellable == bCancellable) return;
    m_bCancellable = bCancellable;
    CThread::Break();
    m_changed |= chgCancellable;
}

bool CDlgRun::CopyFile(LPCTSTR source, LPCTSTR dest, bool bThrow)
{
    static const int BUF_SIZE = 1 << 16;
    int sz = int(FileSize(source) / BUF_SIZE) + 1;

    Step(LocalAfxString(IDS_COPYING, source), sz);
    LPVOID pBuffer = _alloca(BUF_SIZE);

    try
    {
        CFile Fin(source, CFile::modeRead | CFile::shareDenyWrite);
        CFile Fout(dest, CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite);

        while (true)
        {
            int count = Fin.Read(pBuffer, BUF_SIZE);
            if (count <= 0) break;
            Fout.Write(pBuffer, count);
            SetCurrent(int(Fin.GetPosition() / BUF_SIZE));
        }

        SetCurrent(sz);
        Fin.Close();
        Fout.Close();
        COleDateTime odt = FileGetCreationTime(source);

        if (odt != odtZero) FileSetCreationTime(dest, odt);

        odt = FileGetLastWriteTime(source);

        if (odt != odtZero) FileSetLastWriteTime(dest, odt);
        return true;
    }
    catch (CFileException* pExc)
    {
        if (bThrow || pExc->m_cause != CFileException::fileNotFound) throw;
        pExc->Delete();
        return false;
    }
}

COleDateTimeSpan CDlgRun::GetCurrentTimeBusy()
{
    return COleDateTime::GetCurrentTime() - *m_pOdtStart;
}

void CDlgRun::ComputeBusyTime()
{
    *m_pOdtsBusy = GetCurrentTimeBusy();
}

void CDlgRun::ThrowNoMessage()
{
    ThrowMessage(LocalAfxString(IDS_NO_MESSAGE));
}

void CDlgRun::Log(LPCTSTR line)
{}

void CDlgRun::WaitStep()
{}

void CDlgRun::TerminatePlus()
{}

void CDlgRun::Run()
{}

void CDlgRun::Cleanup()
{}

BEGIN_MESSAGE_MAP(CDlgRun, super)
ON_WM_TIMER()
ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

BOOL CDlgRun::OnInitDialog()
{
    bool bLogWndExists = GetDlgItem(IDC_LOG_LOG) != NULL;

    m_pWndProgress->SubclassDlgItem(IDC_RUN_PROGRESS, this);
    m_pWndProgress->SetRange32(0, PROGRESS_MAX);
    m_btnCancel.SubclassDlgItem(IDCANCEL, this);

    if (bLogWndExists) m_editLog.SubclassDlgItem(IDC_LOG_LOG, this);

    super::OnInitDialog();

    SetWindowText(m_dlgTitle + strDashes + LocalAfxString(IDS_WAIT));
    m_changed &= ~chgTitle;
    *m_pThread = CThread::Start(Run2, this);
    SetTimer(timerUpdate, 150, NULL);

    if (m_slg == slgNone)
    {
        if (bLogWndExists)
        {  // log exists, but is not needed
            CRect rectCli;
            GetClientRect(rectCli);

            CRect rectLog;
            m_editLog.GetWindowRect(rectLog);
            ScreenToClient(rectLog);

            CRect rect;
            GetWindowRect(rect);
            rect.bottom -= rectCli.bottom - rectLog.top;
            MoveWindow(rect);
        }

        m_btnCancel.SetFocus();
    }
    else
        m_editLog.SetFocus();

    StartAutoResize(NULL);
    return false;
}

void CDlgRun::PrivateUpdate()
{
    if (IsFinished())
    {
        KillTimer(timerUpdate);
        TerminatePlus();
        m_editLog.AppendLog(m_strAddLog);
        ComputeBusyTime();

        if (!IsEmptyStr(GetMsgLog()))
        {
            m_editLog.AppendLog(strEol + FormatTimeSpent(GetTimeBusy()));

            if (m_status == staSuccess && m_slg == slgKeep)
            {  // leave the dialog on the screen, let the user inspect the log
                m_bCancellable = false;
                m_btnCancel.SetWindowText(strOK);
                m_btnCancel.EnableWindow(true);
                SetDlgItemText(IDC_RUN_ACTION, LocalAfxString(IDS_DONE));
                SetDlgItemText(IDC_RUN_COMMENT, strEmpty);
                SetWindowText(m_dlgTitle + strDashes + LocalAfxString(IDS_DONE));
                m_pWndProgress->SetPos(PROGRESS_MAX);
                m_status = staSuccPlus;
                MessageBeep(MB_OK);
                return;
            }
        }

        EndDialog(GetFinishID());
        return;
    }

    CSingleLock lock(m_pSync, true);
    if (m_changed != 0)
    {
        if (m_changed & chgAction) SetDlgItemText(IDC_RUN_ACTION, m_action);

        if (m_changed & chgProgress) m_pWndProgress->SetPos(Rint(PROGRESS_MAX * (m_current / m_clicks)));

        if (m_changed & chgComment) SetDlgItemText(IDC_RUN_COMMENT, m_comment);

        if (m_changed & chgTitle) SetWindowText(m_dlgTitle + strDashes + LocalAfxString(IDS_WAIT));

        if (m_changed & chgKillLog) m_editLog.ClearLog();

        if (m_changed & chgLog)
        {
            m_editLog.AppendLog(m_strAddLog);
            m_strAddLog.Empty();
        }

        if (m_changed & chgCancellable) m_btnCancel.EnableWindow(m_bCancellable);

        m_changed = 0;
    }

    if (m_bTimed)
    {
        COleDateTimeSpan odts = COleDateTime::GetCurrentTime() - *m_pOdtTimedStart;
        m_pWndProgress->SetPos(Rint(PROGRESS_MAX * (odts.GetTotalSeconds() / m_clicks)));
    }
}

void CDlgRun::ReallyCancel()
{
    if (m_status == staSuccPlus)
    {
        EndDialog(IDOK);
        return;
    }

    if (!m_bCancellable) return;
    SetWindowText(LocalAfxString(IDS_WAITING_TERMINATE));
    m_btnCancel.EnableWindow(false);
    m_pThread->StopNoWait();
    TerminatePlus();
}

void CDlgRun::OnTimer(UINT nIDEvent)
{
    switch (nIDEvent)
    {
        case timerUpdate:
            PrivateUpdate();
            break;

        case timerReallyCancel:
            ReallyCancel();
            break;
    }

    super::OnTimer(nIDEvent);
}

void CDlgRun::OnCancel()
{
    if (IsFinished() || GetCurrentTimeBusy().GetTotalMinutes() < 10)
        ReallyCancel();
    else
        ShowDlgSD(new CDlgConfirm(this));
}

int CDlgRun::DoModal()
{
    int r = IDCANCEL;
    *m_pOdtStart = COleDateTime::GetCurrentTime();

    if (m_bRunParallel)
    {
        r = super::DoModal();
        if (AfxGetApp()->m_pMainWnd != NULL) AfxGetApp()->m_pMainWnd->SetForegroundWindow();
    }
    else
    {
        Run2(this);
        TerminatePlus();
        r = GetFinishID();
    }

    // good time to clean up TStringBuffer cache
    StaticStringBufferStorageA.Cleanup();
    StaticStringBufferStorageW.Cleanup();

    if (r != IDOK && m_errMsg != LocalAfxString(IDS_NO_MESSAGE))
    {
        if (m_bRethrow)
            ThrowMessage(m_errMsg);
        else
            AfxMB(m_errMsg);
    }

    return r;
}

BOOL CDlgRun::PreTranslateMessage(MSG* pMsg)
{
    if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) &&
        (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE))
    {
        if (pMsg->message == WM_KEYDOWN && m_status == staSuccPlus) EndDialog(IDOK);
        return true;
    }

    return super::PreTranslateMessage(pMsg);
}

void CDlgRun::OnSysCommand(UINT nID, LPARAM lParam)
{
    switch (nID)
    {
        case SC_MINIMIZE:
            AfxGetApp()->GetMainWnd()->ShowWindow(SW_MINIMIZE);
            break;

        case SC_RESTORE:
            AfxGetApp()->GetMainWnd()->ShowWindow(SW_RESTORE);
            break;
    }

    super::OnSysCommand(nID, lParam);
}

CDlgRun::TResize CDlgRun::GetResizeMask(UINT idc)
{
    switch (idc)
    {
        case IDC_RUN_ACTION:
        case IDC_RUN_PROGRESS:
        case IDC_RUN_COMMENT:
            return TResize(rzsPull, rzsNone);

        case IDCANCEL:
            return TResize(rzsMove, rzsNone);

        case IDC_LOG_LOG:
            return TResize(rzsPull, rzsPull);
    }

    return super::GetResizeMask(idc);
}
