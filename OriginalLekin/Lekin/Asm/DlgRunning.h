#pragma once

#include "DlgA.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunning dialog

class CDlgRunning : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_RUNNING
    };

public:
    CDlgRunning(UINT nIDTemplate);

    enum TEnd
    {
        Ok,
        Error,
        Timeout,
        Interrupted
    } m_result;
    DWORD m_code;
    bool ProcessFinished();
    void Terminate();
    static UINT GetScreenInfo(LPVOID pData);

protected:
    CButton m_btnInterrupt;

    bool m_bIo;
    CString m_sRead;
    CString m_sWrite;

    HANDLE m_hProcess;
    HANDLE m_hThread;
    HANDLE m_hIn;
    HANDLE m_hOut;

    enum TRunStat
    {
        Going,
        Reading,
        Writing,
        Killed
    };

private:
    bool m_bReadPending;
    int m_writePending;
    TRunStat m_status;
    int m_maxX;
    int m_maxY;
    CString m_lastLine;
    CMutex m_timerMutex;

    void PrivateClearConsole(int x);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnInterrupt();
    afx_msg void OnDestroy();

    void Close(HANDLE& F);
    void ClearConsole();
    void AddEvent(TCHAR c);

    TRunStat GetStatus()
    {
        return m_status;
    }

    void StartReading();
    void StartWriting();
    void Continue();
    void Kill();

    virtual bool Initialize();
    virtual void Start();
    virtual void Update();
    virtual void Stop();

    virtual void OnRead();
    virtual void OnWrite();

    DECLARE_MESSAGE_MAP()
};
