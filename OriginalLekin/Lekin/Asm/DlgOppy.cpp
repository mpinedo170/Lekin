#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgOppy.h"

#include "CmbWkc.h"

#include "Job.h"
#include "Misc.h"
#include "Route.h"
#include "Workcenter.h"

namespace {
const int WindowCount = 4;
const CString EmptyRow = _T("\t1\tA");
}  // namespace

/////////////////////////////////////////////////////////////////////////////
// CDlgOppy dialog

CDlgOppy::CDlgOppy(CWnd* pParent, int bottom, TRoute& route)
{
    theApp.CreateSysFont(m_font, NULL);
    Create(CDlgOppy::IDD, pParent);
    SetStdLimitText();
    AddWkcCtrl();

    for (int i = 1; i < WindowCount; ++i)
    {
        CWnd* pWnd = new CWnd;
        pWnd->Attach(::GetDlgItem(m_hWnd, IDC_WKC_ID0 + i * 1000));
        m_controls.Add(pWnd);
    }

    CRect rect, winRect;
    m_controls[0]->GetWindowRect(rect);
    ScreenToClient(rect);
    m_py = rect.top;
    m_dy = rect.Height();

    for (int i = 0; i < 3; ++i)
    {
        pParent->GetDlgItem(IDC_LAB0 + i)->GetWindowRect(rect);
        CRect rect2(rect);
        pParent->ScreenToClient(rect);
        if (i == 0)
        {
            winRect.top = rect.bottom;
            winRect.left = rect.left;
        }
        if (i == 2) winRect.right = rect.right + 3;

        rect2.top = rect2.bottom = 0;
        rect2.DeflateRect(5, 0);
        m_colRect[i] = rect2;
    }
    m_dy += 2 * m_py;

    winRect.bottom = bottom;
    int temp = GetSystemMetrics(SM_CYVTHUMB);
    winRect.right += temp;
    MoveWindow(&winRect);

    GetClientRect(rect);
    m_height = rect.Height();
    m_width = rect.Width();
    rect.left = rect.right - temp;
    m_wndScroll.MoveWindow(rect);

    for (int i = 0; i < 3; ++i) ScreenToClient(&m_colRect[i]);
    m_colRect[3] = m_colRect[2];
    m_colRect[2] = m_colRect[1];
    GetDlgItem(IDC_PROC_SPIN0)->GetWindowRect(rect);
    m_colRect[2].left = (m_colRect[1].right -= rect.Width()) - 1;

    Init(0);
    RouteToDlg(route);
    m_controls[TJob::GetShop() == FlowShop ? 1 : 0]->SetFocus();
}

void CDlgOppy::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ROUTE_SCRL, m_wndScroll);
}

BEGIN_MESSAGE_MAP(CDlgOppy, super)
ON_WM_PAINT()
ON_WM_VSCROLL()
ON_WM_LBUTTONDOWN()
ON_WM_DESTROY()
ON_CONTROL_RANGE(CBN_SETFOCUS, IDC_WKC_ID0, IDC_WKC_ID0 + 999, OnCtSetFocus)
ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_WKC_ID0, IDC_WKC_ID0 + 999, OnWkcChange)
ON_CONTROL_RANGE(EN_SETFOCUS, IDC_PROCESSING0, IDC_PROCESSING0 + 999, OnCtSetFocus)
ON_CONTROL_RANGE(EN_SETFOCUS, IDC_STATUS0, IDC_STATUS0 + 999, OnCtSetFocus)
END_MESSAGE_MAP()

void CDlgOppy::AddWkcCtrl()
{
    CCmbWkc* pWnd = new CCmbWkc;
    pWnd->SubclassDlgItem(IDC_WKC_ID0, this);
    m_controls.Add(pWnd);
    if (TJob::GetShop() == FlowShop) pWnd->EnableWindow(false);
}

void CDlgOppy::RouteToDlg(TRoute& route)
{
    ShowWindow(SW_HIDE);

    for (int i = WindowCount; i < m_controls.GetSize(); ++i) m_controls[i]->ShowWindow(SW_HIDE);

    m_good = m_sel = 0;
    SetRowAt(0, EmptyRow);

    for (int i = 0; i < route.GetCount(); ++i) InsertRow(i, route.m_a[i]);

    if (route.GetCount() == 0) ResetScrollInfo();
    GetParent()->SetDlgItemText(IDC_PROMPT, strEmpty);
    ShowWindow(SW_SHOWNORMAL);
}

void CDlgOppy::DlgToRoute(TRoute& route)
{
    route.Clear();
    for (int i = 0; i < GetGoodCount(); ++i) route.m_a.Add(GetRowAt(i));
}

int CDlgOppy::GetExCount()
{
    return m_controls.GetSize() / WindowCount;
}

void CDlgOppy::Init(int row)
{
    ((CEdit*)GetControl(row, 1))->SetLimitText(3);
    ((CSpinButtonCtrl*)GetControl(row, 2))->SetRange(1, 999);
    ((CEdit*)GetControl(row, 3))->SetLimitText(1);
}

