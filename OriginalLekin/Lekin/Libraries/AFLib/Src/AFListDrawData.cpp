#include "StdAfx.h"

#include "AFLibAFListDrawData.h"

#include "AFLibDCStateSaver.h"
#include "AFLibFontDescriptor.h"
#include "AFLibWndAFHeader.h"

using namespace AFLibGui;
using namespace AFLib;

TAFListDrawData::TAFListDrawData(const TFontDescriptor* pFdTop, const TFontDescriptor* pFdBody)
{
    m_pFdTop = pFdTop;
    m_pFdBody = pFdBody;
    m_pFontTop = m_pFontBody = NULL;
    PrivateClear();
}

TAFListDrawData::~TAFListDrawData()
{
    PrivateClear();
}

void TAFListDrawData::PrivateClear()
{
    m_hFont = m_hFontTop = m_wTable = m_hTop = 0;
    m_arrTab.RemoveAll();
    delete m_pFontTop;
    m_pFontTop = NULL;
    delete m_pFontBody;
    m_pFontBody = NULL;
}

void TAFListDrawData::Clear()
{
    PrivateClear();
}

void TAFListDrawData::Draw(CDC* pDC, int x, int y, int index) const
{
    int wClosed = GetClosedWidth(pDC);
    int nCols = GetColumnCount();
    COLORREF defColor = pDC->GetTextColor();

    for (int i = 0; i < nCols; ++i)
    {
        if (IsColumnVisible(pDC, i))
        {
            CString s = GetStr(index, i);
            int w = m_arrTab[i];
            COLORREF color = GetCellColor(pDC, index, i);
            bool bCenter = IsColumnCenter(i);

            if (color != colorAuto) pDC->SetTextColor(color);
            pDC->SetTextAlign(TA_TOP | (bCenter ? TA_CENTER : TA_LEFT));
            pDC->TextOut(bCenter ? x + w / 2 : x, y, s);
            x += w;
            pDC->SetTextColor(defColor);
        }
        else if (!pDC->IsPrinting())
            x += wClosed;
    }
}

void TAFListDrawData::FillHeaderColumns(CWndAFHeader* pWnd) const
{
    pWnd->DeleteAllColumns();
    int nCols = GetColumnCount();
    CDC* pDC = pWnd->GetDC();

    for (int i = 0; i < nCols; ++i)
    {
        CWndAFHeader::TColumn col;
        col.m_bCenter = IsColumnCenter(i);
        col.m_color = GetColumnColor(pDC, i);
        col.m_title = GetStr(-1, i);
        pWnd->AddColumn(col);
    }

    pWnd->ReleaseDC(pDC);
}

void TAFListDrawData::ResetHeaderWidths(CWndAFHeader* pWnd) const
{
    pWnd->SetFont(m_pFontTop);
    for (int i = 0; i < GetColumnCount(); ++i) pWnd->SetColumnWidth(i, m_arrTab[i]);
}

void TAFListDrawData::Compute(CDC* pDC)
{
    TDCStateSaver dcss(pDC);
    int wClosed = GetClosedWidth(pDC);
    int nCols = GetColumnCount();
    int nRows = GetRowCount();

    m_pFontTop = new CFont;
    m_pFdTop->CreateFont(*m_pFontTop, pDC);
    m_pFontBody = new CFont;
    m_pFdBody->CreateFont(*m_pFontBody, pDC);

    m_arrTab.SetSize(nCols);
    m_wTable = 0;
    m_hFont = GetFontHeight(pDC, m_pFontBody);
    m_hFontTop = GetFontHeight(pDC, m_pFontTop);

    m_hTop = int(m_hFontTop * 1.2);
    if (!pDC->IsPrinting()) m_hTop += 2 * GetSystemMetrics(SM_CYBORDER);

    for (int i = 0; i < nCols; ++i)
    {
        int w = GetColumnMinWidth(pDC, i);
        pDC->SelectObject(m_pFontBody);

        for (int j = 0; j < nRows; ++j)
        {
            CString s = GetStr(j, i);
            w = max(w, int(pDC->GetTextExtent(s).cx + m_hFont / 2));
        }

        w += 2;
        m_arrTab[i] = w;
        m_wTable += IsColumnVisible(pDC, i) ? w : wClosed;
    }
}

int TAFListDrawData::GetColumnMinWidth(CDC* pDC, int col) const
{
    TDCStateSaver dcss(pDC);
    pDC->SelectObject(m_pFontTop);
    return pDC->GetTextExtent(GetStr(-1, col)).cx + m_hFontTop / 6;
}

COLORREF TAFListDrawData::GetColumnColor(CDC* pDC, int col) const
{
    return colorAuto;
}

COLORREF TAFListDrawData::GetCellColor(CDC* pDC, int row, int col) const
{
    return GetColumnColor(pDC, col);
}
