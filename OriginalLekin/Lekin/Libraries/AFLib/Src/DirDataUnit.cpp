#include "StdAfx.h"

#include "AFLibDirDataUnit.h"
#include "AFLibGuiGlobal.h"

using AFLibGui::TDirDataUnit;

TDirDataUnit::TDirDataUnit(
    LPCTSTR key, UINT idc, CString& data, LPCTSTR init, LPCTSTR zero, bool bCreateOk, UINT idsHelp) :
    super(key, idc, data, init, zero, idsHelp),
    m_bCreateOk(bCreateOk)
{}

void TDirDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_Dir(pDX, m_idc, m_data, m_bCreateOk, bUndefOk);
}
