#include "StdAfx.h"

#include "AFLibMskDataUnit.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::TMskDataUnit;

TMskDataUnit::TMskDataUnit(
    LPCTSTR key, UINT idc, int& data, int init, int zero, int count, UINT errorID, UINT idsHelp) :
    super(key, idc, data, init, zero, count, idsHelp),
    m_errorID(errorID)
{}

bool TMskDataUnit::PeerNeeded(int index) const
{
    return (m_data & (1 << index)) != 0;
}

void TMskDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_Mask(pDX, m_idc, m_nIdc, m_data, m_errorID);
}

bool TMskDataUnit::IsUndefined() const
{
    return false;
}
