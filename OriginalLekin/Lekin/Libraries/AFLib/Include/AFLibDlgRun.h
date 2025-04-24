#pragma once

#include "AFLibDlgSDSupport.h"
#include "AFLibEditLog.h"
#include "AFLibResource.h"

class CProgressCtrl;
class CButton;
class CCriticalSection;

namespace ATL {
class COleDateTime;
class COleDateTimeSpan;
}  // namespace ATL

namespace AFLibThread {
class CThread;
}

namespace AFLibPrivate {
class CDlgConfirm;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRun dialog
// "Progress monitor" dialog.
// Shows the progress of a lengthy operation; allows cancelling.

namespace AFLibGui {
class AFLIB CDlgRun : public CDlgSDSupport
{
private:
    typedef CDlgSDSupport super;
    DEFINE_COPY_AND_ASSIGN(CDlgRun);

    CProgressCtrl* m_pWndProgress;  // progress bar (pointer b/c it is a "custom" control
    CButton m_btnCancel;            // the "Cancel" button
    CEditLog m_editLog;             // Log edit box

protected:
    // values of the worker thread status
    enum TStatus
    {
        staRunning,
        staFailed,
        staCancelled,
        staSuccess,
        staSuccPlus
    };

private:
    volatile TStatus m_status;  // status of the worker thread

    CCriticalSection* m_pSync;        // used for syncronization
    AFLibThread::CThread* m_pThread;  // worker thread

    ATL::COleDateTime* m_pOdtStart;       // Starting time of the whole run
    ATL::COleDateTime* m_pOdtTimedStart;  // Starting time of a timed operation
    ATL::COleDateTimeSpan* m_pOdtsBusy;   // Running time
    CString m_errMsg;                     // Error message to display if operation fails

    CString m_dlgTitle;                  // Dialog title
    AFLibIO::TStringBuffer m_action;     // Text to display in the "action" (top) box
    AFLibIO::TStringBuffer m_comment;    // Text to display in the "comment" (bottom) box
    AFLibIO::TStringBuffer m_strAddLog;  // string to add to the Log

    // mask flags in m_changed
    enum
    {
        chgAction = 1,
        chgProgress = 4,
        chgComment = 8,
        chgTitle = 16,
        chgLog = 32,
        chgKillLog = 64,
        chgCancellable = 128
    };
    int m_changed;  // mask: what was changed since the last updation

    volatile bool m_bCancellable;  // operation is allowed to be cancelled

    bool m_bTimed;     // current step is a "timed" SQL call: the progress bar is updated by the timer
    double m_clicks;   // the range of the progress bar
    double m_current;  // current position of the progress bar

    // calls RunAndCatch, catches most generic messages
    static UINT Run2(LPVOID _pDlg);

    // updates all controls on the "update" timer
    void PrivateUpdate();

    // cancel the runner
    void ReallyCancel();

    // initialize step
    void StartStep(LPCTSTR label, double clicks);

    // IDOK, IDNO, or IDCANCEL depending on m_status
    int GetFinishID();

protected:
    enum
    {
        IDD = IDD_RUN
    };

    // show the log in the window? (no, show, show and keep open after finished)
    enum TShowLog
    {
        slgNone,
        slgShow,
        slgKeep
    };

    bool m_bRunParallel;  // run in parallel? (default -- yes)
    bool m_bRethrow;      // throw an exception from DoModal if error (default -- no, but show MsgBox)
    bool m_bSuppressMB;   // suppress "Operation cancelled" message box
    TShowLog m_slg;       // show the Log?

    bool CopyFile(LPCTSTR source, LPCTSTR dest, bool bThrow = true);

public:
    explicit CDlgRun(UINT nIDTemplate = IDD);
    ~CDlgRun();

    virtual int DoModal();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // get the starting time of the run
    ATL::COleDateTime GetTimeStart();

    // overall running time
    ATL::COleDateTimeSpan GetTimeBusy();

    // current running time
    ATL::COleDateTimeSpan GetCurrentTimeBusy();

    // Functions below must be called from Run()
    // They are public since sometimes they are called through a non-member

    // label the current step, prepare for looping
    virtual void Step(LPCTSTR label, double clicks);

    // wait till SQL process ends, change progress bar by timer
    virtual double TimedStep(LPCTSTR label, double seconds);

    // process a message from SQL (by default just adds it to the Log)
    virtual void ProcessSQLMsg(LPCTSTR msg);

    // add one or more "clicks" to the progess bar
    void AddClick(double clicks = 1.);

    // set the total number of clicks
    void SetTotalClicks(double clicks);

    // set progess bar
    void SetCurrent(double current);

    // set bottom status box
    void SetComment(LPCTSTR comment);

    // set dialog title
    void SetDlgTitle(LPCTSTR dlgTitle);

    // clear the log
    void ClearLog();

    // add message(s) to the log
    void AppendLog(LPCTSTR msg);

    // set cancellable state
    void SetCancellable(bool bCancellable);

    // is the user allowed to cancel?
    bool IsCancellable();

    // show a message box, with "time spent" appended
    void TimedMsgBox(LPCTSTR s);

    // get the log accumulated in m_pLstLog
    const AFLibIO::TStringBuffer& GetMsgLog()
    {
        return m_editLog.GetLog();
    }

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDEvent);
    virtual void OnCancel();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    virtual TResize GetResizeMask(UINT idc);

    // checks whether the running thread has been finished
    bool IsFinished();

    // computes the running time till the current moment
    void ComputeBusyTime();

    // quietly interrupts the execution
    void ThrowNoMessage();

    // override to use TimedStep()
    virtual void WaitStep();

    // override if any extra processes are being run
    virtual void TerminatePlus();

    // override to log times
    virtual void Log(LPCTSTR line);

    // calls Run(), catches exceptions
    virtual void RunAndCatch(CString& errMsg, TStatus& status);

    // main overridable -- does all the work
    virtual void Run();

    // cleans up after the run
    virtual void Cleanup();

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibGui
