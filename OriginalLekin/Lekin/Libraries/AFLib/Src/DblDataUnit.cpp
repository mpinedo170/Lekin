#include "StdAfx.h"

#include "AFLibDblDataUnit.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::TDblDataUnit;

TDblDataUnit::TDblDataUnit(
    LPCTSTR key, UINT idc, double& data, double init, double zero, double low, double high, UINT idsHelp) :
    super(key, idc, data, init, zero, idsHelp),
    m_low(low),
    m_high(high)
{}

void TDblDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    if (DDX_EmptyDbl(pDX, m_idc, m_data, bUndefOk)) return;

    DDX_Text(pDX, m_idc, m_data);
    DDV_MinMaxDouble(pDX, m_data, m_low, m_high);
}
