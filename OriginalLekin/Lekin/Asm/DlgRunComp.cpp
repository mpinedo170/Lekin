#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgRunComp.h"

#include "Misc.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunComp dialog

CDlgRunComp::CDlgRunComp() : super(CDlgRunComp::IDD)
{
    m_bCompress = true;
    m_count = 0;
    m_current = -1;
}

BEGIN_MESSAGE_MAP(CDlgRunComp, super)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRunComp overridden

bool CDlgRunComp::Initialize()
{
    m_barProgress.SubclassDlgItem(IDC_PROGRESS, this);

    if (m_count == 0)
    {
        AfxMB(IDP_NO_EXPORT);
        return true;
    }

    m_key = "Compressing :  ";
    m_barProgress.SetStep(1);
    m_barProgress.SetRange(0, m_count);
    m_barProgress.SetPos(0);
    return super::Initialize();
}

void CDlgRunComp::Start()
{
    CString command = strDQuote + ExePath + _T("IComp.exe") + strDQuote;
    CString line;
    AfxFormatString2(line, m_bCompress ? IDS_COMPRESS : IDS_DECOMPRESS, m_dir, m_zip);
    command += line;
    MakeProcess(command, GetTmpPath(), mpHide, m_hProcess, m_hThread);
}

void CDlgRunComp::Stop()
{
    m_barProgress.SetPos(m_count);
    super::Stop();
    EndDialog(m_code & 0xFFFFFF00 ? IDCANCEL : IDOK);
}

void CDlgRunComp::OnRead()
{
    while (true)
    {
        int index = m_sRead.Find(m_key);
        if (index == -1) break;

        ++m_current;
        index += _tcslen(m_key);
        m_barProgress.SetPos(m_current);
        m_sRead = m_sRead.Right(m_sRead.GetLength() - index);
    }
    Continue();
}
