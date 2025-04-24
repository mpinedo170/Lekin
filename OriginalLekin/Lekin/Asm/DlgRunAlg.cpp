#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgRunAlg.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunAlg dialog

CDlgRunAlg::CDlgRunAlg(UINT nIDTemplate) : super(nIDTemplate)
{
    m_sec = 0;
    m_startTime = CTime::GetCurrentTime();
}

BEGIN_MESSAGE_MAP(CDlgRunAlg, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRunAlg Overridables

bool CDlgRunAlg::Initialize()
{
    m_wndTime.SubclassDlgItem(IDC_TIME, this);
    return super::Initialize();
}

void CDlgRunAlg::Update()
{
    CTime curTime = CTime::GetCurrentTime();
    int sec = int((curTime - m_startTime).GetTotalSeconds());
    if (sec == m_sec) return;

    CString s = IntToStr(sec);
    if (m_pPlug->m_bLimAsk)
    {
        s += _T(" / ");
        s += IntToStr(m_pPlug->m_limSec);
    }
    s += _T(" sec");

    if (sec >= m_pPlug->m_limSec) m_wndTime.EnableWindow(true);
    m_wndTime.SetWindowText(s);
    m_sec = sec;

    if (sec >= m_pPlug->m_limSec + 5 && m_pPlug->m_bLimEnforce)
    {
        m_result = Timeout;
        Terminate();
        Kill();
    }
}

void CDlgRunAlg::Start()
{
    m_pPlug->RunProcess(m_hProcess, m_hThread);
    if (!m_pPlug->m_bLimAsk) m_wndTime.EnableWindow(true);
}
