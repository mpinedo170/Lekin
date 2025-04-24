#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TAFListDrawData
// Drawing/printing data for the contents of a list controlled by CWndAFHeader

namespace AFLibGui {
class CWndAFHeader;
class TFontDescriptor;

class AFLIB TAFListDrawData
{
private:
    DEFINE_COPY_AND_ASSIGN(TAFListDrawData);

    const TFontDescriptor* m_pFdTop;   // font descriptor for the header
    const TFontDescriptor* m_pFdBody;  // font descriptor for the table body

    // initialize drawing data
    void PrivateClear();

public:
    CFont* m_pFontTop;          // header font
    CFont* m_pFontBody;         // body font
    int m_hFont;                // font size
    int m_hFontTop;             // top font size
    int m_wTable;               // total width of the header and the table below
    int m_hTop;                 // height of the top part
    AFLib::CIntArray m_arrTab;  // tab stops

    TAFListDrawData(const TFontDescriptor* pFdTop, const TFontDescriptor* pFdBody);
    virtual ~TAFListDrawData();

    // fill columns for the header ("sort" and "closed" are assumed to be off)
    void FillHeaderColumns(CWndAFHeader* pWnd) const;

    // reset column widths in the given header window
    void ResetHeaderWidths(CWndAFHeader* pWnd) const;

    // clear the structure
    virtual void Clear();

    // draw a row of the table
    virtual void Draw(CDC* pDC, int x, int y, int index) const;

    // fill the data (widths, heights, ...)
    virtual void Compute(CDC* pDC);

    // the number of columns in the table
    virtual int GetColumnCount() const = 0;

    // the number of rows in the table
    virtual int GetRowCount() const = 0;

    // get the string for a cell
    virtual CString GetStr(int row, int col) const = 0;

    // get the color of the cell
    virtual COLORREF GetCellColor(CDC* pDC, int row, int col) const;

    // get width in closed state
    virtual int GetClosedWidth(CDC* pDC) const = 0;

    // is column visible?
    virtual bool IsColumnVisible(CDC* pDC, int col) const = 0;

    // get the color for a column
    virtual bool IsColumnCenter(int col) const = 0;

    // get the color for a column
    virtual COLORREF GetColumnColor(CDC* pDC, int col) const;

    // get minimum column width
    virtual int GetColumnMinWidth(CDC* pDC, int col) const;
};
}  // namespace AFLibGui
