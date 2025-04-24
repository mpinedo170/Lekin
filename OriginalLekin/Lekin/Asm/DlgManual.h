#pragma once

#include "CmbMch.h"
#include "DlgA.h"

class TMachine;
class TSchedule;

/////////////////////////////////////////////////////////////////////////////
// CDlgManual dialog

class CDlgManual : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_MANUAL
    };

    CEdit m_wndSeq;
    CCmbMch m_cmbMch;
    TSchedule* m_pSch;
    TMachine* m_pMch;

    bool ReadSequence();

public:
    CDlgManual(TSchedule* pSch);

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeMchId();
    virtual void OnOK();
    afx_msg void OnNext(UINT idc);

    DECLARE_MESSAGE_MAP()
};
