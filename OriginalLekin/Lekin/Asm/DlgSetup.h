#pragma once

#include "DlgA.h"

class TSetup;

/////////////////////////////////////////////////////////////////////////////
// CDlgSetup dialog

class CDlgSetup : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_SETUP
    };

    CSpinButtonCtrl m_spinDim;
    TSetup& m_setup;

    CStatic m_xAxis[26];
    CStatic m_yAxis[26];
    CEdit m_Edit[26][26];
    CFont m_font;

    int m_width, m_height;
    CSize m_butSize;

    void PutButton(UINT id, int place);
    void Resize();
    void DataToDlg(TSetup& setup);
    void DlgToData(TSetup& setup);
    int m_dim;

public:
    CDlgSetup(TSetup& setup, int dim);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeDim();
    virtual void OnOK();
    afx_msg void OnLoad();
    afx_msg void OnKillFocus(UINT idc);

    DECLARE_MESSAGE_MAP()
};
