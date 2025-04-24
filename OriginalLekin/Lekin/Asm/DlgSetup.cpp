#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgSetup.h"

#include "DlgLoadSetup.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgSetup dialog

CDlgSetup::CDlgSetup(TSetup& setup, int dim) : super(CDlgSetup::IDD), m_setup(setup), m_dim(dim)
{
    theApp.CreateSysFont(m_font, NULL);
}

void CDlgSetup::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DIM_SPIN, m_spinDim);
}

BEGIN_MESSAGE_MAP(CDlgSetup, super)
ON_EN_CHANGE(IDC_DIM, OnChangeDim)
ON_BN_CLICKED(IDC_LOAD, OnLoad)
ON_CONTROL_RANGE(EN_KILLFOCUS, 5000, 5000 + 26 * 26, OnKillFocus)
END_MESSAGE_MAP()

void CDlgSetup::PutButton(UINT id, int place)
{
    GetDlgItem(id)->SetWindowPos(NULL, m_width * place / 6 - m_butSize.cx / 2, m_height, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDlgSetup::Resize()
{
    CRect rect;

    GetDlgItem(IDC_DIM)->GetWindowRect(rect);
    ScreenToClient(rect);
    int h = rect.Height();
    int w = rect.Width();
    int top = rect.bottom + int(h * 1.5);
    int left = h;

    // create controls
    for (int i = 0; i < m_dim; ++i)
    {
        CSize sz(w, h);

        CString s(TCHAR(i + 'A'));
        if (!m_xAxis[i].m_hWnd)
        {
            int x = int((i + 1) * w * 1.2);
            CRect rect(CPoint(left + x, top), sz);
            m_xAxis[i].Create(s, WS_VISIBLE | WS_CHILD | SS_CENTER, rect, this);
            m_xAxis[i].SetFont(&m_font);
        }
        else
            m_xAxis[i].ShowWindow(SW_SHOW);

        if (!m_yAxis[i].m_hWnd)
        {
            int y = int(i * h * 1.2 + h * 1.3);
            CRect rect(CPoint(left, top + y), sz);
            m_yAxis[i].Create(s, WS_VISIBLE | WS_CHILD | SS_CENTER, rect, this);
            m_yAxis[i].SetFont(&m_font);
        }
        else
            m_yAxis[i].ShowWindow(SW_SHOW);

        for (int j = 0; j < m_dim; ++j)
            if (!m_Edit[i][j].m_hWnd)
            {
                int x = int((j + 1) * w * 1.2);
                int y = int((i + 1) * h * 1.2);
                m_Edit[i][j].CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), NULL,
                    WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_NUMBER, left + x, top + y, sz.cx, sz.cy, m_hWnd,
                    HMENU(5000 + 26 * i + j));
                m_Edit[i][j].SetFont(&m_font);
            }
            else
                m_Edit[i][j].ShowWindow(SW_SHOW);
    }

    for (int i = 0; i < 26; ++i)
    {
        for (int j = (i < m_dim ? m_dim : 0); j < 26; ++j)
        {
            if (m_Edit[i][j].m_hWnd == NULL) break;
            m_Edit[i][j].ShowWindow(SW_HIDE);
        }
        if (i < m_dim) continue;

        if (!m_xAxis[i].m_hWnd) continue;
        m_xAxis[i].ShowWindow(SW_HIDE);
        m_yAxis[i].ShowWindow(SW_HIDE);
    }

    m_width = left * 2 + int((m_dim + 1) * w * 1.2);
    m_height = top + int((m_dim + 1.5) * h * 1.2);

    m_width = max(m_width, int(m_butSize.cx * 3));
    GetDlgItem(IDC_LOAD)->GetWindowRect(rect);
    ScreenToClient(rect);
    m_width = max(m_width, int(rect.right + left));

    PutButton(IDOK, 1);
    PutButton(IDCANCEL, 3);
    PutButton(ID_HELP, 5);

    SetWindowPos(NULL, -1, -1, m_width + GetSystemMetrics(SM_CXDLGFRAME) * 2,
        m_height + m_butSize.cy * 3 / 2 + GetSystemMetrics(SM_CYDLGFRAME) * 2 +
            GetSystemMetrics(SM_CYCAPTION),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDlgSetup::DataToDlg(TSetup& setup)
{
    int dim = setup.GetDim();
    if (dim >= 1 && dim <= 26) m_dim = dim;
    SetDlgItemText(IDC_DIM, IntToStr(m_dim));
    Resize();

    for (int i = 0; i < m_dim; ++i)
        for (int j = 0; j < m_dim; ++j)
        {
            int set = setup.GetAt(i, j);
            CString s;
            if (set > 0) s = IntToStr(set);
            m_Edit[i][j].SetWindowText(s);
        }
}

void CDlgSetup::DlgToData(TSetup& setup)
{
    setup.SetDim(0);
    for (int i = 0; i < m_dim; ++i)
        for (int j = 0; j < m_dim; ++j)
        {
            CString s;
            m_Edit[i][j].GetWindowText(s);
            int set = StrToInt(s);
            if (set > 0) setup.SetAtGrow(i, j, set);
        }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSetup message handlers

BOOL CDlgSetup::OnInitDialog()
{
    super::OnInitDialog();
    CRect rect;
    GetDlgItem(IDOK)->GetWindowRect(rect);
    m_butSize = rect.Size();
    m_spinDim.SetRange(1, 26);
    DataToDlg(m_setup);
    m_Edit[0][0].SetFocus();
    return false;
}

void CDlgSetup::OnChangeDim()
{
    if (m_spinDim.GetSafeHwnd() == NULL) return;
    CString s;
    GetDlgItemText(IDC_DIM, s);
    int dim = StrToInt(s);
    if (dim < 1 || dim > 26) return;
    m_dim = dim;
    Resize();
}

void CDlgSetup::OnKillFocus(UINT idc)
{
    CString s;
    GetDlgItemText(idc, s);
    int set = StrToInt(s);
    if (set <= 0) SetDlgItemText(idc, strEmpty);
}

void CDlgSetup::OnOK()
{
    DlgToData(m_setup);
    EndDialog(IDOK);
}

void CDlgSetup::OnLoad()
{
    CDlgLoadSetup dlg;
    DlgToData(dlg.m_setup);
    if (dlg.DoModal() != IDOK) return;
    DataToDlg(dlg.m_setup);
}

BOOL CDlgSetup::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
    {
        bool bBack = GetKeyState(VK_SHIFT) < 0;
        CWnd* pWnd = GetFocus();
        CWnd* pNext = NULL;
        if (!pWnd) return 1;

        if (!bBack && pWnd == GetDlgItem(IDC_LOAD))
            pNext = &m_Edit[0][0];
        else if (bBack && pWnd == GetDlgItem(IDOK))
            pNext = &m_Edit[m_dim - 1][m_dim - 1];

        if (!pNext)
        {
            int i, j;
            for (i = m_dim; --i >= 0;)
                for (j = m_dim; --j >= 0;)
                    if (pWnd == &m_Edit[i][j]) goto Found;
            goto Break;

        Found:
            if (!bBack)
            {
                if (j < m_dim - 1)
                    pNext = &m_Edit[i][j + 1];
                else if (i < m_dim - 1)
                    pNext = &m_Edit[i + 1][0];
                else
                    pNext = GetDlgItem(IDOK);
            }
            else
            {
                if (j > 0)
                    pNext = &m_Edit[i][j - 1];
                else if (i > 0)
                    pNext = &m_Edit[i - 1][m_dim - 1];
                else
                    pNext = GetDlgItem(IDC_LOAD);
            }
        }

        if (pNext)
        {
            pNext->SetFocus();
            return 1;
        }
    }
Break:
    return super::PreTranslateMessage(pMsg);
}
