#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgParameter.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgParameter dialog

CDlgParameter::CDlgParameter() : super(CDlgParameter::IDD)
{
    m_section = secHeu;
    AddDU(new TDblDataUnit(_T("K1"), IDC_K1, m_k1, 1, 0, 1e-5, 1e5));
    AddDU(new TDblDataUnit(_T("K2"), IDC_K2, m_k2, 1, 0, 1e-6, 1e6));
}

BEGIN_MESSAGE_MAP(CDlgParameter, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgParameter message handlers
