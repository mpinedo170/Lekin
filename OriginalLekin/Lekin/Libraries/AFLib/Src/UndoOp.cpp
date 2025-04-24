#include "StdAfx.h"

#include "AFLibUndoOp.h"

#include "AFLibDocAF.h"
#include "AFLibFileTemp.h"
#include "AFLibStrings.h"
#include "AFLibUndo.h"

using namespace AFLib;
using namespace AFLibGui;
using namespace AFLibIO;

TUndoOp::TUndoOp(TUndo* pUndo, LPCTSTR title)
{
    m_pUndo = pUndo;
    m_title = title;
    Append(NULL);
}

TUndoOp::~TUndoOp()
{
    GetFile().SetLength(m_arrOffset[0]);
}

TFileTemp& TUndoOp::GetFile() const
{
    return m_pUndo->m_F;
}

CDocAF* TUndoOp::GetDoc()
{
    return m_pUndo->m_pDoc;
}

void TUndoOp::GetOper(int index, TStringBuffer& buffer) const
{
    int len = int(m_arrOffset[index + 1] - m_arrOffset[index]);

    GetFile().Seek(m_arrOffset[index], CFile::begin);
    buffer.Empty();
    buffer.Read(GetFile(), len);
}

void TUndoOp::Append(LPCTSTR s)
{
    if (s != NULL) FileWrite(GetFile(), s);
    m_arrOffset.Add(GetFile().GetLength());
}

CString TUndoOp::GetFullTitle() const
{
    return m_pUndo->GetTitle() + strSpace + m_title;
}
