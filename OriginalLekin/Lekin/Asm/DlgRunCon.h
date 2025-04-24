#pragma once

#include "DlgRunAlg.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunCon dialog

class CDlgRunCon : public CDlgRunAlg
{
private:
    typedef CDlgRunAlg super;
    enum
    {
        IDD = IDD_RUN_CON
    };

    CRichEditCtrl m_editStatus;
    CFont m_font;
    CPoint m_MinSize;
    CSize m_statD;
    int m_cancelY;
    bool m_bCR;
    int m_editStart;

    CHARFORMAT m_outFormat;
    CHARFORMAT m_inFormat;
    CHARFORMAT m_in2Format;

public:
    CDlgRunCon();

    void Add(LPCTSTR s);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnProtected(NMHDR* pNMHDR, LRESULT* pResult);

    virtual bool Initialize();
    virtual void OnRead();
    virtual void OnWrite();

    DECLARE_MESSAGE_MAP()
};
