#include "StdAfx.h"

#include "AFLibStrDataUnit.h"

#include "AFLibGlobal.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibPrivate;
using AFLibGui::TStrDataUnit;

TStrDataUnit::TStrDataUnit(LPCTSTR key,
    UINT idc,
    CString& data,
    LPCTSTR init,
    LPCTSTR zero,
    int low,
    int high,
    bool bTrim,
    UINT idsHelp) :
    super(key, idc, data, init, zero, idsHelp),
    m_low(low),
    m_high(high),
    m_bTrim(bTrim)
{}

void TStrDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    if (pDX->m_bSaveAndValidate)
    {
        pDX->m_pDlgWnd->GetDlgItemText(m_idc, m_data);
        if (pDX->m_bSaveAndValidate) m_data.Trim();

        DDV_MaxChars(pDX, m_data, m_high);
        if (bUndefOk && m_data.IsEmpty()) return;

        if (m_data.GetLength() < m_low)
        {
            if (m_low == 1)
                LocalAfxMB(IDS_STRING_EMPTY);
            else
                LocalAfxMB(IDS_STRING_TOO_SHORT, IntToStr(m_low));
            pDX->Fail();
        }
    }
    else if (m_bTrim)
    {
        CString s = m_data;
        s.Trim();
        pDX->m_pDlgWnd->SetDlgItemText(m_idc, s);
    }
    else
        pDX->m_pDlgWnd->SetDlgItemText(m_idc, m_data);
}
