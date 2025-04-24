#include "StdAfx.h"

#include "WndDragHelper.h"

#include "AFLibDragField.h"
#include "AFLibStrings.h"
#include "Local.h"

using namespace AFLib;
using AFLibGui::TDragField;
using AFLibPrivate::CWndDragHelper;

CWndDragHelper::CWndDragHelper(TDragField* pDf, int ms)
{
    m_pDf = pDf;
    Create(NULL, strEmpty, WS_CHILD, CRect(0, 0, 10, 10), AfxGetApp()->GetMainWnd(), 1);
    if (ms != 0) SetTimer(timerUpdate, ms, NULL);
    SetCapture();
}

BEGIN_MESSAGE_MAP(CWndDragHelper, super)
ON_WM_CAPTURECHANGED()
ON_WM_DESTROY()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONUP()
ON_WM_TIMER()
END_MESSAGE_MAP()

void CWndDragHelper::OnCaptureChanged(CWnd* pWnd)
{
    m_pDf->CancelDrag();
    super::OnCaptureChanged(pWnd);
}

void CWndDragHelper::OnDestroy()
{
    m_pDf->CancelDrag();
    super::OnDestroy();
}

void CWndDragHelper::OnMouseMove(UINT nFlags, CPoint point)
{
    ClientToScreen(&point);
    m_pDf->DragMouseMove(nFlags, point);
}

void CWndDragHelper::OnLButtonUp(UINT nFlags, CPoint point)
{
    ClientToScreen(&point);
    m_pDf->DragLButtonUp(nFlags, point);
}

void CWndDragHelper::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerUpdate) m_pDf->DragTimer();
}
