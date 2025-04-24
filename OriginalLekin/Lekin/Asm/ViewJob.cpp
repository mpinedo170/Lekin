#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewJob.h"

#include "DlgAddJob.h"
#include "DlgAddOper.h"

#include "Draws.h"
#include "Job.h"
#include "Misc.h"
#include "Schedule.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CViewJob

IMPLEMENT_DYNCREATE(CViewJob, CViewTree)

CViewJob::CViewJob()
{}

BEGIN_MESSAGE_MAP(CViewJob, super)
ON_COMMAND(ID_JOB_ADD, OnJobAdd)
ON_COMMAND(ID_JOB_DEL, OnJobDel)
ON_COMMAND(ID_JOB_EDIT, OnJobEdit)
ON_COMMAND(ID_OPER_ADD, OnOperAdd)
ON_COMMAND(ID_OPER_EDIT, OnOperEdit)
ON_COMMAND(ID_OPER_DEL, OnOperDel)
ON_UPDATE_COMMAND_UI(ID_JOB_ADD, OnUpdateJobAdd)
ON_UPDATE_COMMAND_UI(ID_JOB_EDIT, OnUpdateJobEdit)
ON_UPDATE_COMMAND_UI(ID_JOB_DEL, OnUpdateJobDel)
ON_UPDATE_COMMAND_UI(ID_OPER_ADD, OnUpdateOperAdd)
ON_UPDATE_COMMAND_UI(ID_OPER_EDIT, OnUpdateOperEdit)
ON_UPDATE_COMMAND_UI(ID_OPER_DEL, OnUpdateOperDel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewJob overridables

void CViewJob::Fill()
{
    CString titles[11] = {_T("ID"), _T("Wght"), _T("Rls"), _T("Due"), _T("Pr.tm."), _T("Stat."), _T("Bgn"),
        _T("End"), _T("T"), _T("wT"), strEmpty};

    int widths[11] = {10, 0};
    m_bSingleLevel = TJob::GetShop() == OneShop;

    for (int i = 1; i < 10; ++i) widths[i] = 5;

    if (pSchActive == NULL) titles[6].Empty();

    SetTitles(titles, widths);
    TJobArray& arrJobTemp = theApp.GetJobList();

    m_maxProc = 1;
    for (int i = 0; i < arrJobTemp.GetSize(); ++i)
    {
        TJob* pJob = arrJobTemp[i];
        AddParent(pJob);

        for (int j = 0; j < pJob->GetOpCount(); ++j)
        {
            TOperation* pOp = pJob->m_arrOp[j];
            m_maxProc = max(m_maxProc, pOp->m_proc);
            if (TJob::GetShop() != OneShop) AddChild(pOp);
        }
    }
}

bool CViewJob::IsCellValid(TElemDraw& draw)
{
    if (!draw.m_bParent) return true;
    TJob* pJob = (TJob*)(draw.m_data);
    return pJob && pJob->GetOpCount() > 0;
}

CString CViewJob::GetCellString(TElemDraw& draw)
{
    int cell = draw.m_cell;

    if (draw.m_bParent)
    {
        TJob* pJob = (TJob*)(draw.m_data);
        switch (cell)
        {
            case 0:
                return pJob->m_id;
            case 1:
                return IntToStr(pJob->m_weight);
            case 2:
                return IntToStr(pJob->m_release);
            case 3:
                return IntToStr(pJob->m_due);
            case 4:
                return IntToStr(pJob->GetProcTime());
            case 5:
                return CString(pJob->m_arrOp[0]->m_status);
            case 6:
                return IntToStr(pJob->GetStart());
            case 7:
                return IntToStr(pJob->GetStop());
            case 8:
                return IntToStr(pJob->GetT());
            case 9:
                return IntToStr(pJob->GetWT());
        }
    }
    else
    {
        TOperation* pOp = (TOperation*)(draw.m_data);
        switch (cell)
        {
            case 4:
                return IntToStr(pOp->m_proc);
            case 5:
                return CString(pOp->m_status);
            case 6:
                return IntToStr(pOp->GetStart());
            case 7:
                return IntToStr(pOp->GetStop());
        }
    }
    return strEmpty;
}

CViewTree::TDrawCell CViewJob::NeedDrawCell(TElemDraw& draw)
{
    if (TJob::GetShop() == OneShop) return ndcYes;
    if (!draw.m_bParent)
    {
        switch (draw.m_cell)
        {
            case 0:
            case 8:
            case 9:
                return ndcNo;
            case 1:
            case 2:
                return ndcMiss;
        }
        return ndcYes;
    }

    return draw.m_cell != 5 ? ndcYes : ndcNo;
}

void CViewJob::GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font)
{
    if (!draw.m_bParent && draw.m_cell == 3)
        theApp.CreateAppFont(font, pDC, 1, true, false);
    else
        super::GetCellFont(pDC, draw, font);
}

