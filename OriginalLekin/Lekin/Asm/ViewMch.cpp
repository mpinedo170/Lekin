#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewMch.h"

#include "Dlg3Button.h"
#include "DlgAddMch.h"
#include "DlgAddWkc.h"
#include "DocMch.h"
#include "DocTmpSmart.h"

#include "Job.h"
#include "Misc.h"
#include "Workcenter.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CViewMch

IMPLEMENT_DYNCREATE(CViewMch, CViewTree)

CViewMch::CViewMch()
{
    CString titles[] = {_T("ID"), _T("MCs"), _T("Avail"), _T("Status"), strEmpty};
    int widths[] = {10, 3, 5, 5, 0};

    SetTitles(titles, widths);
}

BEGIN_MESSAGE_MAP(CViewMch, super)
ON_COMMAND(ID_WKC_ADD, OnWkcAdd)
ON_COMMAND(ID_WKC_DEL, OnWkcDel)
ON_COMMAND(ID_WKC_EDIT, OnWkcEdit)
ON_UPDATE_COMMAND_UI(ID_WKC_EDIT, OnUpdateWkcEdit)
ON_UPDATE_COMMAND_UI(ID_WKC_DEL, OnUpdateWkcDel)
ON_COMMAND(ID_MCH_ADD, OnMchAdd)
ON_COMMAND(ID_MCH_DEL, OnMchDel)
ON_COMMAND(ID_MCH_EDIT, OnMchEdit)
ON_UPDATE_COMMAND_UI(ID_MCH_DEL, OnUpdateMchOk)
ON_UPDATE_COMMAND_UI(ID_MCH_ADD, OnUpdateMchAdd)
ON_UPDATE_COMMAND_UI(ID_MCH_EDIT, OnUpdateMchOk)
ON_UPDATE_COMMAND_UI(ID_WKC_ADD, OnUpdateWkcAdd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewMch overridables

namespace {
int Sort(TWorkcenter* pWkc1, TWorkcenter* pWkc2)
{
    return _tcsicmp(pWkc1->m_id, pWkc2->m_id);
}
}  // namespace

void CViewMch::Fill()
{
    m_bSingleLevel = !TWorkcenter::IsFlexible();
    EnumerateWkc AddParent(pWkc);
    if (m_bSingleLevel) continue;
    for (int i = 0; i < pWkc->GetMchCount(); ++i) AddChild(pWkc->m_arrMch[i]);
    EnumerateEnd
}

CString CViewMch::GetCellString(TElemDraw& draw)
{
    if (draw.m_bParent)
    {
        TWorkcenter* pWkc = (TWorkcenter*)(draw.m_data);
        switch (draw.m_cell)
        {
            case 0:
                return pWkc->m_id;
            case 1:
                return TWorkcenter::IsFlexible() ? IntToStr(pWkc->GetMchCount()) : _T("1");
            case 2:
                return IntToStr(pWkc->m_release);
            case 3:
                return CString(pWkc->m_status);
        }
    }
    else
    {
        TMachine* pMch = (TMachine*)(draw.m_data);
        switch (draw.m_cell)
        {
            case 0:
                return pMch->m_id;
            case 2:
                return IntToStr(pMch->m_release);
            case 3:
                return CString(pMch->m_status);
        }
    }
    return strEmpty;
}

CViewTree::TDrawCell CViewMch::NeedDrawCell(TElemDraw& draw)
{
    if (!TWorkcenter::IsFlexible()) return ndcYes;
    switch (draw.m_cell)
    {
        case -1:
            return ndcNo;
        case 0:
            return ndcYes;
        case 1:
            return draw.m_bParent ? ndcYes : ndcNo;
        default:
            return draw.m_bParent ? ndcNo : ndcYes;
    }
}

void CViewMch::OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw)
{
    if (draw.m_bParent && draw.m_cell == 0)
    {
        TWorkcenter* pWkc = (TWorkcenter*)draw.m_data;
        pWkc->DrawRect(pDC, rect);
        return;
    }
    super::OnDrawCell(pDC, rect, draw);
}

void CViewMch::DblClickElem(UINT nFlags, int elemNo)
{
    switch (WhatSelected())
    {
        case 0:
            OnMchEdit();
            break;
        case 1:
            OnWkcEdit();
            break;
        case -1:
            LekinBeep();
            break;
    }
}

void CViewMch::OnDelete()
{
    WhatSelected() ? OnWkcDel() : OnMchDel();
}

/////////////////////////////////////////////////////////////////////////////
// CViewMch message handlers

