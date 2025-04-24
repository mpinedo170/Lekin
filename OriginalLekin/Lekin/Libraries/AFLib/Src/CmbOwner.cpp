#include "StdAfx.h"

#include "AFLibCmbOwner.h"

#include "AFLibStrings.h"

using namespace AFLib;
using AFLibGui::CCmbOwner;

IMPLEMENT_DYNAMIC(CCmbOwner, super)

CCmbOwner::CCmbOwner() : m_draw(ODT_COMBOBOX)
{}

void CCmbOwner::ResetItemHeight()
{
    LPMEASUREITEMSTRUCT pMIS = m_draw.GetMIS();
    MeasureItem(pMIS);
    SetItemHeight(0, pMIS->itemHeight);
}

int CCmbOwner::AddData(LPVOID data)
{
    int index = AddString(strEmpty);
    SetItemDataPtr(index, data);
    return index;
}

LPVOID CCmbOwner::GetData()
{
    int i = GetCurSel();
    return i < 0 ? NULL : GetItemDataPtr(i);
}

void CCmbOwner::SelString(LPCTSTR data)
{
    int i = GetCount();
    while (--i >= 0)
    {
        CString s;
        GetLBText(i, s);
        if (s == data) break;
    }
    SetCurSel(i);
}

void CCmbOwner::SelData(LPVOID data)
{
    int i = GetCount();
    while (--i >= 0)
        if (GetItemDataPtr(i) == data) break;
    SetCurSel(i);
}

BEGIN_MESSAGE_MAP(CCmbOwner, super)
ON_WM_CREATE()
END_MESSAGE_MAP()

void CCmbOwner::PreSubclassWindow()
{
    super::PreSubclassWindow();
    OnInit();
}

int CCmbOwner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (super::OnCreate(lpCreateStruct) == -1) return -1;
    OnInit();
    return 0;
}

void CCmbOwner::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    m_draw.StartDraw(lpDrawItemStruct);

    try
    {
        GetItemText();
        m_draw.StartRegion();
        Draw(&m_draw.m_dc);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    m_draw.Finish();
}

void CCmbOwner::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    m_draw.StartMeasure(m_hWnd, lpMeasureItemStruct);

    try
    {
        GetItemText();
        lpMeasureItemStruct->itemHeight = Measure(lpMeasureItemStruct->itemID);
    }
    catch (CException* pExc)
    {
        pExc->Delete();
    }

    m_draw.Finish();
}

int CCmbOwner::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
    return m_draw.CompareItems(lpCompareItemStruct);
}

void CCmbOwner::GetItemText()
{
    m_draw.GetItemText();
}

void CCmbOwner::Draw(CDC* pDC)
{
    CUIntArray arrTab;
    m_draw.Draw(arrTab);
}

int CCmbOwner::Measure(int index)
{
    return m_draw.Measure().cy;
}

void CCmbOwner::OnInit()
{
    if (GetStyle() & CBS_OWNERDRAWFIXED)
        ResetItemHeight();
    else
        ASSERT(GetStyle() & CBS_OWNERDRAWVARIABLE);
}
