#pragma once

#include "ViewSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CViewObj view

class CViewObj : public CViewSmart
{
private:
    typedef CViewSmart super;

protected:
    struct TRectData
    {
        CRect m_rectMain;
        CRect m_rectCheck;
        TSchedule* m_pSch;

        bool IsShown();
    };

    class TArrRD : public CArray<TRectData>
    {};

    struct TDDObj : public CViewSmart::TDDSmart
    {
        CSize m_szMain;
        TArrRD m_arrRD;
        TDDObj(bool bPrinting) : TDDSmart(bPrinting)
        {}
        int FindSch(const TSchedule* pSch);
    };

private:
    CSize CalcLegendSize(CDC* pDC,
        const CSize& szTotal,
        const CSize& szRect,
        const CSize& szMargin,
        const CSize& szSpace,
        bool bVertical,
        TDDObj* pDD);
    CSize CalcLegend(CDC* pDC, const CSize& szTotal, TDDObj* pDD);

protected:
    CViewObj();

public:
    void SetMask(int axisMask);
    int GetAxisCount();
    TObjective GetRealIndex(int axis);

    int GetObj(int axis, const TSchedule* pSch);

protected:
    int m_axisMask;  // axis mask: which axis' are used

    double m_arrMax[objN];
    CPoint m_origin;

    CPen m_penGray;
    CPen m_penDkGray;
    CPen m_penBlack;

    void DrawAxisRect(CDC* pDC, TDDObj* pDD);
    void DrawAxisStar(CDC* pDC, TDDObj* pDD);

    void DrawGraph1(CDC* pDC, TDDObj* pDD);
    void DrawGraph2(CDC* pDC, TDDObj* pDD);
    void DrawGraphM(CDC* pDC, TDDObj* pDD);
    CPoint AxisToXY(int axis, double val);
    CPoint ObjToXY(int axis, const TSchedule* pSch);

    void DrawGraph(CDC* pDC, TDDObj* pDD);
    void DrawLegends(CDC* pDC, TDDObj* pDD);

    // overridden
    virtual CDocTmpSmart* GetFileTmp();
    virtual void Click(UINT nFlags, CPoint point);
    virtual void DblClick(UINT nFlags, CPoint point);
    virtual TDDSmart* CreateDD(bool bPrinting);

    virtual CSize OnScreenUpdate(CDC* pDC, TDDSmart* pDD);
    virtual int OnPrintUpdate(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin);
    virtual void DrawView(CDC* pDC, CSize sz, TDDSmart* pDD);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CViewObj)
};
