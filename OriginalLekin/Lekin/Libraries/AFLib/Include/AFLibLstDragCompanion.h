#pragma once

#include "AFLibLstOwner.h"

/////////////////////////////////////////////////////////////////////////////
// CLstDragCompanion list box
// A list box that can serve as a drag companion

namespace AFLibGui {
class TDragField;

class AFLIB CLstDragCompanion : public CLstOwner
{
private:
    typedef CLstOwner super;
    DEFINE_COPY_AND_ASSIGN(CLstDragCompanion);

    // drag field (handles cursors, drops, etc.)
    TDragField* m_pDf;

public:
    CLstDragCompanion();

    // set drag handler
    void SetDragField(TDragField* pDf);

    // get drag handler
    TDragField* GetDragField()
    {
        return m_pDf;
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnDestroy();

protected:
    // start dragging from point
    virtual bool StartDrag(const CPoint& pt);

    DECLARE_MESSAGE_MAP()
};
}  // namespace AFLibGui
