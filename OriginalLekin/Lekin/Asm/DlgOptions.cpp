#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgOptions property page

CDlgOptions::CDlgOptions() : super(CDlgOptions::IDD)
{
    AddDU(new TSpnDataUnit(strEmpty, IDC_TYPESIZE, IDC_SIZE_SPIN, m_sizeFont, 0, 0, 3, 16));
    AddDU(new TCbsDataUnit(strEmpty, IDC_TYPEFACE, m_faceFont, strEmpty, strEmpty));
    AddDU(new TRadDataUnit(strEmpty, IDC_CONFIRM0, m_confirmLevel, 0, 0, 3));
}

BEGIN_MESSAGE_MAP(CDlgOptions, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOptions message handlers

BOOL CDlgOptions::OnInitDialog()
{
    m_cmbFaceFont.SubclassDlgItem(IDC_TYPEFACE, this);
    super::OnInitDialog();
    return true;
}
