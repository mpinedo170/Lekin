#include "StdAfx.h"

#include "AFLibIntDataUnit.h"

#include "AFLibMathGlobal.h"
#include "AFLibStrings.h"

using namespace AFLib;
using namespace AFLibMath;
using AFLibGui::TIntDataUnit;

TIntDataUnit::TIntDataUnit(
    LPCTSTR key, UINT idc, int& data, int init, int zero, int low, int high, UINT idsHelp) :
    super(key, idc, data, init, zero, 1, idsHelp),
    m_low(low),
    m_high(high)
{}

void TIntDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{
    if (bUndefOk)
    {
        if (pDX->m_bSaveAndValidate)
        {
            CString s;
            pDX->m_pDlgWnd->GetDlgItemText(m_idc, s);
            if (s.IsEmpty())
            {
                m_data = NanI;
                return;
            }
        }
        else
        {
            if (IsNan(m_data))
            {
                pDX->m_pDlgWnd->SetDlgItemText(m_idc, strEmpty);
                return;
            }
        }
    }

    DDX_Text(pDX, m_idc, m_data);
    DDV_MinMaxInt(pDX, m_data, m_low, m_high);
}
