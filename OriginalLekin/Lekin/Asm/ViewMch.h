#pragma once

/////////////////////////////////////////////////////////////////////////////
// CViewMch view

#include "ViewTree.h"

class CViewMch : public CViewTree
{
private:
    typedef CViewTree super;

protected:
    CViewMch();

    afx_msg void OnWkcAdd();
    afx_msg void OnWkcDel();
    afx_msg void OnWkcEdit();
    afx_msg void OnUpdateWkcEdit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWkcDel(CCmdUI* pCmdUI);
    afx_msg void OnMchAdd();
    afx_msg void OnMchDel();
    afx_msg void OnMchEdit();
    afx_msg void OnUpdateMchOk(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMchAdd(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWkcAdd(CCmdUI* pCmdUI);

    virtual void Fill();
    virtual void OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw);
    virtual CString GetCellString(TElemDraw& draw);
    virtual TDrawCell NeedDrawCell(TElemDraw& draw);
    virtual void DblClickElem(UINT nFlags, int elemNo);
    virtual void OnDelete();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CViewMch)
};
