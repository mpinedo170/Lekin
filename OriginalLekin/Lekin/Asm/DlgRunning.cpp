#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgRunning.h"

#include "Misc.h"

const TCHAR specChar = '\b';

/////////////////////////////////////////////////////////////////////////////
// CDlgRunning dialog

CDlgRunning::CDlgRunning(UINT nIDTemplate) : super(nIDTemplate)
{
    m_result = Ok;
    m_code = -1;
    m_hIn = m_hOut = m_hProcess = m_hThread = NULL;
    m_maxX = m_maxY = 0;

    m_bIo = true;
    m_status = Going;
    m_bReadPending = false;
    m_writePending = 0;
}

BEGIN_MESSAGE_MAP(CDlgRunning, super)
ON_BN_CLICKED(IDC_INTERRUPT, OnInterrupt)
ON_WM_TIMER()
ON_WM_DESTROY()
END_MESSAGE_MAP()

bool CDlgRunning::ProcessFinished()
{
    return WaitForSingleObject(m_hProcess, 0) != WAIT_TIMEOUT;
}

void CDlgRunning::Terminate()
{
    TerminateProcess(m_hProcess, 0xFFFF);
}

void CDlgRunning::Close(HANDLE& F)
{
    if (F != NULL) CloseHandle(F);
    F = NULL;
}

void CDlgRunning::PrivateClearConsole(int x)
{
    DWORD temp;
    COORD coo = {static_cast<SHORT>(x), 0};
    FillConsoleOutputCharacter(m_hOut, ' ', m_maxX * m_maxY - x, coo, &temp);
    SetConsoleCursorPosition(m_hOut, coo);
}

void CDlgRunning::ClearConsole()
{
    PrivateClearConsole(0);
    m_lastLine.Empty();
}

void CDlgRunning::AddEvent(TCHAR c)
{
    INPUT_RECORD rec = {KEY_EVENT, {true, 1, 0, 0, {static_cast<WCHAR>(c)}}};
    switch (c)
    {
        case _T('\r'):
            rec.Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
            break;
        case _T('\b'):
            rec.Event.KeyEvent.wVirtualKeyCode = VK_BACK;
            break;
    }

    DWORD temp;
    WriteConsoleInput(m_hIn, &rec, 1, &temp);
    rec.Event.KeyEvent.bKeyDown = false;
    WriteConsoleInput(m_hIn, &rec, 1, &temp);
}

void CDlgRunning::StartReading()
{
    if (!m_bIo) return;
    m_bReadPending = false;
    m_status = Reading;
}

void CDlgRunning::StartWriting()
{
    if (!m_bIo) return;
    m_writePending = 0;
    m_status = Writing;
}

void CDlgRunning::Continue()
{
    switch (m_status)
    {
        case Writing:
            if (!m_bIo) break;
            for (int i = 0; i < m_sWrite.GetLength(); ++i) AddEvent(m_sWrite[i]);
            AddEvent(specChar);
            break;
        case Killed:
            return;
    }

    m_status = Going;
    ResumeThread(m_hThread);
    SetTimer(timerUpdate, 200, NULL);
}

void CDlgRunning::Kill()
{
    m_status = Killed;
    Stop();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRunning message handlers

BOOL CDlgRunning::OnInitDialog()
{
    m_btnInterrupt.SubclassDlgItem(IDC_INTERRUPT, this);
    super::OnInitDialog();
    m_btnInterrupt.SetFocus();

    if (m_bIo)
    {
        if (!theApp.OpenConsole())
        {
            m_result = Error;
            EndDialog(IDCANCEL);
            return true;
        }

        m_hIn = GetStdHandle(STD_INPUT_HANDLE);
        m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(m_hOut, &info);
        m_maxX = info.dwMaximumWindowSize.X;
        m_maxY = info.dwMaximumWindowSize.Y;

        FlushConsoleInputBuffer(m_hIn);
        ClearConsole();
        AddEvent(specChar);
    }

    if (Initialize())
    {
        m_result = Error;
        EndDialog(IDCANCEL);
        return true;
    }

    Start();
    if (m_hProcess != NULL)
        SetTimer(timerUpdate, 200, NULL);
    else
        EndDialog(IDCANCEL);
    return false;
}

void CDlgRunning::OnInterrupt()
{
    m_result = Interrupted;
    Terminate();
    Kill();
}

BOOL CDlgRunning::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return 1;
    return super::PreTranslateMessage(pMsg);
}

UINT CDlgRunning::GetScreenInfo(LPVOID pData)
{
    return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), (CONSOLE_SCREEN_BUFFER_INFO*)pData)
               ? 1
               : 0;
}

