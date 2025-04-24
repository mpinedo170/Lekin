#include "StdAfx.h"

#include "AFLibEditCode.h"

#include "AFLibGlobal.h"
#include "AFLibStringTokenizerText.h"
#include "AFLibStrings.h"

using AFLibGui::CEditCode;
using namespace AFLib;
using namespace AFLibIO;

CEditCode::CEditCode()
{
    m_pWndLabel = NULL;
    m_idcLabel = 0;
}

void CEditCode::FlushCursorLabel()
{
    if (m_pWndLabel != NULL)
        m_pWndLabel->SetWindowText(m_cursorLabel);
    else if (m_idcLabel != NULL)
        GetParent()->SetDlgItemText(m_idcLabel, m_cursorLabel);
}

void CEditCode::SetCursorLabel(int xPos, int yPos)
{
    CString cursorLabel;

    if (xPos >= 0 && yPos >= 0) cursorLabel = IntToStr(yPos + 1) + strColon + IntToStr(xPos + 1);
    if (cursorLabel == m_cursorLabel) return;

    m_cursorLabel = cursorLabel;
    FlushCursorLabel();
}

void CEditCode::SetCursorLabelWnd(CWnd* pWndLabel)
{
    m_pWndLabel = pWndLabel;
    m_idcLabel = 0;
    FlushCursorLabel();
}

void CEditCode::SetCursorLabelWnd(UINT idcLabel)
{
    m_pWndLabel = NULL;
    m_idcLabel = idcLabel;
    FlushCursorLabel();
}

CString CEditCode::GetTrimmedText()
{
    CString code;
    GetWindowText(code);

    TStringBuffer code2;
    TStringBuffer sZero;
    TStringTokenizerText sp(code);

    while (!sp.IsEof())
    {
        CString s = sp.ReadStr();
        s.Replace(chrFormFeed, chrSpace);
        s.TrimRight();

        if (s.IsEmpty())
            sZero += strEol;
        else
        {
            code2 += sZero;
            code2 += s;
            code2 += strEol;
            sZero.Empty();
        }
    }

    return LPCTSTR(code2);
}

void CEditCode::OnInit()
{
    CFont* pFont = GetParent()->GetFont();
    LOGFONT logFont = {0};
    pFont->GetLogFont(&logFont);
    _tcscpy_s(logFont.lfFaceName, LF_FACESIZE, strCourier);
    m_fontCourier.CreateFontIndirect(&logFont);
    SetFont(&m_fontCourier, false);
}

BEGIN_MESSAGE_MAP(CEditCode, CEdit)
ON_WM_CREATE()
ON_WM_KEYDOWN()
ON_MESSAGE(WM_PASTE, OnPaste)
END_MESSAGE_MAP()

void CEditCode::PreSubclassWindow()
{
    super::PreSubclassWindow();
    OnInit();
}

int CEditCode::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;
    OnInit();
    return 0;
}

BOOL CEditCode::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        int nStartChar = 0;
        int nEndChar = 0;
        GetSel(nStartChar, nEndChar);

        int nLine = LineFromChar(nStartChar);
        int nLineStart = LineIndex(nLine);
        int lineLength = LineLength(nLineStart);

        CAutoVectorPtr<TCHAR> s(new TCHAR[lineLength + 4]);
        GetLine(nLine, s, lineLength + 1);
        s[lineLength] = 0;

        int mxTab = min(lineLength, nStartChar - nLineStart);
        int i = 0;

        for (; i < mxTab; ++i)
            if (s[i] != chrSpace) break;

        ReplaceSel(strEol + CString(chrSpace, i));
        return true;
    }

    return super::PreTranslateMessage(pMsg);
}

LRESULT CEditCode::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = super::WindowProc(message, wParam, lParam);

    switch (message)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        {
            CPoint pt = GetCaretPos();
            DWORD pos = CharFromPos(pt);
            int yPos = short(HIWORD(pos));
            int xPos = short(LOWORD(pos)) - LineIndex(yPos);
            SetCursorLabel(xPos, yPos);
            break;
        }

        case WM_SETTEXT:
            SetCursorLabel(-1, -1);
            break;

        case WM_SETFOCUS:
        {
            int nStartChar = 0;
            int nEndChar = 0;
            GetSel(nStartChar, nEndChar);

            if (nStartChar == 0 && nEndChar == GetWindowTextLength()) SetSel(0, 0);
            break;
        }
    }

    return res;
}

void CEditCode::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar != VK_TAB)
    {
        super::OnKeyDown(nChar, nRepCnt, nFlags);
        return;
    }

    bool bShift = GetKeyState(VK_SHIFT) < 0;
    int nStartChar = 0;
    int nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    if (nStartChar == nEndChar)
    {
        if (bShift) return;
        int nStartLine = LineIndex();
        int nChar = nStartChar - nStartLine;

        ReplaceSel(nChar % 2 == 0 ? str2Space : strSpace, true);
        return;
    }

    int nStartLine = LineFromChar(nStartChar);
    int nEndLine = LineFromChar(nEndChar);
    CString s;
    GetWindowText(s);

    nStartChar = LineIndex(nStartLine);
    if (LineIndex(nEndLine) < nEndChar)
    {
        nEndChar = LineIndex(nEndLine + 1);
        if (nEndChar < 0) nEndChar = s.GetLength();
    }

    SetSel(nStartChar, nEndChar);
    s.Delete(nEndChar, s.GetLength() - nEndChar);
    s.Delete(0, nStartChar);

    TStringTokenizerText sp(s);
    TStringBuffer sNew;

    while (!sp.IsEof())
    {
        CString line = sp.ReadStr();

        if (bShift)
        {
            for (int i = 0; i < 2; ++i)
            {
                if (line.IsEmpty() || line[0] != chrSpace) break;
                line.Delete(0);
            }
        }
        else
            line.Insert(0, str2Space);
        sNew += line;
        sNew += strEol;
    }

    ReplaceSel(sNew, true);
    SetSel(nStartChar, nStartChar + sNew.GetLength());
}

LRESULT CEditCode::OnPaste(WPARAM, LPARAM)
{
    CString s = ReadClipboardString();
    TStringTokenizerText sp(s);
    TStringBuffer sNew;

    int nStartChar = 0;
    int nEndChar = 0;
    GetSel(nStartChar, nEndChar);

    int nStartLine = LineFromChar(nStartChar);
    int col = nStartChar - LineIndex(nStartLine);

    while (!sp.IsEof())
    {
        CString line = sp.ReadStr();

        for (int i = 0; i < line.GetLength(); ++i)
        {
            TCHAR ch = line[i];

            if (ch == chrTab)
            {
                int n = 8 - (col % 8);
                sNew.AppendCh(chrSpace, n);
                col += n;
                continue;
            }

            if (IsCntrl(ch)) ch = chrSpace;
            sNew += ch;
            ++col;
        }

        if (!sp.GetLastSeparator().IsEmpty()) sNew += strEol;
        col = 0;
    }

    ReplaceSel(sNew, true);
    return 0;
}
