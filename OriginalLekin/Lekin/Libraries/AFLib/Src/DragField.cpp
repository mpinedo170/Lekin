#include "StdAfx.h"

#include "AFLibDragField.h"

#include "AFLibLstOwner.h"
#include "AFLibResource.h"
#include "Local.h"
#include "WndDragHelper.h"

using AFLibGui::TDragField;
using namespace AFLibPrivate;

TDragField::TDragField()
{
    m_pWndHelper = NULL;
    m_pWndSource = NULL;
    m_hCurDrag = m_hCurStop = NULL;
    m_timerMS = 0;
}

TDragField::~TDragField()
{
    CancelDrag();
}

void TDragField::AddCompanion(CWnd* pWnd)
{
    if (m_arrCom.FindExact(pWnd) >= 0) return;
    m_arrCom.Add(pWnd);
}

void TDragField::RemoveCompanion(CWnd* pWnd)
{
    m_arrCom.RemoveExact(pWnd);
}

int TDragField::GetDragItemCount(CWnd* pWndSource)
{
    CLstOwner* pLst = dynamic_cast<CLstOwner*>(pWndSource);
    if (pLst == NULL) return 1;
    return pLst->GetSelCount();
}

CWnd* TDragField::FindPoint(const CPoint& point)
{
    for (int i = 0; i < GetCount(); ++i)
    {
        CWnd* pWnd = GetCompanion(i);
        if (!pWnd->IsWindowVisible()) continue;

        CRect rect;
        pWnd->GetWindowRect(rect);
        if (rect.PtInRect(point)) return pWnd;
    }
    return NULL;
}

void TDragField::StartDrag(CWnd* pWndSource)
{
    if (InDrag()) return;

    int itemCount = GetDragItemCount(pWndSource);
    if (itemCount == 0) return;

    m_pWndSource = pWndSource;
    m_hCurStop = LoadCursor(NULL, IDC_NO);
    m_hCurDrag = LoadCursor(AFLibDLL.hModule, MAKEINTRESOURCE(itemCount == 1 ? IDU_MOVE_ONE : IDU_MOVE_MANY));
    m_pWndHelper = new CWndDragHelper(this, m_timerMS);
}

void TDragField::StopDrag(CWnd* pWndDest)
{
    if (!InDrag()) return;
    CWnd* pWndSource = m_pWndSource;
    CancelDrag();
    Drop(pWndSource, pWndDest);
}

void TDragField::CancelDrag()
{
    if (!InDrag()) return;
    m_pWndHelper->KillTimer(timerUpdate);

    CWnd* pWndHelper = m_pWndHelper;
    m_pWndHelper = NULL;
    m_pWndSource = NULL;

    SetCursor(LoadCursor(NULL, IDC_ARROW));
    ReleaseCapture();

    if (pWndHelper->GetSafeHwnd() != NULL) pWndHelper->DestroyWindow();
    delete pWndHelper;
}

void TDragField::Drop(CWnd* pWndSource, CWnd* pWndDest)
{}

void TDragField::DragMouseMove(UINT nFlags, const CPoint& point)
{
    if (!InDrag()) return;
    CWnd* pWndDest = FindPoint(point);
    SetCursor(pWndDest == NULL ? m_hCurStop : m_hCurDrag);
}

void TDragField::DragLButtonUp(UINT nFlags, const CPoint& point)
{
    if (!InDrag()) return;
    CWnd* pWndDest = FindPoint(point);
    if (pWndDest != NULL)
        StopDrag(pWndDest);
    else
        CancelDrag();
}

void TDragField::DragTimer()
{}
