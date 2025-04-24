#include "StdAfx.h"

#include "AppAsm.h"
#include "ViewTree.h"

#include "DocSmart.h"
#include "DocTmpSmart.h"

#include "Draws.h"
#include "ID.h"
#include "Misc.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CViewTree

IMPLEMENT_DYNCREATE(CViewTree, CViewSmart)

CViewTree::CViewTree()
{
    m_bSingleLevel = m_bSingleSel = m_bAltReset = false;
}

CViewTree::~CViewTree()
{
    m_elements.DestroyAll();
    m_columns.DestroyAll();
}

void CViewTree::SetTitles(const CString* titles, const int* widths)
{
    m_columns.DestroyAll();
    for (int i = 0; !titles[i].IsEmpty(); ++i)
    {
        TColumn* col = new TColumn(titles[i], widths ? widths[i] : titles[i].GetLength() + 2);
        m_columns.Add(col);
    }
}

bool CViewTree::BeepWkc()
{
    if (!GotWkc())
    {
        LekinBeep();
        return false;
    }
    return true;
}

BEGIN_MESSAGE_MAP(CViewTree, super)
ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// Manipulators

void CViewTree::SetModifiedFlag2(bool bModified)
{
    GetTmp()->GetDoc()->SetModifiedFlag2(bModified);
}

int CViewTree::GetCount()
{
    return m_elements.GetSize();
}

int CViewTree::GetCellCount()
{
    return m_columns.GetSize();
}

int CViewTree::GetSelCount()
{
    int s = 0;
    for (int i = 0; i < GetCount(); ++i) s += m_elements[i]->m_bSelected;
    return s;
}

void CViewTree::GetSelArray(TCollection& array)
{
    for (int i = 0; i < GetCount(); ++i)
        if (m_elements[i]->m_bSelected) array.Add((TID*)m_elements[i]->m_data);
}

void CViewTree::Select(int elemNo, int sel)
{
    TElement* elem = m_elements[elemNo];
    if (int(elem->m_bSelected) == sel) return;
    elem->m_bSelected = sel == -1 ? !elem->m_bSelected : (sel != 0);
    CRect rect = GetElementRect(elemNo) - GetDeviceScrollPosition();
    InvalidateRect(rect);
}

void CViewTree::UnselectAll()
{
    for (int i = 0; i < GetCount(); ++i) Select(i, 0);
}

int CViewTree::LastSelected()
{
    int i = GetCount();
    while (--i >= 0)
        if (m_elements[i]->m_bSelected) break;
    return i;
}

int CViewTree::WhatSelected()
{
    int i = LastSelected();
    return i < 0 ? -1 : m_elements[i]->IsParent();
}

int CViewTree::FindItem(LPVOID data)
{
    int i = GetCount();
    while (--i >= 0)
        if (m_elements[i]->m_data == data) break;
    return i;
}

void CViewTree::GetParent(TElement*& elem)
{
    if (!elem->IsParent()) elem = elem->m_parent;
}

CViewTree::TElement* CViewTree::AddParent(LPVOID data)
{
    TElement* elem = new TElement(NULL, data);
    elem->m_bSelected = m_selectedSet.Check(data);
    elem->m_bExpanded = m_expandedSet.Check(m_bAltReset ? LPVOID(m_parentCount) : data);
    ++m_parentCount;
    m_elements.Add(elem);
    return elem;
}

CViewTree::TElement* CViewTree::AddChild(LPVOID data)
{
    int i = GetCount();
    ASSERT(!m_bSingleLevel && i > 0);
    TElement* parent = m_elements[i - 1];
    GetParent(parent);
    TElement* elem = new TElement(parent, data);
    elem->m_bSelected = parent->m_bExpanded && m_selectedSet.Check(data);
    m_elements.Add(elem);
    return elem;
}

void CViewTree::Expand(int elemNo, int exp)
{
    TElement* elem = m_elements[elemNo];
    if (!elem->IsParent()) return;
    if (int(elem->m_bExpanded) == exp) return;
    elem->m_bExpanded = exp == -1 ? !elem->m_bExpanded : (exp != 0);

    if (!elem->m_bExpanded)
        for (int i = elemNo + 1; i < GetCount(); ++i)
        {
            TElement* elem2 = m_elements[i];
            if (elem2->m_parent != elem) break;
            elem2->m_bSelected = false;
        }

    ScreenUpdate();
    return;
}

/////////////////////////////////////////////////////////////////////////////
// CViewTree drawing

