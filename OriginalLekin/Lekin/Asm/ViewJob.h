#pragma once

#include "ViewTree.h"

/////////////////////////////////////////////////////////////////////////////
// CViewJob view

class CViewJob : public CViewTree
{
private:
    typedef CViewTree super;

    int m_maxProc;

protected:
    CViewJob();

    afx_msg void OnJobAdd();
    afx_msg void OnJobDel();
    afx_msg void OnJobEdit();
    afx_msg void OnOperAdd();
    afx_msg void OnOperDel();
    afx_msg void OnOperEdit();
    afx_msg void OnUpdateJobAdd(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobEdit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobDel(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOperAdd(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOperEdit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOperDel(CCmdUI* pCmdUI);

    virtual void Fill();
    virtual bool IsCellValid(TElemDraw& draw);
    virtual void OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw);
    virtual void GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font);
    virtual CString GetCellString(TElemDraw& draw);
    virtual TDrawCell NeedDrawCell(TElemDraw& draw);
    virtual void ClickElem(UINT nFlags, int elemNo);
    virtual void DblClickElem(UINT nFlags, int elemNo);
    virtual void OnDelete();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CViewJob)
};
