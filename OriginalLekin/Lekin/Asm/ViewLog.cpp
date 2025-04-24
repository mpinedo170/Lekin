#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewLog.h"

#include "DocTmpSmart.h"

#include "Draws.h"
#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CViewLog

IMPLEMENT_DYNCREATE(CViewLog, CViewTree)

CViewLog::CViewLog()
{
    CString titles[10] = {_T("Schedule"), _T("Time")};
    int widths[10] = {10, 6};

    for (int i = 2; i <= objN; ++i)
    {
        titles[i] = strSpace;
        widths[i] = 6;
    }

    SetTitles(titles, widths);
    m_bSingleSel = true;
    m_bSingleLevel = true;
}

CDocTmpSmart* CViewLog::GetFileTmp()
{
    return theApp.m_pTmpSeq;
}

BEGIN_MESSAGE_MAP(CViewLog, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewLog message handlers

void CViewLog::Fill()
{
    if (!pSchActive) return;
    m_selectedSet.Clear();
    m_selectedSet.Include(pSchActive);

    TScheduleArray& arrSchTemp = theApp.GetSchList();
    for (int i = 0; i < arrSchTemp.GetSize(); ++i) AddParent(arrSchTemp[i]);
}

void CViewLog::OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw)
{
    if (draw.m_cell == 0)
        ((TSchedule*)draw.m_data)->DrawRect(pDC, rect);
    else
        super::OnDrawCell(pDC, rect, draw);
}

void CViewLog::OnDrawHeader(CDC* pDC, const CRect& rect, int cell)
{
    if (cell < 2)
    {
        super::OnDrawHeader(pDC, rect, cell);
        return;
    }

    CRect rect2(rect);
    rect2.DeflateRect(0, 2);
    DrawFancy(pDC, rect2, TObjective(cell - 1), false);
}

void CViewLog::GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font)
{
    theApp.CreateAppFont(font, pDC, 1.1);
}

CString CViewLog::GetCellString(TElemDraw& draw)
{
    if (draw.m_cell == 0) return strEmpty;
    TSchedule* pSch = (TSchedule*)draw.m_data;
    return IntToStr(pSch->m_arrObj[draw.m_cell - 1]);
}

void CViewLog::ClickElem(UINT nFlags, int elemNo)
{
    TSchedule* pSch = (TSchedule*)m_elements[elemNo]->m_data;
    if (pSch == pSchActive) return;
    theApp.SetSchActive(pSch, false);
}

void CViewLog::DblClickElem(UINT nFlags, int elemNo)
{
    AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_LOG_EDIT);
}

void CViewLog::OnDelete()
{
    AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_LOG_DEL);
}