bool CViewTree::GetNextActiveCell(TElemDraw& draw, int& start)
{
    while (true)
    {
        if (++draw.m_cell == GetCellCount()) return false;
        if (NeedDrawCell(draw) != ndcNo) break;
    }

    start = draw.m_cell;

    while (true)
    {
        if (draw.m_cell == GetCellCount() - 1) return true;
        if (NeedDrawCell(draw) == ndcYes) return true;
        ++draw.m_cell;
    }
}

// draw

CRect CViewTree::GetViewRect(CDC* pDC, const CRect& rectFrame, TDDSmart* pDD)
{
    CRect rect(rectFrame);
    CFont font;
    theApp.CreateSysFont(font, pDC);
    rect.top += Rint(GetFontHeight(pDC, &font) * 1.5);
    return rect;
}

void CViewTree::DrawHeader(THeader header, CDC* pDC, CSize sz, TDDSmart* pDD)
{
    TDDTree* pDD2 = (TDDTree*)pDD;
    if (header != Top) return;

    pDC->SetTextAlign(TA_CENTER | TA_TOP);
    CFont font;
    theApp.CreateSysFont(font, pDC);
    CFont* pFontOld = pDC->SelectObject(&font);

    CPen penLight(PS_SOLID, 1, colorLight());
    CPen penDark(PS_SOLID, 1, colorDark());
    CPen* pPenOld = pDC->SelectObject(&penDark);

    for (int j = 0; j < m_columns.GetSize(); ++j)
    {
        CRect rect2(0, 0, sz.cx, sz.cy);
        if (pDD->m_bPrinting)
        {
            if (j > 0) rect2.left = pDD2->m_arrTab[j - 1];
            rect2.right = pDD2->m_arrTab[j];
            OnDrawHeader(pDC, rect2, j);
        }
        else
        {
            rect2.left = j == 0 ? BlueMargin : pDD2->m_arrTab[j - 1];
            rect2.right = pDD2->m_arrTab[j];
            OnDrawHeader(pDC, rect2, j);

            pDC->SelectObject(&penLight);
            pDC->MoveTo(rect2.right - 1, rect2.top);
            pDC->LineTo(rect2.right - 1, rect2.bottom);

            pDC->SelectObject(&penDark);
            pDC->MoveTo(rect2.right + 1, rect2.top);
            pDC->LineTo(rect2.right + 1, rect2.bottom);
        }
    }

    pDC->SelectObject(pFontOld);
    pDC->SelectObject(pPenOld);
}

void CViewTree::DrawView(CDC* pDC, CSize sz, TDDSmart* pDD)
{
    TDDTree* pDD2 = (TDDTree*)pDD;

    if (pDD->m_bPrinting)
    {
        int y = 0;
        for (int i = m_arrPageStart[m_page - 1]; i < min(int(m_arrPageStart[m_page]), GetCount()); ++i)
            PrintElement(pDC, i, y, pDD2);
        return;
    }

    CPen penDark(PS_SOLID, 1, colorDark());
    CPen* pPenOld = pDC->SelectObject(&penDark);
    CRect clipRect;
    pDC->GetClipBox(&clipRect);

    for (int i = 0; i < GetCount(); ++i)
    {
        CRect elemRect = GetElementRect(i);
        CRect rect2(elemRect);
        rect2.left = 0;
        if ((rect2 & clipRect).IsRectEmpty()) continue;

        TElement* elem = m_elements[i];
        TElemDraw draw(elem);
        bool bSelected = elem->m_bSelected;
        int tabY = 0;

        // draw tab
        if (!m_bSingleLevel)
        {
            CRect tabRect = GetTabRect(i);
            CPoint center = tabRect.CenterPoint();
            bool bLast = i == GetCount() - 1 || m_elements[i + 1]->IsParent();

            tabY = center.y;
            if (draw.m_bParent)
            {
                pDC->Draw3dRect(&tabRect, colorDark(), colorDark());
                rect2 = tabRect;
                rect2.DeflateRect(2, 2);

                pDC->MoveTo(tabRect.right, tabY);
                pDC->LineTo(elemRect.left + BlueMargin, tabY);

                if (!bLast)
                {
                    pDC->MoveTo(rect2.left, tabY);
                    pDC->LineTo(rect2.right, tabY);

                    if (!elem->m_bExpanded)
                    {
                        pDC->MoveTo(center.x, rect2.top);
                        pDC->LineTo(center.x, rect2.bottom);
                    }
                    else
                    {
                        pDC->MoveTo(center.x, tabRect.bottom);
                        pDC->LineTo(center.x, elemRect.bottom);
                    }
                }
            }
            else
            {
                pDC->MoveTo(center);
                pDC->LineTo(elemRect.left + BlueMargin, tabY);
                pDC->MoveTo(center.x, elemRect.top);
                pDC->LineTo(center.x, bLast ? tabY : elemRect.bottom);
            }
        }

        // draw selection
        if (bSelected)
        {
            pDC->FillSolidRect(&elemRect, colorDark());
            tabY = 0;
        }

        COLORREF tl = colorLight(), br = colorDark();
        if (!draw.m_bParent)
        {
            COLORREF temp = tl;
            tl = br;
            br = temp;
        }

        // draw cells
        draw.m_cell = -1;
        int start = 0;
        while (GetNextActiveCell(draw, start))
        {
            rect2 = elemRect;
            rect2.left = start == 0 ? elemRect.left + BlueMargin : pDD2->m_arrTab[start - 1];
            rect2.right = pDD2->m_arrTab[draw.m_cell];
            rect2.DeflateRect(2, 2);
            --rect2.left;

            if (tabY)
            {
                pDC->MoveTo(elemRect.left, tabY);
                pDC->LineTo(rect2.left, tabY);
                tabY = 0;
            }

            pDC->FillSolidRect(rect2, colorNormal());
            pDC->Draw3dRect(rect2, tl, br);
            rect2.DeflateRect(1, 1);
            pDC->Draw3dRect(rect2, br, tl);

            rect2.DeflateRect(2, 2);
            DoDrawCell(pDC, rect2, draw);
        }
    }
    pDC->SelectObject(pPenOld);
}

