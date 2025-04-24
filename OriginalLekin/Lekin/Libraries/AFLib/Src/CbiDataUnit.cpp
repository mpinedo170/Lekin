#include "StdAfx.h"

#include "AFLibCbiDataUnit.h"

#include "AFLibMathGlobal.h"

using AFLibGui::TCbiDataUnit;

TCbiDataUnit::TCbiDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, UINT idsHelp) :
    super(key, idc, data, init, zero, 1, idsHelp)
{}

void TCbiDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_CBIndex(pDX, m_idc, m_data);
    if (bUndefOk && pDX->m_bSaveAndValidate && m_data < 0) m_data = AFLibMath::NanI;
}
