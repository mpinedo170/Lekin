#include "StdAfx.h"

#include "AFLibUndo.h"

#include "AFLibDocAF.h"
#include "AFLibUndoOp.h"

using namespace AFLibGui;

TUndo::TUndo(CDocAF* pDoc, LPCTSTR title) :
    m_pDoc(pDoc),
    m_title(title),
    m_F(title, true, CFile::shareDenyWrite)
{}

TUndo::~TUndo()
{
    Clear();
}

void TUndo::Append(LPCTSTR undoTitle)
{
    m_arrUo.Add(m_pDoc->CreateUndoOp(this, undoTitle));
}

void TUndo::Retreat()
{
    int count = GetCount();
    if (count > 0)
    {
        delete GetActive();
        m_arrUo.SetSize(count - 1);
    }
}

void TUndo::Clear()
{
    while (GetCount() > 0) Retreat();
}

TUndoOp* TUndo::GetActive()
{
    int count = GetCount();
    return count > 0 ? m_arrUo[count - 1] : NULL;
}
