#include "StdAfx.h"

#include "AFLibSpnDataUnit.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::TSpnDataUnit;

TSpnDataUnit::TSpnDataUnit(
    LPCTSTR key, UINT idc, UINT idcSpin, int& data, int init, int zero, int low, int high, UINT idsHelp) :
    super(key, idc, data, init, zero, low, high, idsHelp),
    m_idcSpin(idcSpin)
{}

void TSpnDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    super::DDX(pDX, bUndefOk);
    if (m_idcSpin != 0) DDX_SpinMinMax(pDX, m_idcSpin, m_low, m_high);
}