void CViewTree::DoDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw)
{
    if (!IsCellValid(draw)) return;
    CFont font;
    GetCellFont(pDC, draw, font);
    CFont* pFontOld = pDC->SelectObject(&font);
    OnPrepareCell(pDC, draw);
    OnDrawCell(pDC, rect, draw);
    pDC->SelectObject(pFontOld);
}

void CViewTree::PrintElement(CDC* pDC, int line, int& y, TDDTree* pDD)
{
    TElement* elem = m_elements[line];
    TElemDraw draw(elem);

    int dy = draw.m_bParent ? m_prtY1 : m_prtY2;
    int beginX = 0;
    if (!draw.m_bParent) beginX += m_indentX;

    CRect rect(beginX, y, pDD->m_arrTab[GetCellCount() - 1], y + dy);
    if (!m_bSingleLevel && draw.m_bParent)
    {
        CBrush brush;
        brush.CreateSolidBrush(RGB(240, 240, 240));
        pDC->FillRect(rect, &brush);
    }

    draw.m_cell = -1;
    int start = 0;
    while (GetNextActiveCell(draw, start))
    {
        CRect rect2(rect);
        rect2.left = start == 0 ? beginX : pDD->m_arrTab[start - 1];
        rect2.right = pDD->m_arrTab[draw.m_cell];
        rect2.DeflateRect(m_interX, int(dy * 0.05));
        DoDrawCell(pDC, rect2, draw);
    }
    y += dy;
}