void CViewJob::OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw)
{
    if (!draw.m_bParent && draw.m_cell == 3)
    {
        TOperation* pOp = (TOperation*)(draw.m_data);
        CRect rect2(rect), rect3(rect);
        rect2.right = rect2.left + rect2.Width() * pOp->m_proc / m_maxProc;
        rect3.left = rect2.right;

        TID* style = pOp->GetStyle();
        style->DrawRect(pDC, rect2, strEmpty);

        int ext = int(pDC->GetTextExtent(pOp->m_idWkc).cx * 1.2);
        bool bRect3 = false;
        CRect textRect(rect2);

        if (ext > rect2.Width())
        {
            pDC->SetTextAlign(TA_TOP | TA_LEFT);
            if (rect3.Width() > rect2.Width()) bRect3 = true;
        }

        if (bRect3)
            textRect = rect3;
        else
            pDC->SetTextColor(style->GetTextColor(false));

        textRect.DeflateRect(2, 0);
        TextInRect(pDC, textRect, pOp->m_idWkc);
    }
    else if (draw.m_bParent && draw.m_cell == 0)
        ((TJob*)draw.m_data)->DrawRect(pDC, rect);
    else
        super::OnDrawCell(pDC, rect, draw);
}

void CViewJob::DblClickElem(UINT nFlags, int elemNo)
{
    if (pSchActive)
    {
        theApp.OpenDlgJob();
        ClickElem(nFlags, elemNo);
        return;
    }

    switch (WhatSelected())
    {
        case 1:
            OnJobEdit();
            break;
        case 0:
            if (GetSelCount() == 1)
            {
                OnOperEdit();
                break;
            }
        default:
            LekinBeep();
            break;
    }
}

void CViewJob::OnDelete()
{
    switch (WhatSelected())
    {
        case 1:
            OnJobDel();
            break;
        case 0:
            if (TJob::GetShop() == JobShop)
            {
                OnOperDel();
                break;
            }
        default:
            LekinBeep();
            break;
    }
}

void CViewJob::ClickElem(UINT nFlags, int elemNo)
{
    TElement* elem = m_elements[elemNo];
    TOperation* pOp = NULL;
    if (elem->IsParent())
        pOp = ((TJob*)elem->m_data)->m_arrOp[0];
    else
        pOp = (TOperation*)elem->m_data;
    theApp.UpdateFloaters(pOp);
}

/////////////////////////////////////////////////////////////////////////////
// CViewJob message handlers

