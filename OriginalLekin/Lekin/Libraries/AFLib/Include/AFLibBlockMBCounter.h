#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TBlockMBCounter
// Counts message box blocks
// Constructor increments the counter, destructor decrements

namespace AFLibGui
{ class AFLIB TBlockMBCounter
  { private:
      DEFINE_COPY_AND_ASSIGN(TBlockMBCounter);

      static int m_nBlockMB;  // number of nested blocks

    public:
      TBlockMBCounter();
      ~TBlockMBCounter();

      // are message boxes currently blocked?
      static bool IsMsgBoxBlocked();
  };
}