bool CViewTree::IsLineValid(int line)
{
    TElement* elem = m_elements[line];
    TElemDraw draw(elem);
    for (int j = 0; j < GetCellCount(); ++j)
    {
        draw.m_cell = j;
        if (!IsCellValid(draw)) return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CViewTree message handlers

CViewSmart::TDDSmart* CViewTree::CreateDD(bool bPrinting)
{
    return new TDDTree(bPrinting);
}

CSize CViewTree::OnScreenUpdate(CDC* pDC, TDDSmart* pDD)
{
    TDDTree* pDD2 = (TDDTree*)pDD;
    if (GetCellCount() == 0) return super::OnScreenUpdate(pDC, pDD);

    CRect rect;
    GetClientRect(rect);
    CalcTabs(pDC, rect.Width() - BlueMargin, ChildMargin + BlueMargin, pDD2);
    int maxX = pDD2->m_arrTab[GetCellCount() - 1] + BlueMargin;

    int y = 0;

    for (int i = 0; i < GetCount(); ++i)
    {
        TElement* elem = m_elements[i];
        elem->m_rect.SetRectEmpty();
    }

    int dy1 = CalcElementHeight(pDC, true) + 10;
    int dy2 = CalcElementHeight(pDC, false) + 10;

    for (int i = 0; i < GetCount(); ++i)
    {
        TElement* elem = m_elements[i];
        if (m_bSingleLevel)
        {
            elem->m_rect.SetRect(0, y, maxX, y + dy1);
            y += dy1;
        }
        else if (elem->IsParent())
        {
            elem->m_rect.SetRect(ParentMargin, y, maxX, y + dy1);
            y += dy1;
        }
        else if (elem->m_parent->m_bExpanded)
        {
            elem->m_rect.SetRect(ChildMargin, y, maxX, y + dy2);
            y += dy2;
        }
    }

    return CSize(maxX, y);
}

void CViewTree::Update()
{
    m_expandedSet.Clear();
    m_selectedSet.Clear();

    m_parentCount = 0;
    for (int i = 0; i < GetCount(); ++i)
    {
        TElement* elem = m_elements[i];
        if (elem->m_bSelected) m_selectedSet.Include(elem->m_data);
        if (!elem->IsParent()) continue;
        if (elem->m_bExpanded) m_expandedSet.Include(m_bAltReset ? LPVOID(m_parentCount) : elem->m_data);
        ++m_parentCount;
    }
    m_parentCount = 0;
    m_elements.DestroyAll();
    Fill();
    m_expandedSet.Clear();
    super::Update();
}

int CViewTree::CalcTabs(CDC* pDC, int width, int shift, TDDTree* pDD)
{
    int j, x = 0;

    width -= shift;
    pDD->m_arrTab.RemoveAll();

    for (j = 0; j < GetCellCount(); ++j)
    {
        CFont font1;
        theApp.CreateSysFont(font1, pDC);
        CFont font2;
        CreateCourierFont(font2, pDC);
        int x1 = GetFontSize(pDC, &font1, m_columns[j]->m_title).cx + 6;
        int x2 = GetFontSize(pDC, &font2, CString(_T('W'), m_columns[j]->m_width)).cx;
        x += max(x1, x2);
        pDD->m_arrTab.Add(x);
    }

    if (x < width)
    {
        double coef = min(double(width) / x, 2.);
        for (j = 0; j < GetCellCount(); ++j) pDD->m_arrTab[j] = int(pDD->m_arrTab[j] * coef);
        x = int(x * coef);
    }

    for (j = 0; j < GetCellCount(); ++j) pDD->m_arrTab[j] += shift;
    return x;
}

// Rect functions

int CViewTree::CalcElementHeight(CDC* pDC, bool bParent)
{
    TElemDraw draw(bParent);
    int y = 0;

    for (int j = 0; j < GetCellCount(); ++j)
    {
        draw.m_cell = j;
        CFont font;
        GetCellFont(pDC, draw, font);
        y = max(y, int(GetFontHeight(pDC, &font)));
    }
    return y;
}

const CRect& CViewTree::GetElementRect(int elemNo)
{
    return m_elements[elemNo]->m_rect;
}

CRect CViewTree::GetTabRect(int elemNo)
{
    if (m_bSingleLevel) return CRect(0, 0, 0, 0);

    CRect elemRect = GetElementRect(elemNo);
    CRect marRect(0, elemRect.top, ParentMargin, elemRect.bottom);
    CPoint center = marRect.CenterPoint();

    int sz = min(marRect.Width(), marRect.Height()) / 2;
    if (!(sz % 2)) ++sz;
    return CRect(center - CSize(sz / 2, sz / 2), CSize(sz, sz));
}

// overridables

void CViewTree::OnDrawHeader(CDC* pDC, const CRect& rect, int cell)
{
    TextInRect(pDC, rect, m_columns[cell]->m_title);
}

void CViewTree::OnDrawCell(CDC* pDC, const CRect& rect, TElemDraw& draw)
{
    CString s = draw.m_data ? GetCellString(draw) : m_columns[draw.m_cell]->m_title;
    TextInRect(pDC, rect, s);
}

void CViewTree::GetCellFont(CDC* pDC, TElemDraw& draw, CFont& font)
{
    theApp.CreateAppFont(font, pDC, draw.m_bParent ? 1.3 : 1., draw.m_cell < 1, false);
}

void CViewTree::OnPrepareCell(CDC* pDC, TElemDraw& draw)
{
    pDC->SetTextAlign(TA_TOP | (draw.m_cell < 1 ? TA_LEFT : TA_CENTER));
    pDC->SetTextColor(pDC->IsPrinting() ? colorBlack : colorText());
    pDC->SetBkMode(TRANSPARENT);
}

// main overridables

void CViewTree::Fill()
{}

bool CViewTree::IsCellValid(TElemDraw& draw)
{
    return true;
}

CString CViewTree::GetCellString(TElemDraw& draw)
{
    return strEmpty;
}

CViewTree::TDrawCell CViewTree::NeedDrawCell(TElemDraw& draw)
{
    return ndcYes;
}

void CViewTree::ClickElem(UINT nFlags, int elemNo)
{}

void CViewTree::DblClickElem(UINT nFlags, int elemNo)
{}

void CViewTree::OnDelete()
{
    LekinBeep();
}

// message handlers

void CViewTree::Click(UINT nFlags, CPoint point)
{
    for (int i = 0; i < GetCount(); ++i)
    {
        TElement* elem = m_elements[i];
        CRect rect = GetElementRect(i);

        if (rect.PtInRect(point))
        {
            int what = WhatSelected();
            if (m_bSingleSel) goto SINGLE;

            if ((nFlags & MK_CONTROL) && what != 1 - elem->IsParent())
                Select(i, -1);
            else if (nFlags & MK_SHIFT)
            {
                if (elem->m_bSelected) return;
                if (what == -1)
                {
                    Select(i, -1);
                    return;
                }

                int index1 = LastSelected(), index2 = i;
                if (index2 < index1)
                {
                    int temp = index2;
                    index2 = index1;
                    index1 = temp;
                }

                for (int k = index1; k <= index2; ++k)
                    if (int(m_elements[k]->IsParent()) == what)
                    {
                        if (GetElementRect(k).IsRectEmpty()) continue;
                        Select(k, 1);
                    }
            }
            else
            {
            SINGLE:
                UnselectAll();
                Select(i, 1);
            }

            if (IsLineValid(i)) ClickElem(nFlags, i);
            return;
        }

        rect = GetTabRect(i);
        if (rect.PtInRect(point))
        {
            Expand(i, -1);
            return;
        }
    }
}

void CViewTree::DblClick(UINT nFlags, CPoint point)
{
    for (int i = 0; i < GetCount(); ++i)
    {
        CRect rect = GetElementRect(i);
        if (rect.PtInRect(point))
        {
            if (IsLineValid(i)) DblClickElem(nFlags, i);
            return;
        }
    }
}

// Printing

int CViewTree::OnPrintUpdate(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    TDDTree* pDD2 = (TDDTree*)pDD;
    int maxX = CalcTabs(pDC, m_rectPage.Width(), 0, pDD2);
    if (maxX > m_rectPage.Width()) return -1;

    m_indentX = pDD2->m_arrTab[0] / 5;
    m_interX = pDD2->m_arrTab[0] / 30;

    m_prtY1 = int(CalcElementHeight(pDC, true) * 1.2);
    m_prtY2 = int(CalcElementHeight(pDC, false) * 1.2);

    m_arrPageStart.RemoveAll();
    m_arrPageStart.SetSize(0, 16);
    m_arrPageStart.Add(0);

    int y = m_rectCenter.top;
    int lastParent = 0;
    int lastBreak = 0;
    int i = 0;

    for (;; ++i)
    {
        if (i > lastBreak + 1 && y > m_rectCenter.bottom)
        {
            y = m_rectCenter.top;
            lastBreak = lastParent > lastBreak ? lastParent : i - 1;
            m_arrPageStart.Add(lastBreak);
            i = lastParent = lastBreak;
        }

        if (i == GetCount()) break;
        if (m_elements[i]->IsParent())
        {
            lastParent = i;
            y += m_prtY1;
        }
        else
            y += m_prtY2;
    }
    m_arrPageStart.Add(i);
    return m_arrPageStart.GetSize() - 1;
}

int CViewTree::GetPrintYBottom(CDC* pDC, TDDSmart* pDD, const TMargin* pMargin)
{
    return m_yTop;
}

void CViewTree::PrintBottom(CDC* pDC, TDDSmart* pDD)
{
    PrintPageNo(pDC);
}

void CViewTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    int what = WhatSelected();
    int last = LastSelected();
    int count = GetCount();
    int dy = 0;

    switch (nChar)
    {
        case VK_UP:
            if (last == -1) last = count;
            dy = -1;
            goto MOVE;

        case VK_DOWN:
            dy = 1;
        MOVE:
            while (true)
            {
                last += dy;

                if (last < 0 || last >= count)
                {
                    LekinBeep();
                    return;
                }

                if (!GetElementRect(last).IsRectEmpty()) break;
            }

            UnselectAll();
            Select(last, 1);
            if (IsLineValid(last)) ClickElem(nFlags, last);
            break;

        case VK_DELETE:
            if (what == -1)
                LekinBeep();
            else
                OnDelete();
            break;

        case VK_RETURN:
            if (what == -1)
                LekinBeep();
            else
            {
                if (IsLineValid(last)) DblClickElem(nFlags, last);
            }
            break;

        default:
            super::OnKeyDown(nChar, nRepCnt, nFlags);
            break;
    }
}
