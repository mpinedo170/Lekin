#include "StdAfx.h"

#include "SB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSBDlg dialog

CSBDlg::CSBDlg(CWnd* pParent /*=NULL*/) : CDialog(CSBDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSBDlg)
    //}}AFX_DATA_INIT
}

void CSBDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSBDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSBDlg, CDialog)
//{{AFX_MSG_MAP(CSBDlg)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSBDlg message handlers

BOOL CSBDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    SetIcon(m_hIcon, true);   // Set big icon
    SetIcon(m_hIcon, false);  // Set small icon

    CRect rect;
    GetWindowRect(&rect);
    SetWindowPos(NULL, rect.left, rect.top + GetSystemMetrics(SM_CYSCREEN) / 3, -1, -1,
        SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);
    return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSBDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);  // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
        CDialog::OnPaint();
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSBDlg::OnQueryDragIcon()
{
    return (HCURSOR)m_hIcon;
}
