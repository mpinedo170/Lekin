#pragma once

#include "AFLibDlgTT.h"
#include "AFLibEditLog.h"
#include "AFLibResource.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgLog dialog
// Shows error log

namespace AFLibGui {
class AFLIB CDlgLog : public CDlgTT
{
private:
    typedef CDlgTT super;
    DEFINE_COPY_AND_ASSIGN(CDlgLog);

protected:
    enum
    {
        IDD = IDD_LOG
    };

    static CSize m_szSaved;  // the last size of the dialog box

    // controls
    CStatic m_wndAlert;
    CEditLog m_editLog;

public:
    explicit CDlgLog(UINT nIDTemplate = IDD);

    CString m_alert;  // alert shown above the log
    LPCTSTR m_log;    // the log itself

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCopy();
    virtual TResize GetResizeMask(UINT idc);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CDlgLog)
};
}  // namespace AFLibGui
