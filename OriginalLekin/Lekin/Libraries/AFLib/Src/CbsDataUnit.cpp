#include "StdAfx.h"

#include "AFLibCbsDataUnit.h"

using AFLibGui::TCbsDataUnit;

TCbsDataUnit::TCbsDataUnit(LPCTSTR key, UINT idc, CString& data, LPCTSTR init, LPCTSTR zero, UINT idsHelp) :
    super(key, idc, data, init, zero, idsHelp)
{}

void TCbsDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_CBString(pDX, m_idc, m_data);
}
