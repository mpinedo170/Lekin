#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TDragField
// Defines an item that can be dragged

namespace AFLibGui {
class AFLIB TDragField
{
private:
    DEFINE_COPY_AND_ASSIGN(TDragField);

    AFLib::CSmartArray<CWnd> m_arrCom;  // companions -- possible drop targets
    CWnd* m_pWndHelper;                 // supplementary window to handle messages
    CWnd* m_pWndSource;                 // "myself"

protected:
    int m_timerMS;       // milliseconds for timer updates of m_pWndHelper
    HCURSOR m_hCurDrag;  // drag cursor
    HCURSOR m_hCurStop;  // stop cursor (where can't drop)

    // get the window that's the source of the dragged item
    CWnd* GetWndSource()
    {
        return m_pWndSource;
    }

public:
    TDragField();
    ~TDragField();

    // is currently being dragged
    bool InDrag()
    {
        return m_pWndHelper != NULL;
    }

    // number of drop targets
    int GetCount()
    {
        return m_arrCom.GetSize();
    }

    // i-th drop target
    CWnd* GetCompanion(int index)
    {
        return m_arrCom[index];
    }

    // add drop target
    virtual void AddCompanion(CWnd* pWnd);

    // remove drop target
    virtual void RemoveCompanion(CWnd* pWnd);

    // start dragging
    virtual void StartDrag(CWnd* pWndSource);

    // stop dragging (drop!)
    virtual void StopDrag(CWnd* pWndDest);

    // cancel dragging (do not drop)
    virtual void CancelDrag();

    // "drop" easy overridable
    virtual void Drop(CWnd* pWndSource, CWnd* pWndDest);

    // mouse overridables
    virtual void DragMouseMove(UINT nFlags, const CPoint& point);
    virtual void DragLButtonUp(UINT nFlags, const CPoint& point);

    // timer update message handler
    virtual void DragTimer();

protected:
    // if dragged from a list, number of selected items; o.w. 1
    virtual int GetDragItemCount(CWnd* pWndSource);

    // window-from-point -- the current drop target
    virtual CWnd* FindPoint(const CPoint& point);
};
}  // namespace AFLibGui
