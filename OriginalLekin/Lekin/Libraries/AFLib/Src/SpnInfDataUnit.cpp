#include "StdAfx.h"

#include "AFLibSpnInfDataUnit.h"

#include "AFLibMathGlobal.h"
#include "AFLibSpinInfinity.h"
#include "AFLibStrings.h"

using namespace AFLib;
using namespace AFLibMath;
using AFLibGui::TSpnInfDataUnit;

TSpnInfDataUnit::TSpnInfDataUnit(
    LPCTSTR key, UINT idc, UINT idcSpin, int& data, int init, int zero, int low, int high, UINT idsHelp) :
    super(key, idc, data, init, zero, low, high, idsHelp),
    m_idcSpin(idcSpin)
{}

void TSpnInfDataUnit::DDX(CDataExchange* pDX, bool bUndefOk) const
{  // Use dynamic_cast here!  User must define a CSpinInfinity object
    // inside the dialog object and link (subclass) it to the spin control.

    CSpinInfinity* pWndSpin = dynamic_cast<CSpinInfinity*>(pDX->m_pDlgWnd->GetDlgItem(m_idcSpin));
    ASSERT(pWndSpin != NULL);

    if (pDX->m_bSaveAndValidate)
    {
        CString s;
        pDX->m_pDlgWnd->GetDlgItemText(m_idc, s);
        if (bUndefOk && s.IsEmpty())
            m_data = NanI;
        else if (s == CSpinInfinity::strInfinity)
            m_data = INT_MAX;
        else
        {
            DDX_Text(pDX, m_idc, m_data);
            DDV_MinMaxInt(pDX, m_data, m_low, m_high);
        }
    }
    else
    {
        pWndSpin->SetRange32(m_low, m_high);
        if (m_data == NanI)
            pDX->m_pDlgWnd->SetDlgItemText(m_idc, strEmpty);
        else if (m_data == INT_MAX)
        {
            pDX->m_pDlgWnd->SetDlgItemText(m_idc, CSpinInfinity::strInfinity);
            pWndSpin->SetPos32(m_high);
        }
        else
        {
            pDX->m_pDlgWnd->SetDlgItemInt(m_idc, m_data);
            pWndSpin->SetPos32(m_data);
        }
    }
}
