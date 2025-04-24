#pragma once

#include "Route.h"
#include "ViewTree.h"

/////////////////////////////////////////////////////////////////////////////
// CViewSeq view

class CViewSeq : public CViewTree
{
private:
    typedef CViewTree super;

    bool IsSpecial(LPVOID data);
    bool IsSpecial(TElemDraw& draw);
    int m_maxProc;

public:
    int m_sortMethod;
    TRoute m_oldRoute;

protected:
    CViewSeq();

    virtual void Fill();
    virtual void OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw);
    virtual void GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font);
    virtual CString GetCellString(TElemDraw& draw);
    virtual TDrawCell NeedDrawCell(TElemDraw& draw);
    virtual void ClickElem(UINT nFlags, int elemNo);
    virtual void DblClickElem(UINT nFlags, int elemNo);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CViewSeq)
};
