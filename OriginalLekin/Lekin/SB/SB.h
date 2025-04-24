// SB.h : main header file for the SB application
//

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "Heu.h"
#include "Resource.h"  // main symbols
#include "SBDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSBApp:
// See SB.cpp for the implementation of this class
//

class CSBApp : public CWinApp
{
    CSBDlg SBdlg;

public:
    CSBApp();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSBApp)
public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

    // Implementation

    //{{AFX_MSG(CSBApp)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void WinSB(TObjFunc Obj,
        TSeqFunc BNSeq,
        TSeqFunc OptSeq,
        TSeqFunc OptSeqSetup,
        TOptFunc LocalOpt,
        TOptFunc FinalOpt,
        LPCTSTR head);
};

/////////////////////////////////////////////////////////////////////////////
