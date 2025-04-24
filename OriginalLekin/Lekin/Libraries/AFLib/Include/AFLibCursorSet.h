#pragma once

/////////////////////////////////////////////////////////////////////////////
// TCursorSet
// Contains 3 cursors, initializes and shows an appropriate one

namespace AFLibGui {
class AFLIB TCursorSet
{
public:
    HCURSOR m_hCurDrag;  // drag cursor
    HCURSOR m_hCurCopy;  // drag cursor (with copy)
    HCURSOR m_hCurStop;  // stop cursor (where can't drop)
    int m_timerMS;       // timer milliseconds

    TCursorSet();

    // initialize cursors
    void Init();

    // clear cursor handles
    void Clear();

    // get appropriate cursor
    void SetCursor(bool bOk, bool bCopy) const;

    // restore the cursor to basic arrow
    static void RestoreCursor();
};
}  // namespace AFLibGui