void CViewMch::OnWkcAdd()
{
    if (arrWorkcenter.GetSize() >= MaxWkc)
    {
        ReportTooMany(MaxWkc, IDS_WKC);
        return;
    }

    if (GetTotalMchCount() >= MaxMch)
    {
        ReportTooMany(MaxMch, IDS_MCH);
        return;
    }

    if (!theApp.CheckSequences(true)) return;

    CDlgAddWkc dlg;
    dlg.LoadData();
    if (arrWorkcenter.GetSize() > 0)
    {
        CString name = arrWorkcenter[arrWorkcenter.GetSize() - 1]->m_id;
        arrWorkcenter.AlterName(name);
        dlg.WkcID() = name;
    }

    if (dlg.DoModal() != IDOK) return;

    TWorkcenter* pWkc =
        new TWorkcenter(dlg.m_btnStyle.m_style, dlg.m_mchCount, dlg.m_release, dlg.m_status[0]);
    pWkc->m_setup = dlg.m_setup;
    arrWorkcenter.Add(pWkc);
    SetModifiedFlag2();
}

void CViewMch::OnWkcDel()
{
    if (GetSelCount() >= arrWorkcenter.GetSize())
    {
        LekinBeep();
        return;
    }
    if (!theApp.AskConfirmation(IDP_CNF_DEL_WKC, 1)) return;
    if (!theApp.CheckSequences(true)) return;

    TWorkcenterArray arrWkc;
    GetSelArray(*(TCollection*)&arrWkc);
    for (int i = arrWkc.GetSize(); --i >= 0;) arrWorkcenter.DestroyExact(arrWkc[i]);
    SetModifiedFlag2();
}

void CViewMch::OnWkcEdit()
{
    TWorkcenterArray arrWkc;
    GetSelArray(*(TCollection*)&arrWkc);
    theApp.EditWkc(arrWkc);
}

void CViewMch::OnMchAdd()
{
    TElement* elem = m_elements[LastSelected()];
    TWorkcenter* pWkc = (TWorkcenter*)(elem->m_data);

    CDlgAddMch dlg;
    dlg.LoadData();

    int count = pWkc->GetMchCount();
    if (count > 0)
    {
        dlg.m_idMch = pWkc->m_arrMch[count - 1]->m_id;
        AlterName(dlg.m_idMch);
    }
    else
        dlg.m_idMch = pWkc->m_id + _T(".01");

    dlg.m_idWkc = pWkc->m_id;
    dlg.m_mchCount = 1;
    dlg.m_release = pWkc->m_release;
    dlg.m_status = pWkc->m_status;
    if (dlg.DoModal() != IDOK) return;

    pWkc = dlg.m_pWkc;
    ASSERT(pWkc != NULL);

    CString name = dlg.m_idMch;

    for (count = dlg.m_mchCount; --count >= 0;)
    {
        pWkc->m_arrMch.AlterName(name);
        TMachine* pMch = new TMachine(pWkc, name, dlg.m_release, dlg.m_status[0]);
        pMch->m_comment = dlg.m_mchName;
        pWkc->m_arrMch.Add(pMch);
    }

    SetModifiedFlag2();
}

void CViewMch::OnMchDel()
{
    if (!theApp.AskConfirmDel(IDS_MCH)) return;
    if (!theApp.CheckSequences(true)) return;

    TMachineArray arrMch;
    GetSelArray(*(TCollection*)&arrMch);
    for (int i = arrMch.GetSize(); --i >= 0;) arrMch[i]->DestroyThis();

    TWorkcenter::SingleMachine();
    SetModifiedFlag2();
}

void CViewMch::OnMchEdit()
{
    TMachineArray arrMch;
    GetSelArray(*(TCollection*)&arrMch);
    theApp.EditMch(arrMch);
}

void CViewMch::OnUpdateWkcAdd(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!GotWkc() || TJob::GetShop() != OneShop);
}

void CViewMch::OnUpdateWkcDel(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(WhatSelected() == 1 && GetSelCount() < arrWorkcenter.GetSize());
}

void CViewMch::OnUpdateWkcEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(WhatSelected() == 1);
}

void CViewMch::OnUpdateMchOk(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TWorkcenter::IsFlexible() && WhatSelected() == 0);
}

void CViewMch::OnUpdateMchAdd(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TWorkcenter::IsFlexible() && WhatSelected() == 1 && GetSelCount() == 1 &&
                   GetTotalMchCount() < MaxMch);
}
