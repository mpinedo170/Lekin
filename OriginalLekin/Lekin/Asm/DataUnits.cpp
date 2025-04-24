#include "StdAfx.h"

#include "AppAsm.h"
#include "DataUnits.h"

#include "Misc.h"

TIDDataUnit::TIDDataUnit(LPCTSTR key, UINT idc, CString& data, LPCTSTR init, UINT ids2) :
    super(key, idc, data, init, strEmpty, 1, MAX_ID),
    m_ids2(ids2)
{}

void TIDDataUnit::Load(LPCTSTR section) const
{
    super::Load(section);
    AlterName(m_data, true);
}

void TIDDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    super::DDX(pDX, bUndefOk);
    DDV(pDX, bUndefOk);
}

void TIDDataUnit::DDV(CDataExchange* pDX, bool bUndefOk) const
{
    if (!pDX->m_bSaveAndValidate) return;
    if (bUndefOk && m_data.IsEmpty()) return;
    CheckID(m_data, m_ids2);
}

TCbsIDDataUnit::TCbsIDDataUnit(LPCTSTR key, UINT idc, CString& data, UINT ids2) :
    super(key, idc, data, strEmpty, ids2)
{}

void TCbsIDDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_CBString(pDX, m_idc, m_data);
    DDV(pDX, bUndefOk);
}

TCountDataUnit::TCountDataUnit(
    LPCTSTR key, UINT idc, UINT idcSpin, int& data, int init, int maxCount, int curCount, UINT ids2) :
    super(key, idc, idcSpin, data, init, 0, 1, maxCount - curCount),
    m_maxCount(maxCount),
    m_curCount(curCount),
    m_ids2(ids2)
{
    m_data = 0;
}

void TCountDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    if (!pDX->m_bSaveAndValidate)
    {
        super::DDX(pDX, bUndefOk);
        return;
    }

    DDX_Text(pDX, m_idc, m_data);
    if (m_data >= 1 && m_data <= m_maxCount - m_curCount) return;

    ThrowTooMany(m_maxCount, m_ids2);
}

TStatusDataUnit::TStatusDataUnit(LPCTSTR key, UINT idc, CString& data) :
    super(key, idc, data, _T("A"), _T("A"), 0)
{}

void TStatusDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    DDX_Text(pDX, m_idc, m_data);
    if (!pDX->m_bSaveAndValidate) return;

    if (bUndefOk && m_data.IsEmpty()) return;
    CheckStatus(m_data);
}