void CDlgOppy::Reposition()
{
    for (int row = 0; row < GetCount(); ++row)
    {
        for (int i = 0; i < 4; ++i)
        {
            int y = row * m_dy + m_py - GetPos();
            int h = m_dy - 2 * m_py;
            if (i == 0) h *= 4;

            CRect rect(m_colRect[i]);
            rect.top = y;
            rect.bottom = y + h;

            GetControl(row, i)->MoveWindow(rect);
        }
    }
    InvalidateRect(NULL);
}

void CDlgOppy::CreateRow()
{
    CRect rect(0, 0, 0, 0);
    int row = GetExCount();

    for (int i = 0; i < WindowCount; ++i)
    {
        CWnd* pWnd = GetControl(0, i);
        CString className;
        GetClassName(pWnd->m_hWnd, className.GetBuffer(64), 64);
        DWORD style = pWnd->GetStyle();
        if (!i) style |= WS_VSCROLL;
        DWORD styleEx = pWnd->GetExStyle();

        CWnd* wnd2 = i == 0 ? new CCmbWkc : new CWnd;
        wnd2->CreateEx(styleEx, className, strEmpty, style, rect, this, IDC_WKC_ID0 + i * 1000 + row);
        wnd2->SetFont(&m_font);
        m_controls.Add(wnd2);
        className.ReleaseBuffer();
    }
    Init(row);
}

CWnd* CDlgOppy::GetControl(int row, int col)
{
    return m_controls[row * WindowCount + col];
}

void CDlgOppy::ResetScrollInfo()
{
    SCROLLINFO info = {
        sizeof(SCROLLINFO), SIF_PAGE | SIF_RANGE, 0, GetCount() * m_dy, static_cast<UINT>(m_height), 0, 0};
    m_wndScroll.SetScrollInfo(&info);
    Reposition();
}

int CDlgOppy::GetPos()
{
    return m_wndScroll.GetScrollPos();
}

// operations

bool CDlgOppy::Conflict(LPCTSTR idWkc, int row)
{
    for (int i = 0; i < row; ++i)
    {
        CString s;
        GetControl(i, 0)->GetWindowText(s);
        if (s == idWkc) return true;
    }
    return false;
}

bool CDlgOppy::IsValid()
{
    CWnd* pWnd = NULL;
    int proc = 0;

    try
    {
        for (int i = 0; i < m_good; ++i)
        {
            CString s;

            pWnd = GetControl(i, 0);
            pWnd->GetWindowText(s);
            if (Conflict(s, i)) ThrowMessage(IDP_DUP_OPER);

            pWnd = GetControl(i, 1);
            pWnd->GetWindowText(s);
            int p = StrToInt(s);
            CheckInt(p, 0, 999);
            proc += p;

            pWnd = GetControl(i, 3);
            pWnd->GetWindowText(s);
            CheckStatus(s);
            pWnd = NULL;
        }

        if (proc == 0) ThrowMessage(IDP_EMPTY_ROUTE);
    }
    catch (CException* pExc)
    {
        if (pWnd != NULL) pWnd->SetFocus();
        pExc->ReportError();
        pExc->Delete();
        return false;
    }

    return true;
}

bool CDlgOppy::Conflict()
{
    for (int i = 0; i < m_good; ++i)
    {
        CString idWkc;
        GetControl(i, 0)->GetWindowText(idWkc);
        if (Conflict(idWkc, i)) return true;
    }
    return false;
}

int CDlgOppy::GetCount()
{
    return m_good + (m_good < arrWorkcenter.GetSize());
}

int CDlgOppy::GetGoodCount()
{
    return m_good;
}

int CDlgOppy::GetSel()
{
    return m_sel;
}

void CDlgOppy::SetSel(int sel)
{
    if (sel < 0 || sel >= GetCount() || sel == m_sel) return;
    int foc = 0;

    CWnd* pWnd = GetFocus();
    if (pWnd)
    {
        int id = ::GetWindowLong(pWnd->m_hWnd, GWL_ID);
        if (pWnd->GetParent() == this && id >= IDC_WKC_ID0) foc = (id - IDC_WKC_ID0) / 1000;
    }

    if (foc == 0 && TJob::GetShop() == FlowShop) foc = 1;

    m_sel = sel;
    int y = m_sel * m_dy;
    if (y < GetPos())
    {
        m_wndScroll.SetScrollPos(y);
        Reposition();
    }
    else if (y > GetPos() + m_height - m_dy)
    {
        m_wndScroll.SetScrollPos(y + m_dy - m_height);
        Reposition();
    }
    else
        InvalidateRect(NULL);

    GetParent()->SendMessage(WM_TIMER, timerRoute);
    GetControl(m_sel, foc)->SetFocus();
}

CString CDlgOppy::GetRowAt(int row)
{
    ASSERT(row >= 0 && row < GetCount());
    CString buffer;
    GetControl(row, 0)->GetWindowText(buffer);
    buffer += "\t";
    CString s;
    GetControl(row, 1)->GetWindowText(s);
    buffer += s;
    buffer += "\t";
    GetControl(row, 3)->GetWindowText(s);
    return buffer + s;
}

