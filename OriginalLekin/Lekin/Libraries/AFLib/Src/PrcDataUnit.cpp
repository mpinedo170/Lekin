#include "StdAfx.h"

#include "AFLibPrcDataUnit.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::TPrcDataUnit;

TPrcDataUnit::TPrcDataUnit(
    LPCTSTR key, UINT idc, double& data, double init, double zero, double low, double high, UINT idsHelp) :
    super(key, idc, data, init, zero, idsHelp),
    m_low(low),
    m_high(high)
{}

void TPrcDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    if (DDX_EmptyDbl(pDX, m_idc, m_data, bUndefOk)) return;

    DDX_Percentage(pDX, m_idc, m_data);
    DDV_MinMaxPercentage(pDX, m_data, m_low, m_high);
}
