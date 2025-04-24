#pragma once

#include "ViewSmart.h"

class TCollection;

/////////////////////////////////////////////////////////////////////////////
// CViewTree view: "owner-draw tree" document view

class CViewTree : public CViewSmart
{
private:
    typedef CViewSmart super;

    struct TColumn
    {
        CString m_title;
        int m_width;

        TColumn(LPCTSTR title, int width) : m_title(title), m_width(width)
        {}
    };

    int m_parentCount;

protected:
    struct TElement
    {
        TElement* m_parent;
        bool m_bExpanded;
        bool m_bSelected;
        LPVOID m_data;
        CRect m_rect;

        TElement(TElement* parent, LPVOID data) :
            m_parent(parent),
            m_bExpanded(false),
            m_bSelected(false),
            m_data(data),
            m_rect(0, 0, 0, 0)
        {}
        bool IsParent()
        {
            return m_parent == NULL;
        }
    };

    struct TElemDraw
    {
        bool m_bParent;
        LPVOID m_data;
        int m_cell;

        TElemDraw(bool bParent) : m_bParent(bParent), m_data(NULL), m_cell(-1)
        {}

        TElemDraw(TElement* elem) : m_bParent(elem->m_parent == NULL), m_data(elem->m_data), m_cell(-1)
        {}
    };

private:
    bool GetNextActiveCell(TElemDraw& draw, int& start);
    void DoDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw);

public:
    enum TDrawCell
    {
        ndcNo,
        ndcMiss,
        ndcYes
    };
    enum
    {
        ParentMargin = 30,
        ChildMargin = 60,
        BlueMargin = 15
    };

    void SetTitles(const CString* titles, const int* widths);
    bool BeepWkc();
    void GetSelArray(TCollection& array);

    void SetModifiedFlag2(bool bModified = true);

    int GetCount();
    int GetCellCount();
    int GetSelCount();
    int WhatSelected();
    // returns -1 if no selection;
    //          0 if child selected;
    //          1 if parent selected

protected:
    bool m_bSingleSel;
    bool m_bSingleLevel;
    bool m_bAltReset;

    class TSet : public CTestMap<LPCVOID, LPCVOID>
    {};

    TSet m_expandedSet;
    TSet m_selectedSet;

    struct TDDTree : public CViewSmart::TDDSmart
    {
        CUIntArray m_arrTab;
        TDDTree(bool bPrinting) : TDDSmart(bPrinting)
        {}
    };

    int CalcTabs(CDC* pDC, int width, int shift, TDDTree* pDD);
    int CalcElementHeight(CDC* pDC, bool bParent);
    void PrintElement(CDC* pDC, int line, int& y, TDDTree* pDD);
    bool IsLineValid(int line);

    // for printing
    int m_indentX, m_interX;
    int m_prtY1, m_prtY2;
    CUIntArray m_arrPageStart;

    CSmartArray<TElement> m_elements;  // true
    CSmartArray<TColumn> m_columns;    // true

    CViewTree();  // protected constructor used by dynamic creation
    ~CViewTree();

    TElement* AddParent(LPVOID data);
    TElement* AddChild(LPVOID data);
    int FindItem(LPVOID data);
    void Select(int elemNo, int sel);
    void Expand(int elemNo, int exp);
    void UnselectAll();
    void GetParent(TElement*& elem);
    int LastSelected();  // last selected element

    const CRect& GetElementRect(int elemNo);
    CRect GetTabRect(int elemNo);

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    // overridden from CViewSmart
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

    // overridables!
    virtual void ClickElem(UINT nFlags, int elemNo);
    virtual void DblClickElem(UINT nFlags, int elemNo);
    virtual void OnDelete();

    virtual void GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font);
    virtual void OnPrepareCell(CDC* pDC, TElemDraw& draw);
    // SetTextAlign and SetTextColor here

    virtual void OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw);
    virtual void OnDrawHeader(CDC* pDC, const CRect& rect, int cell);

    virtual void Fill();
    virtual bool IsCellValid(TElemDraw& draw);
    virtual CString GetCellString(TElemDraw& draw);

    virtual TDrawCell NeedDrawCell(TElemDraw& draw);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CViewTree)
};
