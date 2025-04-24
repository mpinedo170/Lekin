#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgStyle.h"

#include "DocTmpSmart.h"

#include "DataUnits.h"
#include "Misc.h"
#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgStyle dialog

CDlgStyle::CDlgStyle(const TID& style, int time) : super(CDlgStyle::IDD), m_style(style), m_time(time)
{
    m_bInUpdate = false;
    m_idOld = ID();

    AddDU(new TIDDataUnit(strEmpty, IDC_ID, ID(), ID(), IDS_SCH));
    AddDU(new TIntDataUnit(strEmpty, IDC_TIME, m_time, m_time, 0, 1, MX9));
}

void CDlgStyle::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_ID, m_wndID);
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;

    if (!equals(ID(), m_idOld))
    {
        if (arrSchedule.Exists(ID())) ReportDupID(pDX, IDC_ID, IDS_SCH);
    }
}

BEGIN_MESSAGE_MAP(CDlgStyle, super)
ON_WM_PAINT()
ON_NOTIFY_RANGE(NM_CUSTOMDRAW, IDC_RGB_SLIDER0, IDC_RGB_SLIDER0 + 2, OnSlide)
ON_CONTROL_RANGE(EN_CHANGE, IDC_RGB_EDIT0, IDC_RGB_EDIT0 + 2, OnEdit)
END_MESSAGE_MAP()

void CDlgStyle::SetRGB(int i, int val)
{
    m_style.m_color &= ~(0xFF << i * 8);
    m_style.m_color |= val << i * 8;
    InvalidateRect(&m_sampRect, false);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgStyle message handlers

BOOL CDlgStyle::OnInitDialog()
{
    super::OnInitDialog();
    SetStdLimitText();

    SetWindowText(m_style.GetToken() + _T(" Style"));
    SetDlgItemText(IDC_LAB0, m_style.GetToken() + _T(" ID"));
    if (m_style.m_ty != tySch)
    {
        static const UINT arrIDC[] = {IDC_LAB1, IDC_TIME, 0};
        m_wndID.SetReadOnly(true);
        ShowDlgItems(arrIDC, false);
    }

    m_bInUpdate = true;
    for (int i = 0; i < 3; ++i)
    {
        m_sldRGB[i].SubclassDlgItem(IDC_RGB_SLIDER0 + i, this);
        m_editRGB[i].SubclassDlgItem(IDC_RGB_EDIT0 + i, this);

        m_sldRGB[i].SetRange(0, 255);
        m_editRGB[i].SetLimitText(3);

        int val = (m_style.m_color >> i * 8) & 0xFF;
        m_editRGB[i].SetWindowText(IntToStr(val));
        m_sldRGB[i].SetPos(val);
    }
    m_bInUpdate = false;

    GetDlgItem(IDC_SAMPLE)->GetWindowRect(&m_sampRect);
    ScreenToClient(m_sampRect);
    m_sampRect.top += 15;
    m_sampRect.DeflateRect(10, 10);

    m_wndID.SetFocus();
    m_wndID.SetSel(0, 0);
    return false;
}

void CDlgStyle::OnPaint()
{
    CPaintDC dc(this);
    m_style.DrawRect(&dc, m_sampRect, strEmpty);
}

void CDlgStyle::OnSlide(UINT id, NMHDR*, LRESULT*)
{
    if (m_bInUpdate) return;
    m_bInUpdate = true;

    int i = id - IDC_RGB_SLIDER0;
    int val = m_sldRGB[i].GetPos();
    m_editRGB[i].SetWindowText(IntToStr(val));

    SetRGB(i, val);
    m_bInUpdate = false;
}

void CDlgStyle::OnEdit(UINT id)
{
    if (m_bInUpdate) return;
    m_bInUpdate = true;

    int i = id - IDC_RGB_EDIT0;
    CString s;
    GetDlgItemText(id, s);
    int val = StrToInt(s);
    if (val < 0 || val > 255) goto Ret;

    m_sldRGB[i].SetPos(val);
    SetRGB(i, val);
Ret:
    m_bInUpdate = false;
}

void CDlgStyle::OnOK()
{
    int gray = m_style.GetGray();
    if (gray > 240 || gray < 20)
    {
        AfxMB(IDP_BAD_COLOR);
        return;
    }
    super::OnOK();
}

bool CDlgStyle::EditColor(TID* pStyle)
{
    CDlgStyle dlg(*pStyle, 1);
    if (dlg.DoModal() != IDOK) return false;
    *pStyle = dlg.m_style;
    return true;
}

bool CDlgStyle::EditSch(TSchedule* pSch)
{
    int& time = pSch->m_arrObj[objTime];
    CDlgStyle dlg(*pSch, time);
    if (dlg.DoModal() != IDOK) return false;
    *static_cast<TID*>(pSch) = dlg.m_style;
    time = dlg.m_time;
    return true;
}

bool IsObjMax(TObjective obj)
{
    return obj != objTime && obj <= objLmax;
}

bool IsObjSum(TObjective obj)
{
    return obj != objTime && !IsObjMax(obj);
}

bool IsObjWeighted(TObjective obj)
{
    return obj >= objWCj;
}
