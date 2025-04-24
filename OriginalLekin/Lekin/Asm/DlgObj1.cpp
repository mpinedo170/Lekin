#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgObj1.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgObj1 dialog

CDlgObj1::CDlgObj1(int mask) : super(CDlgObj1::IDD), m_mask(mask)
{
    m_section = secHeu;
    m_bAskLimit = false;

    AddDU(new TRadDataUnit(_T("Objective"), IDC_OBJ0, *reinterpret_cast<int*>(&m_Select), -1, 0, objN));
    AddDU(new TIntDataUnit(strEmpty, IDC_LIM_SEC, m_Limit, 0, 0, 1, 9999));
}

BEGIN_MESSAGE_MAP(CDlgObj1, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgObj1 message handlers

BOOL CDlgObj1::OnInitDialog()
{
    super::OnInitDialog();
    FillObjectives();

    for (int i = 0; i < objN; ++i) EnableDlgItem(IDC_OBJ0 + i, (m_mask & (1 << i)) != 0);

    if (!m_bAskLimit)
    {
        UINT arrIDC[] = {IDC_LAB3, IDC_LAB4, IDC_LIM_SEC, 0};
        EnableDlgItems(arrIDC, false);
    }

    return true;
}
