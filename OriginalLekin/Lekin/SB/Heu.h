// Heu.h
//
//  1) objective functions
//  2) index functions
//  3) sub-problem heuristics
//  4) local optimization
#pragma once

#include "Parameter.h"

TMachine* FirstAvail(TShop* pShop, TMcAvail& MCAvail, TOperation* popSelect, TDGraph&);
TMachine* FirstFinish(TShop* pShop, TMcAvail& MCAvail, TOperation* popSelect, TDGraph& Graph);

TRuleReturn* IndexJobDueDate(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexOpDueDate(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexSPT(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexBetterSPT(TShop* pShop,
    TMcAvail& MCAvail,
    TJob& jbX,
    TDGraph& Graph,
    double fK1 = 0,
    double fK2 = 0);  // scan operations those have not arrived
TRuleReturn* IndexLPT(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexBetterLPT(TShop* pShop,
    TMcAvail& MCAvail,
    TJob& jbX,
    TDGraph& Graph,
    double fK1 = 0,
    double fK2 = 0);  // scan operations those have not arrived
TRuleReturn* IndexFCFS(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexJobReleaseTime(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexSchrage(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexBetterEDD(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexATC(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2 = 0);
TRuleReturn* IndexMATC(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2 = 0);
TRuleReturn* IndexTER(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2 = 0);
TRuleReturn* IndexTER2(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexTER3(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
TRuleReturn* IndexWTail(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1 = 0, double fK2 = 0);
// TRuleReturn *IndexWTail2(TShop *pShop,TMcAvail &MCAvail,TJob &jbX,TDGraph &Graph,double fK1=0,double
// fK2=0); TRuleReturn *IndexShopTER(TShop *pShop,TMcAvail &MCAvail,TJob &jbX,TDGraph &Graph,double fK1,double
// fK2=0.3);
TRuleReturn* IndexELOST(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2 = 0.3);

/* objective functions


*/
double WC(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
double Cmax(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
double WLmax(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
double WT(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
double WTE(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
double WTeE(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
double WU(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);

class TTool;

/*  Workcenter Sequecing

  SUB_WTE = single  : TER->CPI
            parallel: BeamSearch(3)
            batch   : BTER

  SUB_WTE_Setup = single  : TER->CPI->Local(70)
                  parallel: BeamSearch(3,70)
                  batch   : BTER

  SUB_WT  = single  : TER->CPI       ** Change objective to WT
            parallel: BeamSearch(3)
            batch   : BTER

  SUB_ATC  = ATC rule on single/parallel/batch workcenter

  SUB_TER  = Apply TER on single/parallel/batch workcenter

*/
double SUB_WTE(TTool& WkTool);
double SUB_WTE_Setup(TTool& WkTool);
double SUB_Cmax(TTool& WkTool);
double SUB_Cmax_Setup(TTool& WkTool);
double SUB_WC(TTool& WkTool);
double SUB_WC_Setup(TTool& WkTool);
double SUB_WLmax(TTool& WkTool);
double SUB_WLmax_Setup(TTool& WkTool);
double SUB_WT(TTool& WkTool);  // test if WT is similar to WTE or not
double SUB_ATC(TTool& WkTool);
double SUB_EDD_O(TTool& WkTool);
double SUB_TER(TTool& WkTool);
double SUB_TER_Setup(TTool& WkTool);

/* Local-Global reoptimization

*/
void BYPASS(
    TShop&, TStackP&, double (*OptSeq)(TTool&), double (*Obj)(int, double*, double*, double*, double*));
void LocalOpt_4Iter(TShop& Sh,
    TStackP& WkcSchedList,
    double (*OptSeq)(TTool& WkTool),
    double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight));
void LocalOpt(TShop& Sh,
    TStackP& WkcSchedList,
    double (*OptSeq)(TTool& WkTool),
    double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight));

#include "Tool.h"
