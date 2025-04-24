#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CDocAF
// Document with Undo/Redo and other useful functions

namespace AFLibGui {
class CFrmAF;
class TUndo;
class TUndoOp;

class AFLIB CDocAF : public CDocument
{
private:
    typedef CDocument super;
    DEFINE_COPY_AND_ASSIGN(CDocAF);

    // helper for GetFirstFrmPosition() and GetNextFrm()
    CFrmAF* GetFrmPositionHelper(POSITION& rPosition);

protected:
    TUndo* m_pUndo;  // undo structure
    TUndo* m_pRedo;  // redo structure

    // types of document changes
    enum TTypeChange
    {
        tcgNone,
        tcgNoUndo,
        tcgRegular,
        tcgUndo,
        tcgRedo
    };

    TTypeChange m_tcg;  // if not tcgNone, changes under way, do not redraw

    CDocAF();

    // get first frame
    POSITION GetFirstFrmPosition();

    // get next frame
    CFrmAF* GetNextFrm(POSITION& rPosition);

    // active undo operation, may be NULL, or from "undo", or from "redo"
    TUndoOp* GetActiveUndo();

    // undo or redo
    virtual void ProtectedUndoRedo(TUndo* pUndoSource, TUndo* pUndoDest, TTypeChange tcg);

    // overridable: process one Undo operation
    virtual void ProcessUndoOp(TUndoOp* pUo) = 0;

    // helper function for StartChanges()
    virtual void ProcessStartChanges(bool bClearSel) = 0;

public:
    ~CDocAF();

    // start changing the document
    virtual void StartChanges(LPCTSTR undoTitle);

    // start filling a new document
    virtual void StartNewDocChanges();

    // start changes that cannot be undone
    virtual void StartNoUndoChanges(bool bClearSel);

    // finish changing the document
    virtual void FinishChanges();

    // process the undo command
    void Undo()
    {
        ProtectedUndoRedo(m_pUndo, m_pRedo, tcgUndo);
    }

    // process the undo command
    void Redo()
    {
        ProtectedUndoRedo(m_pRedo, m_pUndo, tcgRedo);
    }

    // overridable: create TUndoOp (used from TUndo)
    virtual TUndoOp* CreateUndoOp(TUndo* pUndo, LPCTSTR undoTitle) = 0;

    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnUpdateEditUndoRedo(CCmdUI* pCmdUI);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CDocAF)
};
}  // namespace AFLibGui
