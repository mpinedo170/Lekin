#pragma once

#include "CmbWkc.h"
#include "DlgA.h"

class TWorkcenter;
class TMachine;

/////////////////////////////////////////////////////////////////////////////
// CDlgAddMch dialog

class CDlgAddMch : public CDlgA
{
private:
    typedef CDlgA super;
    enum
    {
        IDD = IDD_ADD_MCH
    };

    CString m_idOld;
    CCmbWkc m_cmbWkc;
    TMode m_mode;

public:
    CDlgAddMch();

    void AddMch(TMachine& mch);
    void ReadMch(TMachine& mch);

    CString m_idWkc;
    CString m_idMch;
    CString m_mchName;
    int m_mchCount;
    int m_release;
    CString m_status;

    TWorkcenter* m_pWkc;
    bool m_bIDChange;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual bool IsDUNeeded(const TDataUnit* pDu);
    virtual bool IsDUMultiSel(const TDataUnit* pDu);

    DECLARE_MESSAGE_MAP()
};
