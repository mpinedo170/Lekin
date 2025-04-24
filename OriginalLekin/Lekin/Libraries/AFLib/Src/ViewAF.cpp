#include "StdAfx.h"

#include "AFLibViewAF.h"

using AFLibGui::CViewAF;

IMPLEMENT_DYNAMIC(CViewAF, super)

CViewAF::CViewAF(UINT nIDTemplate) : super(nIDTemplate)
{
    m_bInitialized = false;
}

BOOL CALLBACK CViewAF::EnumChildProc(HWND hWnd, LPARAM lParam)
{
    CViewAF* pView = reinterpret_cast<CViewAF*>(lParam);
    if (::GetParent(hWnd) != pView->m_hWnd) return true;

    switch (pView->m_act)
    {
        case actHide:
            ::ShowWindow(hWnd, SW_HIDE);
            break;

        case actShow:
            if (pView->IsChildVisible(hWnd)) ::ShowWindow(hWnd, SW_SHOW);
            break;

        case actRedraw:
            if (::IsWindowVisible(hWnd))
                ::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
            break;
    }

    return true;
}

void CViewAF::PrivateResize()
{
    m_act = actHide;
    EnumChildWindows(m_hWnd, EnumChildProc, LPARAM(this));
    OnResize();

    m_act = actShow;
    EnumChildWindows(m_hWnd, EnumChildProc, LPARAM(this));
}

void CViewAF::Resize()
{
    if (!m_bInitialized) return;
    PrivateResize();
    OnUpdateResize();
}

void CViewAF::Update()
{
    OnUpdate(NULL, 0, NULL);
}

void CViewAF::ResizeAndUpdate()
{
    if (!m_bInitialized) return;
    PrivateResize();
    Update();
}

void CViewAF::UpdateSelection()
{
    if (!m_bInitialized) return;
    OnUpdateSelection();
    RedrawAll();
}

void CViewAF::RedrawAll()
{
    if (!m_bInitialized) return;
    m_act = actRedraw;
    EnumChildWindows(m_hWnd, EnumChildProc, LPARAM(this));
}

BEGIN_MESSAGE_MAP(CViewAF, super)
ON_WM_SIZE()
ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrint)
ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_DIRECT, OnUpdateFilePrint)
END_MESSAGE_MAP()

void CViewAF::OnInitialUpdate()
{
    super::OnInitialUpdate();
    OnInit();
    SetScrollSizes(MM_TEXT, CSize(1, 1));
    m_bInitialized = true;
    ResizeAndUpdate();
}

void CViewAF::OnSize(UINT nType, int cx, int cy)
{
    super::OnSize(nType, cx, cy);
    Resize();
}

void CViewAF::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pMargin != NULL);
}

void CViewAF::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    if (!m_bInitialized) return;

    CWaitCursor wc;
    OnUpdateAF();
    OnUpdateResize();
    UpdateSelection();
}

void CViewAF::OnInit()
{}

void CViewAF::OnUpdateAF()
{}

void CViewAF::OnResize()
{}

void CViewAF::OnUpdateResize()
{}

void CViewAF::OnUpdateSelection()
{}

bool CViewAF::IsChildVisible(HWND hWnd)
{
    return true;
}

bool CViewAF::GetMinMaxInfo(MINMAXINFO& MMI)
{
    MMI.ptMinTrackSize = CPoint(250, 250);
    return true;
}
