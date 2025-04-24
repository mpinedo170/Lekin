#pragma once

#include "AFLibExcMessage.h"

/////////////////////////////////////////////////////////////////////////////
// CExcBreak
// The "Break" exception: gets thrown in a running thread
//   when another thread tries to stop it.

namespace AFLibThread {
class AFLIB CExcBreak : public AFLib::CExcMessage
{
private:
    typedef AFLib::CExcMessage super;
    DEFINE_COPY_AND_ASSIGN(CExcBreak);

public:
    explicit CExcBreak(LPCTSTR message = NULL);

    DECLARE_DYNAMIC(CExcBreak)
};
}  // namespace AFLibThread
