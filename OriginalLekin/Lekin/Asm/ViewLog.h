#pragma once

/////////////////////////////////////////////////////////////////////////////
// CViewLog view

#include "ViewTree.h"

class CViewLog : public CViewTree
{
private:
    typedef CViewTree super;

protected:
    CViewLog();

    virtual CDocTmpSmart* GetFileTmp();
    virtual void Fill();
    virtual void OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw);
    virtual void OnDrawHeader(CDC* pDC, const CRect& rect, int cell);
    virtual void GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font);
    virtual CString GetCellString(TElemDraw& draw);

    virtual void ClickElem(UINT nFlags, int elemNo);
    virtual void DblClickElem(UINT nFlags, int elemNo);
    virtual void OnDelete();

    DECLARE_DYNCREATE(CViewLog)
    DECLARE_MESSAGE_MAP()
};
