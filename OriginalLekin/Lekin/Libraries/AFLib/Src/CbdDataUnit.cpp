#include "StdAfx.h"

#include "AFLibCbdDataUnit.h"

#include "AFLibGuiGlobal.h"

using AFLibGui::TCbdDataUnit;

TCbdDataUnit::TCbdDataUnit(LPCTSTR key, UINT idc, int& data, int init, int zero, UINT idsHelp) :
    super(key, idc, data, init, zero, 1, idsHelp)
{}

void TCbdDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_CBData(pDX, m_idc, m_data);
}
