#pragma once

#include "ViewSmart.h"

class TWorkcenter;
class TMachine;
class TOperation;
struct TSequence;

/////////////////////////////////////////////////////////////////////////////
// CViewGantt view

class CViewGantt : public CViewSmart
{
private:
    typedef CViewSmart super;

    enum
    {
        viGrid = 1,
        viJobs = 2,
        viLate = 4,
        viStats = 8
    };
    int m_grid;

    void AAA(bool bX, bool bUp);

    // printing data
    int m_pagesX;
    int m_pagesY;
    int m_wSquare;
    int m_hSquare;

    bool m_bNoSplit;

    int m_curPageX;
    int m_curPageY;
    int m_perPageX;

protected:
    CViewGantt();

    void DrawCursor();
    void InitDragging();
    HCURSOR CreateCursor(COLORREF color);
    void StopDragging();

    struct TPosition
    {
        int m_page;
        int m_yFrom;
        int m_yTo;
    };

    struct TDDGantt : CViewSmart::TDDSmart
    {
        double m_scaleX;
        double m_scaleY;
        int m_yBlk;
        int m_ySep;
        int m_yMax;
        int m_pagesY;

        int m_xFrom;
        int m_xTo;

        int m_iMark;
        int m_iWave;
        int m_iHeight;
        int m_iBegin;
        int m_iEnd;

        CMap<LPVOID, LPVOID, TPosition, TPosition> m_mapPos;

        TDDGantt(bool bPrinting);
        int GetX(double x);
        double GetAntiX(int x);
        void InitDrawData(CDC* pDC, int maxX);
        void GetWaveFont(CDC* pDC, CFont& font);
        void GetBeginEnd(CDC* pDC, int maxX);
        bool FillPageArray(double scaleY, int height, int pagesY, bool bNoSplit);
        bool FindScale(double minScaleY, double maxScaleY, int height, int pagesY, bool bNoSplit);
        void GetPosition(const TMachine* pMch, TPosition& pos);
    };

    static int GetApproxY();

    void GetFont(CDC* pDC, CFont& font, bool bBold, TDDGantt* pDD);
    int Visibility(const TWorkcenter* pWkc, CRect& clip, TDDGantt* pDD);
    int Visibility(const TMachine* pMch, CRect& clip, TDDGantt* pDD);
    CRect GetRect(const TOperation* pOp, TDDGantt* pDD);

    void DrawMark(CDC* pDC, int cy, int where, COLORREF color, TDDGantt* pDD);
    void DrawTimes(CDC* pDC, CSize sz, TDDGantt* pDD);
    void DrawMch(CDC* pDC, CSize sz, TDDGantt* pDD);

public:
    bool m_bDragging;
    HCURSOR m_cursor;
    TOperation* m_dragOp;

    TSequence* m_dropSeq;
    int m_dropTag;
    int m_dropTime;
    CPoint m_lastPt;

    int m_maxX;

    bool ZoomFit();

protected:
    afx_msg void OnZoomLonger();
    afx_msg void OnZoomShorter();
    afx_msg void OnZoomWider();
    afx_msg void OnZoomNarrower();
    afx_msg void OnUpdateZoomLonger(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoomShorter(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoomWider(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoomNarrower(CCmdUI* pCmdUI);
    afx_msg void OnZoomFit();
    afx_msg void OnUpdateZoomFit(CCmdUI* pCmdUI);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnDestroy();
    afx_msg void OnGrid(UINT nID);
    afx_msg void OnUpdateGrid(CCmdUI* pCmdUI);

    // overridden from CViewSmart

    virtual CDocTmpSmart* GetFileTmp();
    virtual TDDSmart* CreateDD(bool bPrinting);
    virtual void Update();
    virtual void DblClick(UINT nFlags, CPoint point);

    virtual CRect GetViewRect(CDC* pDC, const CRect& rectFrame, TDDSmart* pDD);
    virtual CSize OnScreenUpdate(CDC* pDC, TDDSmart* pDD);
    virtual int OnPrintUpdate(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin);
    virtual int GetPrintYBottom(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin);
    virtual void PrintBottom(CDC* pDC, TDDSmart* pDD);

    virtual void DrawView(CDC* pDC, CSize sz, TDDSmart* pDD);
    virtual void DrawHeader(THeader header, CDC* pDC, CSize sz, TDDSmart* pDD);
    virtual void DrawMisc(CDC* pDC, TDDSmart* pDD);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CViewGantt)
};
