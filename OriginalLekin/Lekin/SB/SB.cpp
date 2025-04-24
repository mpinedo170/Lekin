#include "StdAfx.h"

#include "SB.h"
#include "Shop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSBApp

BEGIN_MESSAGE_MAP(CSBApp, CWinApp)
//{{AFX_MSG_MAP(CSBApp)
//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSBApp construction

CSBApp::CSBApp()
{}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSBApp object

CSBApp theApp;

bool ConvertWkc(ostream& os);
void ConvertJob(ostream& os);
void ConvertSeq(istream& is, LPCSTR head);

/////////////////////////////////////////////////////////////////////////////
// CSBApp initialization

void GetMaxValues(int& MaxJob, int& MaxWkc, int& MaxMc);

BOOL CSBApp::InitInstance()
{
    int i1, i2, i3;
    GetMaxValues(i1, i2, i3);
    m_pMainWnd = &SBdlg;
    SBdlg.Create(CSBDlg::IDD);

    // SBdlg.ShowWindow(SW_HIDE);

    int param;
    if (_stscanf(m_lpCmdLine, "%d", &param) < 1) exit(1);
    CString label;

    BeginWaitCursor();
    switch (param)
    { /*
    case 23:
      // asking for parameter k
      dlg2.DoModal();
      // modified ATC
      Shop.SB(::WTE, SUB_TER, SUB_TER, BYPASS, BYPASS);
      break;
      */
        case 1:
            WinSB(::Cmax, SUB_EDD_O, SUB_Cmax, SUB_Cmax_Setup, LocalOpt_4Iter, LocalOpt, "Cmax");
            break;
        case 2:
            WinSB(::WLmax, SUB_EDD_O, SUB_WLmax, SUB_WLmax_Setup, LocalOpt_4Iter, LocalOpt, "Lmax");
            break;
        case 4:
            label = "sum(C)";
            goto WC;
        case 6:
            label = "sum(wC)";
        WC:
            WinSB(::WC, SUB_TER, SUB_WC, SUB_WC_Setup, LocalOpt_4Iter, LocalOpt, label);
            break;
        case 5:
            label = "sum(T)";
            goto WT;
        case 7:
            label = "sum(wT)";
        WT:
            WinSB(::WTE, SUB_TER, SUB_WTE, SUB_WTE_Setup, LocalOpt_4Iter, LocalOpt, label);
            break;
        default:
            exit(1);
    }
    EndWaitCursor();

    return false;
}

void CSBApp::WinSB(TObjFunc Obj,
    TSeqFunc BNSeq,
    TSeqFunc OptSeq,
    TSeqFunc OptSeqSetup,
    TOptFunc LocalOpt,
    TOptFunc FinalOpt,
    LPCSTR head)
{
    // SB(bottleneckdetermination, bottlenecksequencing, localreoptimization,
    // localreoptimization_if_setup, finalreoptimization)
    // SB(Obj(,,,),BNSeq(WKC),OptSeq(WKC),LocalOpt,FinalOpt)
    // ex
    // SB(WTE,TER,SC,LocalSearch,LocalSearch)

    //***** Initialize
    TStackP WkUnschedList, WkcSchedList, WkListTemp;
    TWorkcenter* pWk;
    TSequenceList sqlWorst;
    TTool WkTool, WkToolBN;
    double fWorst = -fBigM, fTemp, fPrevObj = fBigM;
    int i, iN;
    CString sTemp;

    TShop Shop;
#ifdef _DEBUG
    ofstream os1("u.wkt");
    ofstream os2("u.job");
#else
    ostrstream os1, os2;
#endif
    bool bSetups = ConvertWkc(os1);
    ConvertJob(os2);
    if (bSetups) OptSeq = OptSeqSetup;

#ifdef _DEBUG
    os1.close();
    os2.close();
    ifstream is1("u.wkt");
    ifstream is2("u.job");
#else
    istrstream is1(const_cast<const char*>(os1.str()), os1.pcount());
    istrstream is2(const_cast<const char*>(os2.str()), os2.pcount());
#endif

    Shop.LoadWk(is1);
    Shop.LoadJob(is2);

    Shop.ClearSequence();
    WkTool.SetObj(Obj);
    iN = Shop.WkCenter.Num();
    Shop.WkCenter.Head();
    for (i = 1; i <= iN; i++) WkUnschedList.Push((TWorkcenter*)Shop.WkCenter.Next());

    TDGraph Grh;

    while (WkUnschedList.Num())
    {
        //***** Find bottleneck workcenter
        SBdlg.CheckRadioButton(101, 105, IDC_FIND);

        fWorst = -fBigM;
        for (i = 1; i <= WkUnschedList.Num(); i++)
        {
            pWk = (TWorkcenter*)WkUnschedList[i];
            Shop.GraphOnWkc(pWk, Grh);
            WkTool.SetData(&Shop, pWk, &Grh);
            WkTool.SetObj(Obj);
            fTemp = BNSeq(WkTool);  // rough sequencing to find a bottleneck wkc
            if (fTemp > fWorst)
            {
                fWorst = fTemp;
                WkToolBN = WkTool;
            }
        }

        //***** Sequence the bottleneck
        SBdlg.CheckRadioButton(101, 105, IDC_SEQ);
        // itoa(WkToolBN.pWkCenter->wID,szTemp,10);
        // sTemp = "Bottleneck Workcenter = ";
        // sTemp += szTemp;
        // AfxMessageBox(sTemp);

        if (fabs(fPrevObj - fWorst) > fSmall) OptSeq(WkToolBN);
        Shop.Append(WkToolBN.Seq);

        WkUnschedList.Delete(WkUnschedList.Locate(WkToolBN.pWkCenter));
        WkcSchedList.Push(WkToolBN.pWkCenter);
        fPrevObj = Shop.Objective(Obj);

        //***** Local re-optimization
        SBdlg.CheckRadioButton(101, 105, IDC_LOCAL);
        // sTemp = "Bottleneck Workcenter = ";
        // itoa(SBdlg.m_WkID,szTemp,10);
        // sTemp += szTemp;
        // AfxMessageBox(sTemp);

        LocalOpt(Shop, WkcSchedList, OptSeq, Obj);
    }

    //***** Final re-optimization
    SBdlg.CheckRadioButton(101, 105, IDC_GLOBAL);
    FinalOpt(Shop, WkcSchedList, OptSeq, Obj);

    ostrstream os3;
    os3 << Shop.Seq;

    istrstream is3(const_cast<const char*>(os3.str()), os3.pcount());
    ConvertSeq(is3, head);
}
