#pragma once

#include "DlgRunning.h"
#include "Plugin.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgRunAlg dialog

class CDlgRunAlg : public CDlgRunning
{
private:
    typedef CDlgRunning super;

protected:
    CStatic m_wndTime;

public:
    CDlgRunAlg(UINT nIDTemplate);

    TPlugin* m_pPlug;
    CTime m_startTime;
    int m_sec;

protected:
    virtual bool Initialize();
    virtual void Start();
    virtual void Update();

    DECLARE_MESSAGE_MAP()
};
