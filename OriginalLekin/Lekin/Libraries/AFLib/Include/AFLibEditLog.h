#pragma once

#include "AFLibDefine.h"
#include "AFLibStringBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// CEditLog edit box
// Parses and displays a log

namespace AFLibGui {
class AFLIB CEditLog : public CEdit
{
private:
    typedef CEdit super;
    DEFINE_COPY_AND_ASSIGN(CEditLog);

    int m_savedStartChar;  // saved selection for last SetSel: start character
    int m_savedEndChar;    // saved selection for last SetSel: end character
    bool m_bInSetFocus;    // inside OnSetFocus()

    // text of the log (before the window is displayed)
    AFLibIO::TStringBuffer m_logMsg;

    // clear the "saved selection" values
    void ClearSavedSel()
    {
        m_savedStartChar = m_savedEndChar = -1;
    }

    // append the log in the window
    void Flush(int startPos);

public:
    CEditLog();

    // clear the log
    void ClearLog();

    // append log with messages
    void AppendLog(LPCTSTR msg);

    // get the final log
    const AFLibIO::TStringBuffer& GetLog()
    {
        return m_logMsg;
    }

protected:
    // called in Subclass() and Create()
    virtual void OnInit();

    virtual void PreSubclassWindow();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnEmSetSel(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnEditTop();
    afx_msg void OnEditBottom();
    afx_msg void OnEditSelectAll();
    afx_msg void OnEditCopy();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CEditLog)
};
}  // namespace AFLibGui
