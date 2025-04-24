#include "StdAfx.h"

#include "AppAsm.h"
#include "LstSeq.h"

#include "Draws.h"
#include "Operation.h"
#include "Sequence.h"
#include "Workcenter.h"

void CLstSeq::Draw(CDC* pDC)
{
    TOperation* pOp = (TOperation*)m_draw.m_itemData;
    if (pOp == NULL) return;

    TMachine* pMch = pOp->m_tm.m_pSeq->m_pMch;
    CRect rect(m_draw.m_rect);
    rect.DeflateRect(2, 2);

    rect.right = m_arrTab[0];
    pDC->SetTextAlign(TA_TOP + TA_LEFT);
    pMch->m_pWkc->DrawRect(pDC, rect, pMch->m_id);
    pDC->SetTextAlign(TA_TOP + TA_CENTER);

    int dd[] = {0, pOp->m_tm.m_setup, pOp->GetStart(), pOp->GetStop(), pOp->m_proc};

    for (int i = 1;; ++i)
    {
        rect.left = rect.right;
        rect.right += m_arrTab[i];
        if (i == 5) break;

        CString s = IntToStr(dd[i]);
        if (i == 1) s += _T(" (") + pOp->GetStatString() + _T(")");
        TextInRect(pDC, rect, s);
    }

    int rel = pOp->m_tm.m_rel;
    if (rel > 0) TextInRect(pDC, rect, IntToStr(rel));
}

int CLstSeq::Measure(int index)
{
    return super::Measure(index) + 2;
}
