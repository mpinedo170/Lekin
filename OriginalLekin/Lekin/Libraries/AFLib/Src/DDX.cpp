#include "StdAfx.h"

#include "AFLib3State.h"
#include "AFLibGuiGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringParser.h"
#include "Local.h"

using namespace AFLibMath;
using namespace AFLibPrivate;
using AFLib::T3State;

namespace AFLibGui {
void DDX_Check(CDataExchange* pDX, UINT idc, bool& value)
{
    if (pDX->m_pDlgWnd->GetDlgItem(idc) == NULL) return;
    int t = value;
    DDX_Check(pDX, idc, t);
    value = t != 0;
}

void DDX_Check(CDataExchange* pDX, UINT idc, T3State& value)
{
    if (pDX->m_pDlgWnd->GetDlgItem(idc) == NULL) return;
    int t = value.ToInt();
    DDX_Check(pDX, idc, t);
    value = T3State(t, 0);
}

void DDX_Dir(CDataExchange* pDX, UINT idc, CString& value, bool bCreateOk, bool bUndefOk)
{
    DDX_Text(pDX, idc, value);
    if (!pDX->m_bSaveAndValidate) return;

    if (value.IsEmpty())
    {
        if (bUndefOk) return;
        LocalAfxMB(IDS_EMPTY_DIR);
        pDX->Fail();
    }

    if (!AFLibIO::CheckDirectory(value, bCreateOk))
    {
        LocalAfxMB(IDS_INVALID_DIR);
        pDX->Fail();
    }
}

void DDX_Mask(CDataExchange* pDX, UINT idc, int count, int& value, int errorID)
{
    pDX->PrepareCtrl(idc);
    if (!pDX->m_bSaveAndValidate)
    {
        for (int i = 0; i < count; ++i) pDX->m_pDlgWnd->CheckDlgButton(idc + i, (value & (1 << i)) != 0);
        return;
    }

    value = 0;
    for (int i = 0; i < count; ++i)
        if (pDX->m_pDlgWnd->IsDlgButtonChecked(idc + i)) value |= 1 << i;
    if (!errorID) return;

    if (value == 0)
    {
        AfxMB(errorID);
        pDX->Fail();
    }
}

bool DDX_EmptyDbl(CDataExchange* pDX, UINT idc, double& value, bool bUndefOk)
{
    CString s;

    if (pDX->m_bSaveAndValidate)
    {
        if (bUndefOk)
        {
            value = Nan;
            DDX_Text(pDX, idc, s);
            if (s.IsEmpty()) return true;
        }
    }
    else if (IsNan(value))
    {
        DDX_Text(pDX, idc, s);
        return true;
    }

    return false;
}

void DDX_Percentage(CDataExchange* pDX, UINT idc, double& value)
{
    double val2 = Round(value * Const100, 6);
    DDX_Text(pDX, idc, val2);
    if (pDX->m_bSaveAndValidate) value = val2 / Const100;
}

void DDV_MinMaxPercentage(CDataExchange* pDX, double const& value, double minVal, double maxVal)
{
    double val2 = Round(value * Const100, 6);
    DDV_MinMaxDouble(pDX, val2, minVal * Const100, maxVal * Const100);
}

void DDX_SpinMinMax(CDataExchange* pDX, UINT idc, int minVal, int maxVal)
{
    if (pDX->m_bSaveAndValidate) return;

    CWnd* pWndSpin = pDX->m_pDlgWnd->GetDlgItem(idc);
    if (pWndSpin == NULL) return;

    // dynamic_cast is not used here
    // since CWnd::GetDlgItem() may return CWnd*, not CSpinButtonCtrl*

    ((CSpinButtonCtrl*)pWndSpin)->SetRange32(minVal, maxVal);
}

void DDX_CBData(CDataExchange* pDX, UINT idc, int& value)
{  // dynamic_cast is not used here
    // since CWnd::GetDlgItem() may return CWnd*, not CComboBox*

    CComboBox* pCmb = (CComboBox*)pDX->m_pDlgWnd->GetDlgItem(idc);
    ASSERT(pCmb != NULL);

    if (pDX->m_bSaveAndValidate)
    {
        int index = pCmb->GetCurSel();
        value = index < 0 ? AFLibMath::NanI : pCmb->GetItemData(index);
    }
    else
    {
        int index = pCmb->GetCount();
        while (--index >= 0)
            if (pCmb->GetItemData(index) == value) break;
        pCmb->SetCurSel(index);
    }
}
}  // namespace AFLibGui
