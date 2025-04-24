#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"
#include "AFLibStringBuffer.h"

namespace AFLibIO {
class TFileTemp;
}

/////////////////////////////////////////////////////////////////////////////
// TUndoOp
// Set of elementary document changes constituting one operation.
// Used for undo/redo implementation.
// Elementary operations are stored in a temporary file.

namespace AFLibGui {
class TUndo;
class CDocAF;

class AFLIB TUndoOp
{
private:
    TUndo* m_pUndo;                  // containing sequence
    CString m_title;                 // name of the operation (e.g., "Add Funds")
    AFLib::CInt64Array m_arrOffset;  // pointers to elementary operations

    // temporary data file
    AFLibIO::TFileTemp& GetFile() const;

protected:
    // append operation
    void Append(LPCTSTR s);

public:
    TUndoOp(TUndo* pUndo, LPCTSTR title);
    virtual ~TUndoOp();

    // underlying document
    CDocAF* GetDoc();

    // get operation title
    const CString& GetTitle() const
    {
        return m_title;
    }

    // "Undo" or "Redo" plus title
    CString GetFullTitle() const;

    // get the number of operations
    int GetOperCount() const
    {
        return m_arrOffset.GetCount() - 1;
    }

    // read the indexed operation into the buffer
    void GetOper(int index, AFLibIO::TStringBuffer& buffer) const;
};
}  // namespace AFLibGui
