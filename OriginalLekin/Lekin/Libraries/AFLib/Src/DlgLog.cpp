#include "StdAfx.h"

#include "AFLibDlgLog.h"

#include "AFLibGlobal.h"
#include "AFLibStringTokenizer.h"
#include "Local.h"

using AFLibGui::CDlgLog;

CSize CDlgLog::m_szSaved(0, 0);

IMPLEMENT_DYNAMIC(CDlgLog, super)

CDlgLog::CDlgLog(UINT nIDTemplate) : super(nIDTemplate)
{
    m_log = NULL;
}

BEGIN_MESSAGE_MAP(CDlgLog, super)
END_MESSAGE_MAP()

BOOL CDlgLog::OnInitDialog()
{
    CWaitCursor wc;

    SetWindowText(AfxGetApp()->m_pszAppName);
    m_wndAlert.SubclassDlgItem(IDC_LOG_ALERT, this);
    super::OnInitDialog();

    m_wndAlert.SetWindowText(m_alert);
    m_editLog.AppendLog(m_log);
    m_editLog.SubclassDlgItem(IDC_LOG_LOG, this);

    StartAutoResize(&m_szSaved);
    return true;
}

void CDlgLog::OnBnClickedCopy()
{
    AFLib::WriteClipboardString(m_log, true);
}

CDlgLog::TResize CDlgLog::GetResizeMask(UINT idc)
{
    switch (idc)
    {
        case IDC_LOG_ALERT:
            return TResize(rzsPull, rzsNone);
        case IDC_LOG_LOG:
            return TResize(rzsPull, rzsPull);
        case IDOK:
            return TResize(0.5, 1, 0.5, 1);
    }
    return super::GetResizeMask(idc);
}
