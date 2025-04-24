#pragma once

#include "DlgRunning.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunComp dialog

class CDlgRunComp : public CDlgRunning
{
private:
    typedef CDlgRunning super;
    enum
    {
        IDD = IDD_RUN_COMP
    };

    int m_current;
    CString m_key;

protected:
    CProgressCtrl m_barProgress;

public:
    CDlgRunComp();

    bool m_bCompress;
    CString m_dir;
    CString m_zip;
    int m_count;

protected:
    virtual bool Initialize();
    virtual void Start();
    virtual void Stop();
    virtual void OnRead();

    DECLARE_MESSAGE_MAP()
};