void CDlgOppy::SetRowAt(int row, LPCTSTR data)
{
    CString s = NextToken(data, '\t');
    ((CCmbOwner*)GetControl(row, 0))->SelString(s);
    s = NextToken(data, '\t');
    GetControl(row, 1)->SetWindowText(s);
    GetControl(row, 3)->SetWindowText(NextToken(data, '\t'));
}

void CDlgOppy::InsertRow(int row, LPCTSTR data)
{
    if (m_good < arrWorkcenter.GetSize())
    {
        int count = GetCount();
        if (count == GetExCount()) CreateRow();
        for (int i = 0; i < WindowCount; ++i) GetControl(count, i)->ShowWindow(SW_SHOW);
    }
    ++m_good;

    for (int i = GetCount(); --i > row;)
    {
        CString s = GetRowAt(i - 1);
        SetRowAt(i, s);
    }
    SetRowAt(row, data);
    ResetScrollInfo();
}

void CDlgOppy::DeleteRow(int row)
{
    if (row < 0 || row >= m_good) return;

    for (int i = row + 1; i < GetCount(); ++i)
    {
        CString s = GetRowAt(i);
        SetRowAt(i - 1, s);
    }

    if (m_good == arrWorkcenter.GetSize())
        SetRowAt(m_good - 1, EmptyRow);
    else
        for (int i = 0; i < WindowCount; ++i) GetControl(m_good, i)->ShowWindow(SW_HIDE);
    --m_good;
    ResetScrollInfo();
}

void CDlgOppy::InsertNewRow()
{
    if (m_good == arrWorkcenter.GetSize()) return;
    CString s;
    EnumerateWkc s = pWkc->m_id;
    if (!Conflict(s, m_good)) break;
    EnumerateEnd InsertRow(m_sel, s + EmptyRow);
}

void CDlgOppy::MoveUp()
{
    if (m_sel <= 0) return;
    CString s1 = GetRowAt(m_sel);
    CString s2 = GetRowAt(m_sel - 1);
    SetRowAt(m_sel, s2);
    SetRowAt(m_sel - 1, s1);
    SetSel(m_sel - 1);
}

void CDlgOppy::MoveDown()
{
    if (m_sel >= m_good - 1) return;
    CString s1 = GetRowAt(m_sel);
    CString s2 = GetRowAt(m_sel + 1);
    SetRowAt(m_sel, s2);
    SetRowAt(m_sel + 1, s1);
    SetSel(m_sel + 1);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgOppy message handlers

void CDlgOppy::OnPaint()
{
    CPaintDC dc(this);
    CRect rect(0, m_sel * m_dy - GetPos(), m_width, (m_sel + 1) * m_dy - GetPos());
    dc.FillSolidRect(rect, colorDark());
}

void CDlgOppy::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (pScrollBar->m_hWnd != m_wndScroll.m_hWnd) return;
    int pos, mx;
    m_wndScroll.GetScrollRange(&pos, &mx);
    pos = GetPos();
    mx -= m_height;

    switch (nSBCode)
    {
        case SB_BOTTOM:
            pos = mx;
            break;
        case SB_LINEDOWN:
            pos += m_dy;
            break;
        case SB_LINEUP:
            pos -= m_dy;
            break;
        case SB_PAGEDOWN:
            pos += m_height;
            break;
        case SB_PAGEUP:
            pos -= m_height;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            pos = nPos;
            break;
        case SB_TOP:
            pos = 0;
            break;
    }

    pos = max(0, min(mx, pos));
    if (pos == GetPos()) return;
    m_wndScroll.SetScrollPos(pos);

    Reposition();
    int y = m_sel * m_dy;
    if (y < pos)
        SetSel(pos / m_dy + 1);
    else if (y > pos + m_height - m_dy)
        SetSel((pos + m_height) / m_dy - 1);
}

void CDlgOppy::OnLButtonDown(UINT nFlags, CPoint point)
{
    int i = (point.y + GetPos()) / m_dy;
    if (i >= 0 && i < GetCount()) SetSel(i);
}

void CDlgOppy::OnCtSetFocus(UINT id)
{
    SetSel(id % 1000);
}

void CDlgOppy::OnWkcChange(UINT id)
{
    int row = id % 1000;
    CString idWkc;
    GetDlgItemText(id, idWkc);
    if (idWkc.IsEmpty()) return;
    if (row == m_good) InsertRow(row + 1, EmptyRow);

    GetParent()->SetDlgItemText(IDC_PROMPT, Conflict() ? _T("Conflict") : strEmpty);
    GetParent()->SendMessage(WM_TIMER, timerRoute);
}

void CDlgOppy::OnDestroy()
{
    for (int i = 1; i < WindowCount; ++i) m_controls[i]->Detach();

    super::OnDestroy();
    m_controls.DestroyAll();
}
