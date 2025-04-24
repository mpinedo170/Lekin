#include "StdAfx.h"

#include "AFLibMargin.h"

#include "AFLibGlobal.h"

using AFLibGui::TMargin;
using namespace AFLib;

namespace {
const CString keyLeft = _T("Left");
const CString keyRight = _T("Right");
const CString keyTop = _T("Top");
const CString keyBottom = _T("Bottom");
const CString keyLandscape = _T("Landscape");
const int defaultMargin = 1000;  // 1 inch
}  // namespace

IMPLEMENT_DYNCREATE(TMargin, super)

TMargin::TMargin()
{
    PrivateInit();
}

TMargin::TMargin(LPCTSTR section) : m_section(section)
{
    PrivateInit();
}

TMargin::~TMargin()
{}

void TMargin::Copy(const TMargin* pMargin)
{
    m_section = pMargin->m_section;
    m_left = pMargin->m_left;
    m_right = pMargin->m_right;
    m_top = pMargin->m_top;
    m_bottom = pMargin->m_bottom;
    m_bLandscape = pMargin->m_bLandscape;
    m_bSelection = pMargin->m_bSelection;
}

void TMargin::PrivateInit()
{
    m_left = m_right = m_top = m_bottom = defaultMargin;
    m_bLandscape = m_bSelection = false;
}

TMargin* TMargin::Clone() const
{
    TMargin* pMargin = dynamic_cast<TMargin*>(GetRuntimeClass()->CreateObject());
    pMargin->Copy(this);
    return pMargin;
}

void TMargin::LoadReg()
{
    ASSERT(!m_section.IsEmpty());
    m_left = DynamicGetProfileInt(m_section, keyLeft, m_left);
    m_right = DynamicGetProfileInt(m_section, keyRight, m_right);
    m_top = DynamicGetProfileInt(m_section, keyTop, m_top);
    m_bottom = DynamicGetProfileInt(m_section, keyBottom, m_bottom);
    m_bLandscape = DynamicGetProfileInt(m_section, keyLandscape, m_bLandscape) != 0;
}

void TMargin::SaveReg() const
{
    ASSERT(!m_section.IsEmpty());
    DynamicWriteProfileInt(m_section, keyLeft, m_left);
    DynamicWriteProfileInt(m_section, keyRight, m_right);
    DynamicWriteProfileInt(m_section, keyTop, m_top);
    DynamicWriteProfileInt(m_section, keyBottom, m_bottom);
    DynamicWriteProfileInt(m_section, keyLandscape, m_bLandscape);
}

void TMargin::Load(LPPAGESETUPDLGW pPageSetup)
{
    m_left = pPageSetup->rtMargin.left;
    m_right = pPageSetup->rtMargin.right;
    m_top = pPageSetup->rtMargin.top;
    m_bottom = pPageSetup->rtMargin.bottom;

    if (pPageSetup->hDevMode == NULL) return;
    LPDEVMODEW lpDevMode = LPDEVMODEW(GlobalLock(pPageSetup->hDevMode));
    m_bLandscape = lpDevMode->dmOrientation == DMORIENT_LANDSCAPE;
    GlobalUnlock(pPageSetup->hDevMode);
}

void TMargin::Save(LPPAGESETUPDLGW pPageSetup) const
{
    pPageSetup->rtMargin.left = m_left;
    pPageSetup->rtMargin.right = m_right;
    pPageSetup->rtMargin.top = m_top;
    pPageSetup->rtMargin.bottom = m_bottom;

    if (pPageSetup->hDevMode == NULL) return;
    LPDEVMODEW lpDevMode = LPDEVMODEW(GlobalLock(pPageSetup->hDevMode));
    lpDevMode->dmOrientation = m_bLandscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
    GlobalUnlock(pPageSetup->hDevMode);
}

CRect TMargin::ComputeRect(CDC* pDC) const
{
    int xScale = pDC->GetDeviceCaps(LOGPIXELSX);
    int yScale = pDC->GetDeviceCaps(LOGPIXELSY);

    return CRect(m_left * xScale / 1000, m_top * yScale / 1000,
        pDC->GetDeviceCaps(HORZRES) - m_right * xScale / 1000,
        pDC->GetDeviceCaps(VERTRES) - m_bottom * yScale / 1000);
}
