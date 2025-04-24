#include "StdAfx.h"

#include "AFLibProFont.h"

#include "AFLibResource.h"

using AFLibGui::CProFont;

IMPLEMENT_DYNAMIC(CProFont, super)

CProFont::CProFont(UINT nIDTemplate) : super(nIDTemplate)
{
    m_sel = -1;
    m_minFontSize = 4;
    m_maxFontSize = 20;
    m_pSpinSize = new CSpinButtonCtrl;
}

CProFont::~CProFont()
{
    delete m_pSpinSize;
}

void CProFont::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FONT_TYPE, m_cmbFontType);
    DDX_Control(pDX, IDC_SPIN_FONT_SIZE, *m_pSpinSize);
}

bool CProFont::ReadFontData()
{
    TFontDescriptor* pFdc = m_arrFont[m_sel];
    try
    {
        pFdc->LoadDlg(this, m_minFontSize, m_maxFontSize);
    }
    catch (CException* pExc)
    {
        pExc->ReportError();
        pExc->Delete();
        return false;
    }
    return true;
}

BEGIN_MESSAGE_MAP(CProFont, super)
ON_CBN_SELCHANGE(IDC_FONT_TYPE, OnCbnSelChangeFontType)
END_MESSAGE_MAP()

BOOL CProFont::OnInitDialog()
{
    m_cmbFace.SubclassDlgItem(IDC_FONT_FACE, this);
    super::OnInitDialog();
    m_pSpinSize->SetRange(m_minFontSize, m_maxFontSize);

    for (int i = 0; i < m_arrFont.GetSize(); ++i) m_cmbFontType.AddString(m_arrFont[i]->GetTitle());

    m_cmbFontType.SetCurSel(0);
    OnCbnSelChangeFontType();
    return true;
}

void CProFont::OnCbnSelChangeFontType()
{
    int sel = m_cmbFontType.GetCurSel();
    if (sel == m_sel) return;

    if (m_sel != -1 && !ReadFontData())
    {
        m_cmbFontType.SetCurSel(m_sel);
        return;
    }

    m_sel = sel;
    TFontDescriptor* pFdc = m_arrFont[m_sel];
    pFdc->SaveDlg(this);
}

BOOL CProFont::OnKillActive()
{
    if (!ReadFontData()) return false;
    return super::OnKillActive();
}
