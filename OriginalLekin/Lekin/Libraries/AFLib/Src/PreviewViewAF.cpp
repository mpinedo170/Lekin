#include "StdAfx.h"

#include "AFLibPreviewViewAF.h"

#include "AFLibViewEx.h"
#include "AFLibViewTempEx.h"
#include "Local.h"

using AFLibGui::CPreviewViewAF;
using AFLibGui::TViewEx;
using namespace AFLibPrivate;

IMPLEMENT_DYNCREATE(CPreviewViewAF, super)

namespace AFLibPrivate {
CRuntimeClass* const pClassPrintPreview = RUNTIME_CLASS(CPreviewViewAF);
}

CPreviewViewAF::CPreviewViewAF()
{}

BEGIN_MESSAGE_MAP(CPreviewViewAF, super)
ON_WM_TIMER()
ON_WM_DESTROY()
END_MESSAGE_MAP()

TViewEx* CPreviewViewAF::GetViewEx()
{
    return dynamic_cast<TViewEx*>(m_pPrintView);
}

void CPreviewViewAF::UpdatePageValid()
{
    m_bPageValid = true;
    for (UINT page = 0; page < m_nPages; ++page)
    {
        if (!GetViewEx()->ViewExIsPageReady(m_nCurrentPage + page))
        {
            m_bPageValid = false;
            break;
        }
    }
}

void CPreviewViewAF::OnDraw(CDC* pDC)
{
    if (m_pPrintView == NULL || m_dcPrint.m_hDC == NULL) return;

    bool bFinished = GetViewEx()->ViewExHasUpdateFinished();
    if (bFinished) KillTimer(timerUpdate);

    UpdatePageValid();
    super::OnDraw(pDC);

    if (!bFinished) SetTimer(timerUpdate, 50, NULL);
}

void CPreviewViewAF::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerUpdate)
    {
        if (GetViewEx()->ViewExHasUpdateFinished())
        {
            KillTimer(timerUpdate);
            int nPages = GetViewEx()->ViewExGetPageMax();
            int minPage = m_pPreviewInfo->GetMinPage();
            int maxPage = max(nPages, minPage);
            m_pPreviewInfo->SetMaxPage(maxPage);

            if (maxPage < TViewEx::pageMaxHigh && maxPage - minPage < TViewEx::pageMaxHigh)
            {
                SCROLLINFO info;
                info.fMask = SIF_PAGE | SIF_RANGE;
                info.nMin = m_pPreviewInfo->GetMinPage();
                info.nMax = m_pPreviewInfo->GetMaxPage();
                info.nPage = 1;

                if (!SetScrollInfo(SB_VERT, &info, false))
                    SetScrollRange(SB_VERT, info.nMin, info.nMax, false);
            }
            else
                ShowScrollBar(SB_VERT, false);

            SetCurrentPage(m_nCurrentPage, true);
        }

        if (!m_bPageValid)
        {
            UpdatePageValid();
            if (m_bPageValid) Invalidate(true);
        }

        return;
    }

    super::OnTimer(nIDEvent);
}

void CPreviewViewAF::OnDestroy()
{
    KillTimer(timerUpdate);
    super::OnDestroy();
}
