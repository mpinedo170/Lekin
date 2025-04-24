#include "StdAfx.h"

#include "AFLibDlgTT.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "Local.h"

using namespace AFLibPrivate;
using namespace AFLibMath;
using AFLibGui::CDlgTT;

IMPLEMENT_DYNAMIC(CDlgTT, super)

/////////////////////////////////////////////////////////////////////////////
// CDlgTT::TResize

CDlgTT::TResize::TResize()
{
    m_left = m_top = m_right = m_bottom = 0;
}

CDlgTT::TResize::TResize(TResizeStock rzsX, TResizeStock rzsY)
{
    m_left = rzsX == rzsMove ? 1 : 0;
    m_top = rzsY == rzsMove ? 1 : 0;
    m_right = rzsX == rzsNone ? 0 : 1;
    m_bottom = rzsY == rzsNone ? 0 : 1;
}

CDlgTT::TResize::TResize(double left, double top, double right, double bottom)
{
    m_left = left;
    m_top = top;
    m_right = right;
    m_bottom = bottom;
}

bool CDlgTT::TResize::IsNull() const
{
    return fabs(m_left) + fabs(m_top) + fabs(m_right) + fabs(m_bottom) < 0.01;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgTT

CDlgTT::CDlgTT()
{}

CDlgTT::CDlgTT(UINT nIDTemplate, CWnd* pWndParent) :
    super(nIDTemplate, pWndParent),
    m_szMin(0, 0),
    m_pSzSaved(NULL)
{}

BOOL CALLBACK CDlgTT::EnumChildProc(HWND hWnd, LPARAM lParam)
{
    CDlgTT* pDlg = reinterpret_cast<CDlgTT*>(lParam);
    if (::GetParent(hWnd) != pDlg->m_hWnd) return true;

    UINT idc = ::GetDlgCtrlID(hWnd);
    if (idc == IDC_STATIC) return true;

    TResize rz = pDlg->GetResizeMask(idc);
    if (rz.IsNull()) return true;

    TPrivateResize qrz = {idc, rz, false};
    pDlg->m_arrResize.Add(qrz);
    return true;
}

void CDlgTT::ResizeToContents(CWnd* pWndContents)
{
    CRect rectWin;
    GetWindowRect(rectWin);
    CRect rectCli;
    GetClientRect(rectCli);
    CRect rectCont;
    pWndContents->GetWindowRect(rectCont);

    rectWin.InflateRect((rectCont.Width() - rectCli.Width()) / 2, (rectCont.Height() - rectCli.Height()) / 2);
    MoveWindow(rectWin);
}

bool CDlgTT::StartAutoResize(CSize* pSzSaved)
{
    m_pSzSaved = pSzSaved;
    if (!m_arrResize.IsEmpty()) return false;

    EnumChildWindows(m_hWnd, EnumChildProc, LPARAM(this));

    CRect rect;
    GetWindowRect(rect);
    m_szMin = rect.Size();

    for (int i = 0; i < m_arrResize.GetSize(); ++i)
    {
        TPrivateResize& qrz = m_arrResize[i];
        GetDlgItem(qrz.m_idc)->GetWindowRect(qrz.m_rectOrig);
        ScreenToClient(&qrz.m_rectOrig);
    }

    if (pSzSaved != NULL && pSzSaved->cx != 0)
    {
        int cxScreen = GetSystemMetrics(SM_CXSCREEN);
        int cyScreen = GetSystemMetrics(SM_CYSCREEN);
        int cx = min(int(max(m_szMin.cx, pSzSaved->cx)), cxScreen);
        int cy = min(int(max(m_szMin.cy, pSzSaved->cy)), cyScreen);

        MoveWindow((cxScreen - cx) / 2, (cyScreen - cy) / 2, cx, cy);
    }

    CreateDlgIcon();
    return true;
}

CDlgTT::TResize CDlgTT::GetResizeMask(UINT idc)
{
    return TResize();
}

void CDlgTT::CustomResize(int cx, int cy)
{}

namespace {
BOOL CALLBACK IconProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    HICON hIcon = ::LoadIcon(hModule, lpszName);
    if (hIcon == NULL) return true;

    *reinterpret_cast<LPTSTR*>(lParam) = lpszName;
    return false;
}
}  // namespace

void CDlgTT::CreateDlgIcon()
{
    LPTSTR lpszName = NULL;
    EnumResourceNames(
        AfxGetApp()->m_hInstance, RT_GROUP_ICON, IconProc, reinterpret_cast<LONG_PTR>(&lpszName));
    if (lpszName == NULL) return;

    HANDLE hBig = ::LoadImage(AfxGetApp()->m_hInstance, lpszName, IMAGE_ICON, GetSystemMetrics(SM_CXICON),
        GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    SetIcon(HICON(hBig), true);

    HANDLE hSmall = ::LoadImage(AfxGetApp()->m_hInstance, lpszName, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    SetIcon(HICON(hSmall), false);
}

BEGIN_MESSAGE_MAP(CDlgTT, super)
ON_WM_CREATE()
ON_WM_SIZE()
ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

int CDlgTT::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int r = super::OnCreate(lpCreateStruct);
    EnableToolTips();
    return r;
}

void CDlgTT::OnSize(UINT nType, int cx, int cy)
{
    super::OnSize(nType, cx, cy);
    if (m_szMin.cx == 0 || m_arrResize.IsEmpty()) return;

    CRect rect;
    GetWindowRect(rect);
    if (m_pSzSaved != NULL) *m_pSzSaved = rect.Size();

    CSize szShift(rect.Width() - m_szMin.cx, rect.Height() - m_szMin.cy);

    for (int i = 0; i < m_arrResize.GetSize(); ++i)
    {
        TPrivateResize& qrz = m_arrResize[i];
        qrz.m_bVisible = GetDlgItem(qrz.m_idc)->IsWindowVisible() != 0;
    }

    for (int i = 0; i < m_arrResize.GetSize(); ++i)
    {
        TPrivateResize& qrz = m_arrResize[i];

        rect = qrz.m_rectOrig;
        rect.left += Rint(qrz.m_rz.m_left * szShift.cx);
        rect.top += Rint(qrz.m_rz.m_top * szShift.cy);
        rect.right += Rint(qrz.m_rz.m_right * szShift.cx);
        rect.bottom += Rint(qrz.m_rz.m_bottom * szShift.cy);

        AndrewMoveWindow(GetDlgItem(qrz.m_idc), rect, false);
    }

    CustomResize(cx, cy);
    RedrawWindow();
}

void CDlgTT::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
    super::OnGetMinMaxInfo(lpMMI);
    lpMMI->ptMinTrackSize = CPoint(m_szMin);
}