void CDlgRunning::OnTimer(UINT nIDEvent)
{
    CSingleLock lock(&m_timerMutex);
    if (!lock.Lock(0)) return;

    KillTimer(timerUpdate);
    SuspendThread(m_hThread);

    if (m_result == Interrupted)
    {
        Kill();
        return;
    }

    if (m_bIo)
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        CWinThread* pThread =
            AfxBeginThread(GetScreenInfo, &info, THREAD_PRIORITY_ABOVE_NORMAL, 0, CREATE_SUSPENDED);
        if (pThread == NULL) goto END;
        pThread->m_bAutoDelete = false;
        pThread->ResumeThread();

        bool bOk = WaitForSingleObject(pThread->m_hThread, 50) == WAIT_OBJECT_0;
        DWORD res = 0;
        if (!bOk)
            TerminateThread(pThread->m_hThread, res);
        else
            GetExitCodeThread(pThread->m_hThread, &res);

        delete pThread;
        pThread = NULL;
        if (res != 1) goto END;

        COORD& cur = info.dwCursorPosition;
        int chars = cur.Y * m_maxX + cur.X;
        bool bLineChanged = cur.Y > 0;

        COORD first = {0, 0};
        DWORD temp;
        CString output;
        ReadConsoleOutputCharacter(m_hOut, output.GetBuffer(chars + 1), chars, first, &temp);
        chars = temp;
        output.ReleaseBuffer(chars);

        if (m_writePending < 2)
        {
            INPUT_RECORD rec;
            if (PeekConsoleInput(m_hIn, &rec, 1, &temp) && temp == 0)
            {
                ++m_writePending;
                if (m_writePending < 2) AddEvent(specChar);
            }
            else
                m_writePending = 0;
        }

        if (chars > 0)
        {
            CString line = output.Left(m_maxX);
            if (bLineChanged)
            {
                int i = line.GetLength();
                while (--i >= m_lastLine.GetLength())
                    if (!isspace(line[i])) break;
                line = line.Left(i + 1);
            }

            int len = m_lastLine.GetLength();

            if (line.Left(len) != m_lastLine)
            {
                m_sRead += strCarRet;
                m_sRead += line;
                m_bReadPending = true;
            }
            else if (line.GetLength() > len)
            {
                m_sRead += line.Mid(len);
                m_bReadPending = true;
            }
            else
                m_bReadPending = bLineChanged;

            for (int i = m_maxX; i <= chars; i += m_maxX)
            {
                line = output.Mid(i, m_maxX);
                if (line.GetLength() == m_maxX) line.TrimRight();
                m_sRead += strEol;
                m_sRead += line;
            }
            m_lastLine = line;

            if (bLineChanged)
            {
                PrivateClearConsole(cur.X);
                WriteConsoleOutputCharacter(m_hOut, line, line.GetLength(), first, &temp);
            }
        }

        if (m_bReadPending)
        {
            StartReading();
            OnRead();
            Update();
            return;
        }

        if (m_writePending >= 2 && !ProcessFinished())
        {
            StartWriting();
            OnWrite();
            Update();
            return;
        }
    }

    Update();
    if (GetStatus() != Going) return;

END:
    if (m_bReadPending || !ProcessFinished())
        Continue();
    else
        Kill();
}

void CDlgRunning::OnDestroy()
{
    Close(m_hThread);
    Close(m_hProcess);
    if (m_bIo) theApp.CloseConsole();
    super::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRunning overridables

bool CDlgRunning::Initialize()
{
    return false;
}

void CDlgRunning::Update()
{}

void CDlgRunning::Stop()
{
    if (!ProcessFinished()) Terminate();

    if (m_result == Interrupted)
    {
        EndDialog(IDCANCEL);
        return;
    }

    if (m_result == Ok)
    {
        GetExitCodeProcess(m_hProcess, &m_code);
        if (m_code != 0) m_result = Error;
    }
    SetForegroundWindow();
    LekinBeep(m_result == Ok);

    m_btnInterrupt.SetWindowText(_T("Close"));
    int res = m_result == Ok ? IDOK : IDCANCEL;
    m_btnInterrupt.SetDlgCtrlID(res);
    SetDefID(res);

    CString title;
    AfxFormatString1(title, IDS_READY + m_result, IntToStr(m_code));
    SetWindowText(title);
}

void CDlgRunning::OnRead()
{
    m_sRead.Empty();
}

void CDlgRunning::OnWrite()
{
    m_sWrite = "1";
}

void CDlgRunning::Start()
{}
