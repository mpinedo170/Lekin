#pragma once

#include "CmbWkc.h"
#include "DlgA.h"

class TJob;
class TOperation;

/////////////////////////////////////////////////////////////////////////////
// CDlgAddOper dialog

class CDlgAddOper : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_ADD_OPER
    };

    CCmbWkc m_cmbWkc;
    CSpinButtonCtrl m_spinNumber;
    int m_maxNumber;

public:
    CDlgAddOper();

    CString m_idJob;
    int m_proc;
    CString m_status;
    CString m_idWkc;
    int m_number;

    TOperation* m_pOp;
    TJob* m_pJob;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()
};
