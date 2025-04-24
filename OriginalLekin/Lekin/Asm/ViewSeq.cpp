#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewSeq.h"

#include "DocTmpSmart.h"
#include "ViewMch.h"

#include "Draws.h"
#include "Job.h"
#include "Schedule.h"
#include "Sequence.h"
#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CViewSeq

IMPLEMENT_DYNCREATE(CViewSeq, CViewTree)

CViewSeq::CViewSeq()
{
    CString titles[] = {_T("Mch/Job"), _T("Setup"), _T("Start"), _T("Stop"), _T("Pr.tm"), strEmpty};
    int widths[] = {10, 5, 5, 5, 5, 0};

    SetTitles(titles, widths);
    m_bSingleSel = true;
    m_bAltReset = true;
}

BEGIN_MESSAGE_MAP(CViewSeq, super)
END_MESSAGE_MAP()

bool CViewSeq::IsSpecial(LPVOID data)
{
    return DWORD(data) >= 0x100 && DWORD(data) <= 0x100 + objN;
}

bool CViewSeq::IsSpecial(TElemDraw& draw)
{
    return IsSpecial(draw.m_data);
}

/////////////////////////////////////////////////////////////////////////////
// CViewSeq overridables

void CViewSeq::Fill()
{
    if (pSchActive == NULL) return;

    for (int i = 0; i < pSchActive->m_arrSequence.GetSize(); ++i)
    {
        TSequence* pSeq = pSchActive->m_arrSequence[i];
        AddParent(pSeq);
        for (int j = 0; j < pSeq->GetOpCount(); ++j) AddChild(pSeq->m_arrOp[j]);
    }

    AddParent(LPVOID(0x100 + objN));
    for (int i = 0x100; i < 0x100 + objN; ++i) AddChild(LPVOID(i));
}

CString CViewSeq::GetCellString(TElemDraw& draw)
{
    if (IsSpecial(draw))
    {
        int d = DWORD(draw.m_data) & 0xFF;
        return draw.m_cell == 4 ? draw.m_bParent ? _T("Summary") : IntToStr(pSchActive->m_arrObj[d])
                                : strEmpty;
    }

    if (draw.m_bParent)
    {
        TSequence* pSeq = (TSequence*)(draw.m_data);
        switch (draw.m_cell)
        {
            case 0:
                return pSeq->m_pMch->m_id;
            case 1:
            {
                int s = 0;
                for (int i = 0; i < pSeq->GetOpCount(); ++i) s += pSeq->m_arrOp[i]->m_tm.m_setup;
                return IntToStr(s);
            }
            case 4:
            {
                int s = 0;
                for (int i = 0; i < pSeq->GetOpCount(); ++i) s += pSeq->m_arrOp[i]->m_proc;
                return IntToStr(s);
            }
        }
    }
    else
    {
        TOperation* pOp = (TOperation*)(draw.m_data);
        switch (draw.m_cell)
        {
            case 0:
                return pOp->m_pJob->m_id;
            case 1:
                return IntToStr(pOp->m_tm.m_setup);
            case 2:
                return IntToStr(pOp->GetStart());
            case 3:
                return IntToStr(pOp->GetStop());
            case 4:
                return IntToStr(pOp->m_proc);
        }
    }
    return strEmpty;
}

CViewTree::TDrawCell CViewSeq::NeedDrawCell(TElemDraw& draw)
{
    if (IsSpecial(draw))
    {
        if (draw.m_cell == 0 && !draw.m_bParent) return ndcYes;
        if (draw.m_cell == 4) return ndcYes;
        return ndcMiss;
    }
    return draw.m_bParent && (draw.m_cell == 2 || draw.m_cell == 3) ? ndcNo : ndcYes;
}

void CViewSeq::GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font)
{
    super::GetCellFont(pDC, draw, font);
}

void CViewSeq::OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw)
{
    if (IsSpecial(draw) && !draw.m_bParent && draw.m_cell == 0)
    {
        CRect rect2 = rect;
        rect2.left += rect2.Width() / 20;
        DrawFancy(pDC, rect2, TObjective(DWORD(draw.m_data) & 0xFF), true);
        return;
    }

    if (!IsSpecial(draw) && draw.m_cell == 0)
    {
        if (draw.m_bParent)
        {
            TSequence* pSeq = (TSequence*)draw.m_data;
            pSeq->m_pMch->m_pWkc->DrawRect(pDC, rect, pSeq->m_pMch->m_id);
        }
        else
        {
            TOperation* pOp = (TOperation*)draw.m_data;
            pOp->m_pJob->DrawRect(pDC, rect);
        }
        return;
    }
    super::OnDrawCell(pDC, rect, draw);
}

void CViewSeq::DblClickElem(UINT nFlags, int elemNo)
{
    TElement* elem = m_elements[elemNo];

    if (IsSpecial(elem->m_data))
    {
        theApp.OnToolPrf();
        return;
    }

    if (!elem->IsParent())
    {
        theApp.OpenDlgJob();
        ClickElem(nFlags, elemNo);
        return;
    }

    TMachine* pMch = ((TSequence*)elem->m_data)->m_pMch;
    if (TWorkcenter::IsFlexible())
    {
        TMachineArray arrMch;
        arrMch.Add(pMch);
        theApp.EditMch(arrMch);
    }
    else
    {
        TWorkcenterArray arrWkc;
        arrWkc.Add(pMch->m_pWkc);
        theApp.EditWkc(arrWkc);
    }
}

void CViewSeq::ClickElem(UINT nFlags, int elemNo)
{
    TElement* elem = m_elements[elemNo];
    if (elem->IsParent() || IsSpecial(elem->m_data)) return;
    theApp.UpdateFloaters((TOperation*)elem->m_data);
}

/////////////////////////////////////////////////////////////////////////////
// CViewSeq message handlers
