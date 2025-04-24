#pragma once

#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibModalCounter.h"
#include "AFLibPointers.h"

/////////////////////////////////////////////////////////////////////////////
// CTempTT template
// Implements tool tips and common functions.
// Since it is a template, can be used to add this functionality
//   not only to plain CDialog, but to CPropertyPage or anything else

namespace AFLibGui {
template <class W>
class CTempTT : public W
{
private:
    typedef W super;

    bool m_bFallThrough;  // fall-through flag for keyboard messages

    // this function is necessary for AFLIB_TEMP_CONSTRUCTORS
    void PrivateInit()
    {
        m_bFallThrough = false;
    }

    // process WM_NOTIFY message
    bool PrivateOnNotify(WPARAM wParam, LPARAM lParam);

protected:
    AFLIB_TEMP_CONSTRUCTORS(CTempTT)

    // tool tip for a control (redefine if necessary)
    virtual CString GetToolTip(UINT idc)
    {
        CString s = AFLib::AfxString(idc);
        int i = s.Find(AFLib::chrNewLine);
        if (i >= 0) s.Delete(i, s.GetLength() - i);
        return s;
    }

    // redefinition of CWnd::OnNotify()
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
    {
        if (PrivateOnNotify(wParam, lParam))
        {
            *pResult = 1;
            return true;
        }
        return super::OnNotify(wParam, lParam, pResult);
    }

    // redefinition of CWnd::OnToolHitTest()
    virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const
    {
        HWND hWndChild = AFLibGui::FindLiveChildFromPoint(m_hWnd, point);
        if (hWndChild == NULL) return -1;

        if (pTI != NULL)
        {
            pTI->hwnd = m_hWnd;
            pTI->uId = reinterpret_cast<UINT_PTR>(hWndChild);
            pTI->uFlags |= TTF_IDISHWND | TTF_NOTBUTTON | TTF_CENTERTIP | TTF_ALWAYSTIP;
            pTI->lpszText = LPSTR_TEXTCALLBACK;
        }
        return 1;
    }

public:
    // DoModal() counts the number of stacked calls
    virtual int DoModal()
    {
        TModalCounter modalCounter;
        return super::DoModal();
    }

    // PreTranslateMessage() sends keystrokes to the parent window
    // if the fall-through flag has been set
    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (m_bFallThrough && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) return false;

        return super::PreTranslateMessage(pMsg);
    }

    // check if WM_SYSKEYDOWN message can be processed by this window
    bool IsMsgHot(MSG* pMsg);

    // get fall-through flag
    bool IsFallThrough()
    {
        return m_bFallThrough;
    }

    // set fall-through flag
    void SetFallThrough(bool bFallThrough)
    {
        m_bFallThrough = bFallThrough;
    }

    // check if dlg item is enabled
    bool IsDlgItemEnabled(UINT idc)
    {
        CWnd* pWnd = GetDlgItem(idc);
        if (pWnd == NULL) return false;
        return pWnd->IsWindowEnabled() != 0;
    }

    // same as IsDlgButtonChecked(), but returns bool
    bool IsDlgItemChecked(UINT idc)
    {
        return IsDlgButtonChecked(idc) != 0;
    }

    // check if dlg button is enabled and checked
    bool IsDlgItemEC(UINT idc)
    {
        return IsDlgItemEnabled(idc) && IsDlgItemChecked(idc);
    }

    // enable/disable one dlg item
    void EnableDlgItem(UINT idc, bool bEna)
    {
        CWnd* pWnd = GetDlgItem(idc);
        if (pWnd != NULL) pWnd->EnableWindow(bEna);
    }

    // enable/disable several dlg items
    void EnableDlgItems(LPCUINT arrIDC, bool bEna)
    {
        for (int i = 0; arrIDC[i] != 0; ++i) EnableDlgItem(arrIDC[i], bEna);
    }

    // show/hide one dlg item
    void ShowDlgItem(UINT idc, bool bShow)
    {
        CWnd* pWnd = GetDlgItem(idc);
        if (pWnd != NULL) pWnd->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
    }

    // show/hide several dlg items
    void ShowDlgItems(LPCUINT arrIDC, bool bShow)
    {
        for (int i = 0; arrIDC[i] != 0; ++i) ShowDlgItem(arrIDC[i], bShow);
    }

    // for edit dlg items -- set read only
    void SetDlgItemReadOnly(UINT idc, bool bRO)
    {
        CWnd* pWnd = GetDlgItem(idc);
        if (pWnd != NULL) pWnd->SendMessage(EM_SETREADONLY, WPARAM(bRO));
    }

