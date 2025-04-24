#include "StdAfx.h"

#include "AFLibRadDataUnit.h"

#include "AFLibMathGlobal.h"

using AFLibGui::TRadDataUnit;

TRadDataUnit::TRadDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, int count, UINT idsHelp) :
    super(key, idc, data, init, zero, count, idsHelp)
{}

bool TRadDataUnit::PeerNeeded(int index) const
{
    return m_data == index;
}

void TRadDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_Radio(pDX, m_idc, m_data);
    if (bUndefOk && pDX->m_bSaveAndValidate && m_data < 0) m_data = AFLibMath::NanI;
}
