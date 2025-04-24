#include "StdAfx.h"

#include "AFLibFrmAF.h"

#include "AFLibGlobal.h"
#include "AFLibGuiGlobal.h"
#include "AFLibViewAF.h"
#include "AFLibViewEx.h"
#include "Local.h"

using AFLibGui::CFrmAF;
using namespace AFLib;
using namespace AFLibPrivate;

IMPLEMENT_DYNCREATE(CFrmAF, super)

CFrmAF::CFrmAF()
{
    m_pView = NULL;
}

BOOL CFrmAF::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= WS_CLIPCHILDREN;
    return super::PreCreateWindow(cs);
}

void CFrmAF::UpdateTitle()
{
    OnUpdateFrameTitle(true);
}

void CFrmAF::CallCloseProc()
{
    if (m_lpfnCloseProc != NULL) (*m_lpfnCloseProc)(this);
}

bool CFrmAF::PrivateProcessGetMinMaxInfo(MINMAXINFO& MMI)
{
    if (!GetMinMaxInfo(MMI)) return false;

    CViewAF* pViewAF = GetView();
    if (pViewAF == NULL) return false;

    MINMAXINFO MMIView = {0};
    MMIView.ptMaxSize = CPoint(MXI, MXI);
    MMIView.ptMaxTrackSize = CPoint(MXI, MXI);
    if (!pViewAF->GetMinMaxInfo(MMIView)) return false;

    CRect rectCliFrm;
    GetClientRect(rectCliFrm);
    CRect rectCliView;
    pViewAF->GetClientRect(rectCliView);
    CSize sz = rectCliFrm.Size() - rectCliView.Size();

    sz.cx += 2 * GetSystemMetrics(SM_CXFRAME);
    sz.cy += 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);

    MMI.ptMinTrackSize.x = max(MMI.ptMinTrackSize.x, MMIView.ptMinTrackSize.x + sz.cx);
    MMI.ptMinTrackSize.y = max(MMI.ptMinTrackSize.y, MMIView.ptMinTrackSize.y + sz.cy);
    MMI.ptMaxTrackSize.x = min(MMI.ptMaxTrackSize.x, MMIView.ptMaxTrackSize.x + sz.cx);
    MMI.ptMaxTrackSize.y = min(MMI.ptMaxTrackSize.y, MMIView.ptMaxTrackSize.y + sz.cy);
    MMI.ptMaxSize.x = min(MMI.ptMaxSize.x, MMIView.ptMaxSize.x + sz.cx);
    MMI.ptMaxSize.y = min(MMI.ptMaxSize.y, MMIView.ptMaxSize.y + sz.cy);

    return true;
}

bool CFrmAF::GetMinMaxInfo(MINMAXINFO& MMI)
{
    return true;
}

AFLibGui::CViewAF* CFrmAF::GetView()
{
    if (m_pView == NULL) m_pView = dynamic_cast<CViewAF*>(GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE));
    return m_pView;
}

void CFrmAF::Activate()
{
    GetView()->Update();
    if (IsIconic()) ShowWindow(SW_RESTORE);
    ActivateFrame();
}

BEGIN_MESSAGE_MAP(CFrmAF, super)
ON_WM_GETMINMAXINFO()
ON_WM_TIMER()
END_MESSAGE_MAP()

void CFrmAF::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
    super::OnGetMinMaxInfo(lpMMI);
    if (!PrivateProcessGetMinMaxInfo(*lpMMI)) PostMessage(WM_TIMER, timerUpdate);
}

CDocument* CFrmAF::GetActiveDocument()
{
    return GetView()->GetDocument();
}

void CFrmAF::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerUpdate)
    {
        CRect rect;
        GetWindowRect(rect);
        GetParent()->ScreenToClient(rect);
        AndrewMoveWindow(this, rect);
    }

    super::OnTimer(nIDEvent);
}