    // set several dlg items read only
    void SetDlgItemsReadOnly(LPCUINT arrIDC, bool bRO)
    {
        for (int i = 0; arrIDC[i]; ++i) SetDlgItemReadOnly(arrIDC[i], bRO);
    }

    // read radio button
    int GetDlgRadioButton(UINT idc0, int count)
    {
        UINT idc = GetCheckedRadioButton(idc0, idc0 + count - 1);
        return idc == 0 ? count : idc - idc0;
    }

    // set radio button
    void SetDlgRadioButton(UINT idc0, int count, int value)
    {
        CheckRadioButton(idc0, idc0 + count - 1, idc0 + value);
    }
};

template <class W>
bool CTempTT<W>::PrivateOnNotify(WPARAM wParam, LPARAM lParam)
{
    NMHDR* pNMHDR = reinterpret_cast<NMHDR*>(lParam);
    HWND hWndChild = (HWND)pNMHDR->idFrom;

    // if this is a spin button that has a buddy, return buddy window
    const CString sClass = AFLibGui::GetWindowClass(hWndChild);
    if (sClass.CompareNoCase(_T("msctls_updown32")) == 0)
    {
        HWND hWndChild2 = (HWND)::SendMessage(hWndChild, UDM_GETBUDDY, 0, 0l);
        if (::IsWindow(hWndChild2)) hWndChild = hWndChild2;
    }

    const UINT idc = ::GetDlgCtrlID(hWndChild);
    if (idc == 0) return false;

    switch (pNMHDR->code)
    {
        case TTN_NEEDTEXTA:
        {
            static CStringA buffer;
            TOOLTIPTEXTA* pTTT = reinterpret_cast<TOOLTIPTEXTA*>(pNMHDR);
            if (!(pTTT->uFlags & TTF_IDISHWND)) return false;

            CString s = GetToolTip(idc);
            AFLib::Capitalize(s);
            buffer = s;

            pTTT->lpszText = LPSTR(LPCSTR(buffer));
            pTTT->hinst = NULL;
            return true;
        }

        case TTN_NEEDTEXTW:
        {
            static CStringW buffer;
            TOOLTIPTEXTW* pTTT = reinterpret_cast<TOOLTIPTEXTW*>(pNMHDR);
            if (!(pTTT->uFlags & TTF_IDISHWND)) return false;

            CString s = GetToolTip(idc);
            AFLib::Capitalize(s);
            buffer = s;

            pTTT->lpszText = LPWSTR(LPCWSTR(buffer));
            pTTT->hinst = NULL;
            return true;
        }
    }

    return false;
}

template <class W>
bool CTempTT<W>::IsMsgHot(MSG* pMsg)
{
    static const CString strEdit = _T("Edit");
    static const CString strComboBox = _T("ComboBox");
    const int BUF_SIZE = 1 << 8;
    CString s;

    switch (pMsg->message)
    {
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            if (pMsg->wParam == VK_TAB || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_LEFT ||
                pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_UP)
                return true;

            // ESCAPE is hot only in an open combobox
            s = GetWindowClass(pMsg->hwnd);

            if (pMsg->wParam == VK_ESCAPE)
            {
                if (s == strComboBox)
                {  // ESCAPE in a combobox is hot if the combobox is open
                    BOOL bDropped = ::SendMessage(pMsg->hwnd, CB_GETDROPPEDSTATE, 0, 0);
                    bool bSimple = (::GetWindowLong(pMsg->hwnd, GWL_STYLE) & CBS_DROPDOWNLIST) == CBS_SIMPLE;
                    if (bDropped && !bSimple) return true;
                }
                return false;
            }

            // any key message from Edit or ComboBox, except ESCAPE, is hot
            if (s == strEdit || s == strComboBox) return true;

            return false;

        default:
            return false;
    }

    if ((pMsg->lParam & 0x20000000) == 0) return false;
    if (pMsg->wParam < 32 || pMsg->wParam > 126) return false;

    TCHAR ch = TCHAR(pMsg->wParam);
    HWND hWndChild = ::GetWindow(m_hWnd, GW_CHILD);

    for (; hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
    {
        s = GetWindowClass(hWndChild);
        if (s == strEdit || s == strComboBox) continue;

        ::GetWindowText(hWndChild, s.GetBuffer(BUF_SIZE), BUF_SIZE);
        s.ReleaseBuffer();
        if (AFLib::IsKeyHotInString(s, ch)) return true;
    }

    return false;
}
}  // namespace AFLibGui
