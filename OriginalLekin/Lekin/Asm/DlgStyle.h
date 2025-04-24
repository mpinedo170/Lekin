#pragma once

#include "DlgA.h"
#include "ID.h"

class TSchedule;

/////////////////////////////////////////////////////////////////////////////
// CDlgStyle dialog

class CDlgStyle : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_STYLE
    };

    CEdit m_wndID;
    CSliderCtrl m_sldRGB[3];
    CEdit m_editRGB[3];

    CRect m_sampRect;
    bool m_bInUpdate;
    CString m_idOld;

public:
    static bool EditColor(TID* pStyle);
    static bool EditSch(TSchedule* pSch);

protected:
    explicit CDlgStyle(const TID& style, int time);
    void SetRGB(int i, int val);

    TID m_style;
    int m_time;

    CString& ID()
    {
        return m_style.m_id;
    }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    virtual void OnOK();
    afx_msg void OnSlide(UINT id, NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnEdit(UINT id);

    DECLARE_MESSAGE_MAP()
};
