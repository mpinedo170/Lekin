#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgRunCon.h"

#include "Misc.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunCon dialog

CDlgRunCon::CDlgRunCon() : super(CDlgRunCon::IDD)
{
    m_bCR = false;
    m_MinSize.x = m_MinSize.y = 0;
    m_editStart = 0;
}

BEGIN_MESSAGE_MAP(CDlgRunCon, super)
ON_WM_GETMINMAXINFO()
ON_WM_SIZE()
ON_NOTIFY(EN_PROTECTED, IDC_STATUS, OnProtected)
END_MESSAGE_MAP()

void CDlgRunCon::Add(LPCTSTR s)
{
    if (*s == 0) return;

    CString sOld;
    m_editStatus.GetWindowText(sOld);
    CString sNew = sOld;

    for (; *s; ++s) switch (*s)
        {
            case '\r':
                m_bCR = true;
                break;
            case chrNewLine:
                m_bCR = false;
                sNew += strEol;
                break;
            default:
                if (m_bCR)
                {
                    int i = sNew.ReverseFind(chrNewLine) + 1;
                    sNew = sNew.Left(i);
                }
                m_bCR = false;
                sNew += *s;
        }

    int common = min(sNew.GetLength(), sOld.GetLength());
    for (int i = 0; i < common; ++i)
        if (sNew[i] != sOld[i])
        {
            common = i;
            break;
        }

    CString s1 = sNew.Mid(common);
    m_editStatus.SetSel(common, sOld.GetLength());
    m_editStatus.ReplaceSel(s1);
    m_editStatus.SetSel(common, sNew.GetLength());
    m_editStatus.SetSelectionCharFormat(m_outFormat);
    m_editStatus.SetSel(sNew.GetLength(), sNew.GetLength());
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRunCon message handlers

void CDlgRunCon::OnSize(UINT nType, int cx, int cy)
{
    super::OnSize(nType, cx, cy);
    if (!::IsWindow(m_btnInterrupt.m_hWnd)) return;

    CRect rect;
    m_btnInterrupt.GetWindowRect(rect);
    VERIFY(m_btnInterrupt.SetWindowPos(
        NULL, (cx - rect.Width()) / 2, cy - m_cancelY, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));

    VERIFY(m_editStatus.SetWindowPos(
        NULL, -1, -1, cx - m_statD.cx, cy - m_statD.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));

    m_wndTime.GetWindowRect(rect);
    VERIFY(m_wndTime.SetWindowPos(NULL, cx - rect.Width() - m_statD.cx, cy - m_cancelY, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));

    m_editStatus.GetClientRect(rect);
    m_editStatus.SetRect(rect);
}

void CDlgRunCon::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    super::OnGetMinMaxInfo(lpMMI);
    lpMMI->ptMinTrackSize = m_MinSize;
}

void CDlgRunCon::OnProtected(NMHDR* pNMHDR, LRESULT* pResult)
{
    ENPROTECTED* pMsg = (ENPROTECTED*)pNMHDR;

    if (pMsg->msg == WM_COPY)
    {
        *pResult = 0;
        return;
    }

    if (pMsg->chrg.cpMin >= m_editStart)
    {
        *pResult = 0;
        return;
    }

    LekinBeep();
    *pResult = 1;
}

BOOL CDlgRunCon::PreTranslateMessage(MSG* pMsg)
{
    if (GetStatus() == Writing && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        CString s;
        m_editStatus.GetWindowText(s);
        int len = s.GetLength();

        m_editStatus.SetSel(m_editStart, len);
        m_sWrite = m_editStatus.GetSelText() + _T("\r");
        m_editStatus.SetSelectionCharFormat(m_in2Format);

        m_editStatus.SetBackgroundColor(false, colorNormal());
        m_editStatus.SetReadOnly(true);

        Add(strEol);
        ClearConsole();
        m_editStart = len;
        Continue();
        return 1;
    }
    return super::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRunCon overridables

bool CDlgRunCon::Initialize()
{
    CRect rect;
    GetWindowRect(rect);
    m_MinSize = CPoint(rect.Size());
    GetClientRect(rect);

    CWnd* pWnd = GetDlgItem(IDC_STATUS);
    CRect rect2;
    pWnd->GetWindowRect(rect2);
    ScreenToClient(rect2);
    pWnd->DestroyWindow();
    pWnd = NULL;

    m_editStatus.Create(ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | ES_NUMBER | WS_BORDER | WS_VSCROLL |
                            WS_TABSTOP | WS_VISIBLE,
        rect2, this, IDC_STATUS);
    m_statD = rect.Size() - rect2.Size();

    m_btnInterrupt.GetWindowRect(rect2);
    ScreenToClient(rect2);
    m_cancelY = rect.Height() - rect2.top;

    memset(&m_outFormat, 0, sizeof(CHARFORMAT));
    m_outFormat.cbSize = sizeof(CHARFORMAT);
    m_outFormat.dwMask = CFM_BOLD | CFM_COLOR | CFM_PROTECTED;
    m_outFormat.dwEffects = CFE_PROTECTED;
    m_outFormat.crTextColor = colorBlack;
    m_outFormat.bCharSet = DEFAULT_CHARSET;
    m_outFormat.bPitchAndFamily = FIXED_PITCH;
    _tcscpy_s(m_outFormat.szFaceName, LF_FACESIZE, strCourier);

    m_in2Format = m_outFormat;
    m_in2Format.dwEffects = CFE_BOLD | CFE_PROTECTED;

    m_inFormat = m_in2Format;
    m_inFormat.crTextColor = RGB(0, 0, 255);

    CreateCourierFont(m_font, NULL);
    m_editStatus.SetFont(&m_font);
    m_editStatus.SetEventMask(ENM_PROTECTED);
    m_editStatus.SetDefaultCharFormat(m_inFormat);
    m_editStatus.SetBackgroundColor(false, colorNormal());
    m_editStatus.SetFocus();
    return super::Initialize();
}

void CDlgRunCon::OnRead()
{
    Add(m_sRead);
    m_sRead.Empty();
    Continue();
}

void CDlgRunCon::OnWrite()
{
    CString s;
    m_editStatus.GetWindowText(s);
    m_editStart = s.GetLength();
    SetForegroundWindow();

    m_editStatus.SetSel(m_editStart, m_editStart);
    m_editStatus.SetSelectionCharFormat(m_inFormat);

    m_editStatus.SetReadOnly(false);
    m_editStatus.SetFocus();
    m_editStatus.SetBackgroundColor(false, colorWindow());
    LekinBeep(true);
}
