#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TModalCounter
// Counts modal dialogs on top of each other
// Constructor increments the counter, destructor decrements

namespace AFLibGui {
class AFLIB TModalCounter
{
private:
    DEFINE_COPY_AND_ASSIGN(TModalCounter);

    static int m_nModalDlgs;  // number of modal dialogs

public:
    TModalCounter();
    ~TModalCounter();

    // are we currently in a modal dialog?
    static bool IsModal();
};
}  // namespace AFLibGui
