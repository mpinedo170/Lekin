#pragma once

#include "AFLibArrays.h"
#include "AFLibFileTemp.h"

/////////////////////////////////////////////////////////////////////////////
// TUndo
// Sequence of document modifications for undo/redo implementation.

namespace AFLibGui {
class TUndoOp;
class CDocAF;

class AFLIB TUndo
{
private:
    friend TUndoOp;
    DEFINE_COPY_AND_ASSIGN(TUndo)

    CDocAF* m_pDoc;                       // underlying document
    CString m_title;                      // "Undo" or "Redo"
    AFLibIO::TFileTemp m_F;               // storage for all undo/redo data
    AFLib::CSmartArray<TUndoOp> m_arrUo;  // sequence of modifications

    // number of modifications
    int GetCount() const
    {
        return m_arrUo.GetSize();
    }

public:
    TUndo(CDocAF* pDoc, LPCTSTR title);
    ~TUndo();

    // add one more operation
    void Append(LPCTSTR undoTitle);

    // remove last operation
    void Retreat();

    // remove everything
    void Clear();

    // last operation
    TUndoOp* GetActive();

    // get title ("Undo" or "Redo")
    const CString& GetTitle() const
    {
        return m_title;
    }
};
}  // namespace AFLibGui