void CViewJob::OnJobAdd()
{
    if (arrJob.GetSize() >= MaxJob)
    {
        AfxMB(IDP_TOO_MANY, IntToStr(MaxJob), _T("arrJob"));
        return;
    }
    if (!theApp.CheckSequences(true)) return;

    CDlgAddJob dlg;
    dlg.LoadData();

    if (arrJob.GetSize() > 0)
    {
        TJob* pJob = arrJob[arrJob.GetSize() - 1];
        dlg.JobID() = pJob->m_id;
        AlterName(dlg.JobID());
        dlg.m_route.Load(*pJob);
    }

    if (!dlg.m_route.IsValid()) dlg.m_route.CreateDefault();

    if (dlg.DoModal() != IDOK) return;
    TID style = dlg.m_btnStyle.m_style;

    for (int i = 0; i < dlg.m_jobCount; ++i)
    {
        TJob* pJob = new TJob;
        dlg.ReadJob(*pJob);
        arrJob.AlterName(pJob->m_id);
        if (dlg.m_jobCount > 1) pJob->RandomColor();
        arrJob.Add(pJob);
    }

    SetModifiedFlag2();
}

void CViewJob::OnJobDel()
{
    if (!theApp.AskConfirmDel(IDS_JOB)) return;
    if (!theApp.CheckSequences(true)) return;

    for (int i = GetCount(); --i >= 0;)
        if (m_elements[i]->m_bSelected)
        {
            TJob* pJob = (TJob*)m_elements[i]->m_data;
            arrJob.DestroyExact(pJob);
        }

    SetModifiedFlag2();
}

void CViewJob::OnJobEdit()
{
    if (!BeepWkc()) return;
    if (!theApp.CheckSequences(true)) return;

    TJobArray arrJob;
    GetSelArray(*(TCollection*)&arrJob);
    theApp.EditJob(arrJob);
}

void CViewJob::OnOperAdd()
{
    if (!theApp.CheckSequences(true)) return;

    CDlgAddOper dlg;
    TElement* elem = m_elements[LastSelected()];
    TJob* pJob = NULL;
    if (elem->IsParent())
    {
        pJob = (TJob*)(elem->m_data);
        dlg.m_number = 1;
    }
    else
    {
        TOperation* pOp = (TOperation*)(elem->m_data);
        GetParent(elem);
        pJob = (TJob*)(elem->m_data);
        dlg.m_number = pJob->m_arrOp.FindExact(pOp) + 2;
    }
    dlg.m_pJob = pJob;
    if (dlg.DoModal() != IDOK) return;
    SetModifiedFlag2();
}

void CViewJob::OnOperEdit()
{
    if (!BeepWkc()) return;
    if (!theApp.CheckSequences(true)) return;

    CDlgAddOper dlg;
    dlg.m_pOp = (TOperation*)(m_elements[LastSelected()]->m_data);
    if (dlg.DoModal() != IDOK) return;
    SetModifiedFlag2();
}

void CViewJob::OnOperDel()
{
    if (!theApp.AskConfirmDel(IDS_OPER)) return;
    if (!theApp.CheckSequences(true)) return;

    for (int i = GetCount(); --i >= 0;)
        if (m_elements[i]->m_bSelected)
        {
            TOperation* pOp = (TOperation*)m_elements[i]->m_data;
            pOp->DestroyThis();
        }

    DeleteEmptyJobs();
    SetModifiedFlag2();
}

void CViewJob::OnUpdateJobAdd(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GotWkc());
}

void CViewJob::OnUpdateJobEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GotWkc() && WhatSelected() == 1);
}

void CViewJob::OnUpdateJobDel(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(WhatSelected() == 1);
}

void CViewJob::OnUpdateOperDel(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TJob::GetShop() == JobShop && WhatSelected() == 0);
}

void CViewJob::OnUpdateOperEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GotWkc() && WhatSelected() == 0 && GetSelCount() == 1);
}

void CViewJob::OnUpdateOperAdd(CCmdUI* pCmdUI)
{
    int res = false;
    if (GotWkc() && TJob::GetShop() == JobShop && GetSelCount() == 1)
    {
        TElement* elem = m_elements[LastSelected()];
        GetParent(elem);
        TJob* pJob = (TJob*)(elem->m_data);
        res = pJob->GetOpCount() < arrWorkcenter.GetSize();
    }
    pCmdUI->Enable(res);
}
