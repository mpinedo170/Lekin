#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadRS dialog

#include "DlgA.h"

class CDlgLoadRS : public CDlgA
{
private:
    typedef CDlgA super;

public:
    CDlgLoadRS(UINT nIDTemplate);

protected:
    bool m_bNoDup;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnNoDup();

    virtual void FillListBox() = 0;
    virtual void SaveSelection() = 0;
    virtual void LoadSelection() = 0;

    DECLARE_MESSAGE_MAP()
};
