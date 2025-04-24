#include "StdAfx.h"

#include "AFLibDocAF.h"

#include "AFLibFrmAF.h"
#include "AFLibUndo.h"
#include "AFLibUndoOp.h"
#include "Local.h"

using namespace AFLibGui;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;

IMPLEMENT_DYNAMIC(CDocAF, super)

CDocAF::CDocAF()
{
    m_pUndo = new TUndo(this, LocalAfxString(IDS_UNDO));
    m_pRedo = new TUndo(this, LocalAfxString(IDS_REDO));
    m_tcg = tcgNone;
}

CDocAF::~CDocAF()
{
    delete m_pUndo;
    delete m_pRedo;
}

CFrmAF* CDocAF::GetFrmPositionHelper(POSITION& rPosition)
{
    for (; rPosition != NULL; m_viewList.GetNext(rPosition))
    {
        CView* pView = static_cast<CView*>(m_viewList.GetAt(rPosition));
        CFrmAF* pFrm = dynamic_cast<CFrmAF*>(pView->GetParent());
        if (pFrm != NULL) return pFrm;
    }
    return NULL;
}

POSITION CDocAF::GetFirstFrmPosition()
{
    POSITION pos = m_viewList.GetHeadPosition();
    GetFrmPositionHelper(pos);
    return pos;
}

CFrmAF* CDocAF::GetNextFrm(POSITION& rPosition)
{
    CFrmAF* pFrm = GetFrmPositionHelper(rPosition);
    m_viewList.GetNext(rPosition);
    GetFrmPositionHelper(rPosition);
    return pFrm;
}

TUndoOp* CDocAF::GetActiveUndo()
{
    switch (m_tcg)
    {
        case tcgRegular:
        case tcgRedo:
            return m_pUndo->GetActive();
        case tcgUndo:
            return m_pRedo->GetActive();
    }
    return NULL;
}

void CDocAF::ProtectedUndoRedo(TUndo* pUndoSource, TUndo* pUndoDest, TTypeChange tcg)
{
    TUndoOp* pUo = pUndoSource->GetActive();
    if (pUo == NULL) return;

    m_tcg = tcg;
    pUndoDest->Append(pUo->GetTitle());
    ProcessStartChanges(false);
    ProcessUndoOp(pUo);
    pUndoSource->Retreat();
    FinishChanges();
}

void CDocAF::StartChanges(LPCTSTR undoTitle)
{
    m_tcg = tcgRegular;
    m_pRedo->Clear();
    m_pUndo->Append(undoTitle);
    ProcessStartChanges(false);
}

void CDocAF::StartNewDocChanges()
{
    m_tcg = tcgNoUndo;
}

void CDocAF::StartNoUndoChanges(bool bClearSel)
{
    m_tcg = tcgNoUndo;
    ProcessStartChanges(bClearSel);
}

void CDocAF::FinishChanges()
{
    m_tcg = tcgNone;
    SetModifiedFlag();
    UpdateAllViews(NULL, 0, NULL);
}

BEGIN_MESSAGE_MAP(CDocAF, super)
ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndoRedo)
ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditUndoRedo)
END_MESSAGE_MAP()

void CDocAF::OnEditUndo()
{
    Undo();
}

void CDocAF::OnEditRedo()
{
    Redo();
}

void CDocAF::OnUpdateEditUndoRedo(CCmdUI* pCmdUI)
{
    bool bUndo = pCmdUI->m_nID == ID_EDIT_UNDO;
    TUndo* pUndo = bUndo ? m_pUndo : m_pRedo;
    TUndoOp* pUo = pUndo->GetActive();
    pCmdUI->Enable(pUo != NULL);

    TStringBuffer buffer;
    buffer += _T('&');
    buffer += pUndo->GetTitle();

    if (pUo != NULL)
    {
        buffer += chrSpace;
        buffer += pUo->GetTitle();
    }

    buffer += _T("\tCtrl+");
    buffer += bUndo ? _T('Z') : _T('Y');
    pCmdUI->SetText(buffer);
}
