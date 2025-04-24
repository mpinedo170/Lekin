#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CWndAFHeader window
// Header for a list box; has columns, handles sorting

namespace AFLibGui {
class TFontDescriptor;

class AFLIB CWndAFHeader : public CWnd
{
private:
    typedef CWnd super;
    DEFINE_COPY_AND_ASSIGN(CWndAFHeader);

    HFONT m_hFont;  // font for processing WM_SETFONT

protected:
    int m_wClosed;        // width of a closed column
    double m_coefButton;  // size of a button in terms of header height

public:
    // Column information structure
    struct AFLIB TColumn
    {
        CString m_title;   // column title
        int m_width;       // column width
        bool m_bSort;      // sortable?
        bool m_bClose;     // collapsible?
        bool m_bCenter;    // center column?
        COLORREF m_color;  // color of the column

        TColumn();
    };

    // Sorting order
    struct TSort
    {
        int m_index;        // column index
        bool m_bAscending;  // ascending or descending order
    };

    // a sequence of TSort's fully defines the sorting order
    class TSortArray : public CArray<TSort>
    {};

private:
    // three types of buttons
    enum TAction
    {
        aClose,
        aSortAsc,
        aSortDesc
    };

    // internal structure for a column
    struct TColumnEx
    {
        TColumn m_col;         // "visible" part
        int m_sortOrder;       // current sort order (0 if not sorted by this column)
        bool m_bAscending;     // sort ascending?
        bool m_bVisible;       // column visible?
        CWndAFHeader* m_pWnd;  // host window

        // draw a button; may be "hilighted" if the mouse is in it
        bool DrawSquare(CDC* pDC, int& x, int side, TAction action, const CPoint& ptMouse, bool bDown);

        // paint the column
        bool Paint(CDC* pDC, const CSize& sz, const CPoint& ptMouse, bool bButtons);

        // get column's minimum width
        int GetMinWidth(CDC* pDC);
    };

    friend TColumnEx;

    AFLib::CSmartArray<TColumnEx> m_arrColumn;  // columns
    TSortArray m_arrSort;                       // sorting order
    CWnd* m_pWndScroll;                         // window whose horizontal scrollbar is used, or a scrollbar

    bool m_bMouseInside;  // is mouse inside the window?
    bool m_bLButtonDown;  // is mouse left button down?

    TAction m_action;   // last action chosen by user
    int m_actColumn;    // action column
    CPen* m_pPenLight;  // "light" pen for drawing buttons
    CPen* m_pPenDark;   // "dark" pen
    CPen* m_pPenBlack;  // black pen

    // current column width
    int GetWidth(int index);

    // fill m_arrSort based on columns
    void ResetSortOrder();

    // draw "sorting" triangle
    void DrawTriangle(CDC* pDC, int x, int y, int h, bool bAscending, bool b3D);

public:
    CWndAFHeader();
    ~CWndAFHeader();

    // create myself
    void Create(CWnd* pWndParent, CWnd* pWndScroll);

    // save current state (visibility and sorting)
    CString SaveState();

    // load state from string
    void LoadState(LPCTSTR sPtr);

    // insert a column
    void InsertColumn(int index, const TColumn& column);

    // add a column
    void AddColumn(const TColumn& column);

    // delete a column
    void DeleteColumn(int index);

    // delete all columns
    void DeleteAllColumns();

    // get column information
    const TColumn& GetColumn(int index);

    // change column information
    void SetColumn(int index, const TColumn& column);

    // show or hide column
    void ShowColumn(int index, bool bShow);

    // change sort order (direction does not change)
    void SetSortOrder(int index, int sortOrder);

    // change sort order and direction
    void SetSortOrder(int index, int sortOrder, bool bAscending);

    // the header has buttons
    bool HasButtons();

    // set column width
    void SetColumnWidth(int index, int width)
    {
        m_arrColumn[index]->m_col.m_width = width;
    }

    // get column width
    int GetColumnWidth(int index)
    {
        return m_arrColumn[index]->m_col.m_width;
    }

    // set column title
    void SetColumnTitle(int index, LPCTSTR title)
    {
        m_arrColumn[index]->m_col.m_title = title;
    }

    // get column title
    const CString& GetColumnTitle(int index)
    {
        return m_arrColumn[index]->m_col.m_title;
    }

    // get width in closed state
    int GetClosedWidth()
    {
        return m_wClosed;
    }

    // get sort order
    void GetSortOrder(int index, int& sortOrder, bool& bAscending);

    // get screen position of a column (in pixels)
    void GetInterval(int index, int& x1, int& x2);

    // get screen position of a column (in pixels); scrollbar is ignored
    void GetUnscrolledInterval(int index, int& x1, int& x2);

    // get minimum width for drawing a column in the given DC
    int GetColumnMinWidth(int index, CDC* pDC);

    // is column visible?
    bool IsVisible(int index)
    {
        return m_arrColumn[index]->m_bVisible;
    }

    // number of columns
    int GetColumnCount()
    {
        return m_arrColumn.GetSize();
    }

    // sorting order
    const TSortArray& GetSortArray()
    {
        return m_arrSort;
    }

    // controlling window
    CWnd* GetWndScroll()
    {
        return m_pWndScroll;
    }

protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);

    afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);

    // called when a column is shown or hidden
    virtual void Reshow();

    // called when sorting order changes
    virtual void Resort();

    // called when a user tries to show or hide a column;
    // if returns false, the action is not carried out
    virtual bool OnShowColumn(int column, bool bShow);

    // called when a user tries to resort a column;
    // if returns false, the action is not carried out
    virtual bool OnSortColumn(int column, bool bAscending);

    // called when a column was hidden, shown, or resorted
    virtual void OnChangeView();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CWndAFHeader)
};
}  // namespace AFLibGui
