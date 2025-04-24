#pragma once

class CFrmSmart;

/////////////////////////////////////////////////////////////////////////////
// CViewSmart view

class CViewSmart : public CViewTempEx<CScrollView>
{
private:
    typedef CViewTempEx<CScrollView> super;

    bool m_bBarX;
    bool m_bBarY;  // is there a scroll bar?

    friend CFrmSmart;

protected:
    CViewSmart();

    // print data
    CFont* m_pFontTitle;
    CRect m_rectPage;
    CRect m_rectCenter;
    int m_yTop;
    int m_yBottom;
    int m_pageCount;
    int m_page;

    // data for drawing
    struct TDDSmart
    {
        bool m_bPrinting;
        TDDSmart(bool bPrinting) : m_bPrinting(bPrinting)
        {}
        virtual ~TDDSmart()
        {}
    };

    TDDSmart* m_pDDPaint;
    TDDSmart* m_pDDPrint;

public:
    CFrmSmart* m_pWndParent;

    CDocTmpSmart* GetTmp();

    virtual BOOL Create(LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle,
        const RECT& rect,
        CWnd* pParentWnd,
        UINT nID,
        CCreateContext* pContext = NULL);
    virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = true);

protected:
    virtual ~CViewSmart();

    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnDraw(CDC* pDC);
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnFileNew();
    afx_msg void OnFileOpen();
    afx_msg void OnFileApp();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    afx_msg void OnUpdateFile(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWin(CCmdUI* pCmdUI);

    bool HasBarX()
    {
        return m_bBarX;
    }
    bool HasBarY()
    {
        return m_bBarY;
    }

    enum THeader
    {
        Center = -1,
        Top,
        Bottom,
        Left,
        Right
    };
    CRect GetHeaderRect(const CRect& rectFrame, const CRect& rectView, THeader header, bool bPrinting);

    void ScreenUpdate();
    bool IsVisible();
    void PrintPageNo(CDC* pDC);

    // overridden from TViewEx

    virtual T3State ViewExUpdateFg(CDC* pDC, const TMargin* pMargin);
    virtual void ViewExPrintPage(CDC* pDC, int page);

    // overridables!

    virtual CDocTmpSmart* GetFileTmp();
    virtual TDDSmart* CreateDD(bool bPrinting);
    virtual void Update();
    virtual void Click(UINT nFlags, CPoint point);
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
    DECLARE_DYNCREATE(CViewSmart)
};
