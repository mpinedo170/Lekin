#include "StdAfx.h"

#include "AppAsm.h"
#include "LstLoadRoute.h"

#include "Draws.h"
#include "Job.h"
#include "Route.h"

/////////////////////////////////////////////////////////////////////////////
// CLstLoadRoute

void CLstLoadRoute::OnInit()
{
    super::OnInit();
    m_total = 1;
}

void CLstLoadRoute::Draw(CDC* pDC)
{
    TJob* pJob = (TJob*)m_draw.m_itemData;
    if (!pJob) return;

    int proc = pJob->GetProcTime();
    CRect rect(m_draw.m_rect);
    rect.DeflateRect(2, 2);
    rect.right = m_arrTab[0];

    pDC->SetTextAlign(TA_TOP + TA_LEFT);
    pJob->DrawRect(pDC, rect);
    pDC->SetTextAlign(TA_TOP + TA_CENTER);

    rect.left = rect.right;
    rect.right += m_arrTab[1];
    TextInRect(pDC, rect, IntToStr(proc));

    rect.left = rect.right;
    rect.right += m_arrTab[2] * proc / m_total;
    TRoute route;
    route.Load(*pJob);
    route.DrawRect(pDC, rect);
}

int CLstLoadRoute::Measure(int index)
{
    return super::Measure(index) + 4;
}
