#include "StdAfx.h"

#include "Shop.h"
#include "Tool.h"

TDGraph grhDGraph1_buf;
TTool tlDGraph1_buf;
TJobList jlDGraph1_buf;

TMachine* FirstAvail(TShop*, TMcAvail& MCAvail, TOperation* popSelect, TDGraph&)
{
    //*** select the first available machine
    if (MCAvail.Num() == 1) return (MCAvail.GetMC(1));
    TMachine *pmcSelect, *pmcTemp;
    int iM = MCAvail.Num();
    double fBest = fBigM;
    for (int j = 1; j <= iM; j++)
    {
        pmcTemp = popSelect->GetMachine(j);
        if (MCAvail.IsMember(pmcTemp->mID))
            if (MCAvail.Get(pmcTemp->mID) < fBest)
            {
                fBest = MCAvail.Get(pmcTemp->mID);
                pmcSelect = pmcTemp;
            }
    }
    return pmcSelect;
}

TMachine* FirstFinish(TShop* pShop, TMcAvail& MCAvail, TOperation* popSelect, TDGraph& Graph)
{
    if (MCAvail.Num() == 1) return (MCAvail.GetMC(1));
    TMachine *pmcSelect, *pmcTemp;
    TStatus sFrom, sTo;
    TNode* pndTemp = Graph.GetID(popSelect->oID);
    int iM = popSelect->stkMachine.Num(), iTemp;
    double fBest = fBigM, fTime;
    for (int j = 1; j <= iM; j++)
    {
        pmcTemp = popSelect->GetMachine(j);
        if (MCAvail.IsMember(pmcTemp->mID))
        {
            iTemp = MCAvail.LastOp(pmcTemp->mID);
            // iTemp   = pShop->Seq.GetLast(pmcTemp->mID);
            if (iTemp)
                sFrom = (pShop->Job.Geto_oID(iTemp))->sMC_Status;
            else
                sFrom = pmcTemp->sInit;
            sTo = popSelect->sMC_Status;
            fTime = pmcTemp->suSetup.Get(sFrom, sTo);                        // setup
            fTime = max(MCAvail.Get(pmcTemp->mID) + fTime, pndTemp->EST());  // release
            fTime += (double)(popSelect->fProcessTime / pmcTemp->fSpeed);

            if (fTime < fBest)
            {
                fBest = fTime;
                pmcSelect = pmcTemp;
            }
        }
    }
    return pmcSelect;
}

//************************ indexing functions ************************//
/*
   Indexing functions are used in PriorityRule(**) function. A new
   priority rule can be developed by creating the indexing function that
   suits your task. When PriorityRule is called, it will sort the
   active operations (operations that do not have precedent constraints)
   using the indexing function specified when calling PriorityRule.
   The operation with minimum index will be selected as the next
   operation to sequence. The procedure will be repeated until all the
   operations are sequenced.

   The active operations are passed to indexing function as a TJob object.
   There are 2 special variables used to carry spacial data as follows.

     TOperation::fStartTime contains Earliest Starting Time of the operation.
     TOperation::fDueDate  contains the Latest Stating Time + Length of the arc.

   The 2nd variable is used in Schrage's algo.
*/

TRuleReturn* IndexJobDueDate(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TJob* pJob;
    TOperation *popSelect, *popTemp;
    double fBest = fBigM, fTemp;
    int i, iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        pJob = popTemp->pJob;
        fTemp = pJob->fDueDate;
        if (fTemp < fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    return pRtn;
}

TRuleReturn* IndexOpDueDate(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popSelect, *popTemp;
    // TNode      *pNode;
    double fBest = fBigM, fTemp;
    int i, iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        fTemp = popTemp->fDueDate;
        if (fTemp < fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    pRtn->iDataReCalc = 1;
    return pRtn;
}

TRuleReturn* IndexSPT(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popSelect, *popTemp;
    double fBest = fBigM, fTemp;
    int i, iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        fTemp = popTemp->fProcessTime;
        if (fTemp < fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    return pRtn;
}

TRuleReturn* IndexLPT(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation* popSelect;
    TOperation* popTemp;
    double fBest = -fBigM, fTemp;
    int i, iN = jbX.Num();

    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        fTemp = popTemp->fProcessTime;
        if (fTemp > fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    return pRtn;
}

TRuleReturn* IndexFCFS(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popSelect, *popTemp;
    double fBest = fBigM, fTemp;
    int i, iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        TNode* pNode = Graph.GetID(popTemp->oID);
        fTemp = pNode->EST();
        if (fTemp < fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    return pRtn;
}

TRuleReturn* IndexJobReleaseTime(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popSelect, *popTemp;
    double fBest = fBigM, fTemp;
    // TJob *pJob;
    int i, iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        fTemp = popTemp->pJob->fReleaseTime;
        if (fTemp < fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    return pRtn;
}

TRuleReturn* IndexBetterSPT(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TMachine* pmcTemp;
    int i, j, iN;

    TArrayF fMinRelease;
    TArrayP popFirst, pmcOperation;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fWorstIndex = -fBigM,
                    fMinStart = fBigM, fBestIndex;
    TStatus sS1, sS2;
    TNode* pndTemp;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM, fFirstComplete, fMinCompleteForOp, fComplete;
    TMachine* pmcFirstComplete;
    TOperation* popFirstComplete;
    TArrayF aIndex;

    // pRtn->iDataReCalc = 1;

    //* this part will remove the operation that has not arrived
    fFirstComplete =
        fBigM;  // the completion time of first op, op that release after this time should not be considered
    iN = jbX.Num();

    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        fMinCompleteForOp = fBigM;

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
            fLength = fActProc;
            popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
            if (fAvgSetup)
            {
                if (popLastInSeq)
                    sS1 = popLastInSeq->sMC_Status;
                else
                    sS1 = pmcTemp->sInit;
                sS2 = popTemp->sMC_Status;
                fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
            }
            else
                fActSetup = 0;
            fComplete = MCAvail.Get(pmcTemp->mID) + fActSetup;
            if (popTemp2->fReleaseTime > fComplete) fComplete = popTemp2->fReleaseTime;
            fComplete += fActProc;
            if (fComplete < fMinCompleteForOp)
            {
                pmcOperation.Put(popTemp->oID, pmcTemp);
                fMinCompleteForOp = fComplete;

                if (fComplete < fFirstComplete)
                {
                    fFirstComplete = fComplete;
                    popFirstComplete = popTemp2;
                    pmcFirstComplete = pmcTemp;
                }
            }
        }
        // calc index put in aIndex[oID]
        if (popTemp2->fReleaseTime > fFirstComplete)
            aIndex[popTemp2->oID] = fBigM;
        else
        {  //////////////// calc index for popTemp2 //////////////
            // aIndex[popTemp2->oID] // pmcOperation[popTemp2->oID] //
            aIndex[popTemp2->oID] = popTemp->fProcessTime;
        }  //////////////////////end calc index //////////////////
    }
    // find best index that its release time<=fFirstComplete
    pRtn->op = popFirstComplete;
    pRtn->mc = pmcFirstComplete;
    fBestIndex = fBigM;
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        if ((aIndex[popTemp2->oID] < fBestIndex) && (popTemp2->fReleaseTime <= fFirstComplete))
        {
            pRtn->op = popTemp2;
            pRtn->mc = (TMachine*)pmcOperation.Get(popTemp2->oID);
            fBestIndex = aIndex[popTemp2->oID];
        }
    }
    return pRtn;
}

TRuleReturn* IndexBetterLPT(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TMachine* pmcTemp;
    int i, j, iN;

    TArrayF fMinRelease;
    TArrayP popFirst, pmcOperation;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fWorstIndex = -fBigM,
                    fMinStart = fBigM, fBestIndex;
    TStatus sS1, sS2;
    TNode* pndTemp;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM, fFirstComplete, fMinCompleteForOp, fComplete;
    TMachine* pmcFirstComplete;
    TOperation* popFirstComplete;
    TArrayF aIndex;

    // pRtn->iDataReCalc = 1;

    //* this part will remove the operation that has not arrived
    fFirstComplete =
        fBigM;  // the completion time of first op, op that release after this time should not be considered
    iN = jbX.Num();

    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        fMinCompleteForOp = fBigM;

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
            fLength = fActProc;
            popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
            if (fAvgSetup)
            {
                if (popLastInSeq)
                    sS1 = popLastInSeq->sMC_Status;
                else
                    sS1 = pmcTemp->sInit;
                sS2 = popTemp->sMC_Status;
                fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
            }
            else
                fActSetup = 0;
            fComplete = MCAvail.Get(pmcTemp->mID) + fActSetup;
            if (popTemp2->fReleaseTime > fComplete) fComplete = popTemp2->fReleaseTime;
            fComplete += fActProc;
            if (fComplete < fMinCompleteForOp)
            {
                pmcOperation.Put(popTemp->oID, pmcTemp);
                fMinCompleteForOp = fComplete;

                if (fComplete < fFirstComplete)
                {
                    fFirstComplete = fComplete;
                    popFirstComplete = popTemp2;
                    pmcFirstComplete = pmcTemp;
                }
            }
        }
        // calc index put in aIndex[oID]
        if (popTemp2->fReleaseTime > fFirstComplete)
            aIndex[popTemp2->oID] = fBigM;
        else
        {  //////////////// calc index for popTemp2 //////////////
            // aIndex[popTemp2->oID] // pmcOperation[popTemp2->oID] //
            aIndex[popTemp2->oID] = popTemp->fProcessTime;
        }  //////////////////////end calc index //////////////////
    }
    // find best index that its release time<=fFirstComplete
    pRtn->op = popFirstComplete;
    pRtn->mc = pmcFirstComplete;
    fBestIndex = -fBigM;
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        if ((aIndex[popTemp2->oID] > fBestIndex) && (popTemp2->fReleaseTime <= fFirstComplete))
        {
            pRtn->op = popTemp2;
            pRtn->mc = (TMachine*)pmcOperation.Get(popTemp2->oID);
            fBestIndex = aIndex[popTemp2->oID];
        }
    }
    return pRtn;
}

/*
TRuleReturn *IndexTER(TShop *pShop,TMcAvail &MCAvail,TJob &jbX,TDGraph &Graph,double fK1,double fK2)
{// suitable for workcenter scheduling
  TRuleReturn *pRtn = new TRuleReturn;
  TOperation  *popTemp,*popTemp2,
              *popLastInSeq;
  TJob     *pJob;
  TMachine *pmcSelect,
           *pmcTemp;
  int i,j,k,iN,iNumSink,
      iFlag=0;
  TArrayF fComplete,fNComplete,fDue,fWeight,fEWeight,fLTime;
  TArrayL SinkID;
  double fLength,
         fAvgSetup = MCAvail.fData2,
         fActProc=0,fActSetup=0,
         fTimeTemp,
         fIndex,
         fWorstIndex=-fBigM,
         fMin,fMin_Length,fMin_Setup,
         fMinStart=fBigM,
         fTemp,
         fL,
         fMinDue;
  TStatus sS1,sS2;
  TNode  *pSinkNode,
         *pndTemp,
         *pndBefore,
         *pndFrom,
         *pndTo,
         *pndNew;
  TWorkcenter *pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
  TStackP stkPostN;
  TArc   *pArc;

  double fMinMCRelease=fBigM,
         fMinRelease=fBigM;

  pRtn->iDataReCalc = 1;

//* this part will remove the operation that has not arrived
  for(i=1;i<=MCAvail.Num();i++)
  {
    if(MCAvail.fTime[i]<fMinMCRelease)
    {
      fMinMCRelease = MCAvail.fTime[i];
      pRtn->mc=MCAvail.GetMC(i);
    }
  }

  iN = jbX.Num();
  for(i=1;i<=iN;i++)
  {
    popTemp = jbX.Get(i);
    if(popTemp->fReleaseTime>fMinMCRelease)
    {
      if(popTemp->fReleaseTime<fMinRelease)
      {
        if(fMinRelease<fBigM)
          iFlag = 1;
        fMinRelease = popTemp->fReleaseTime;
        pRtn->op=(Graph.GetID(popTemp->oID))->op;
      }
      if(popTemp->fReleaseTime>fMinMCRelease+pRtn->op->fProcessTime/pRtn->mc->fSpeed)
      {
        jbX.Delete(i);
        i--;
        iN--;
      }
    }
  }
  if(iFlag)
    for(i=1;i<=iN;i++)
    {
      popTemp = jbX.Get(i);
      if(popTemp->fReleaseTime>fMinMCRelease+pRtn->op->fProcessTime/pRtn->mc->fSpeed)
      {
        jbX.Delete(i);
        i--;
        iN--;
      }
    }
  if(jbX.Num()==0)
    return pRtn;


 if(MCAvail.fData1==0) // need to determine the fAvgSetup (fAvgProcess == 0)
   fAvgSetup = pShop->AvgSetup();

 iN = Graph.NumSink();
 for(i=iN;i>=1;i--)
  {
    pSinkNode   = (TNode*)Graph.GetSink(i);
    pJob        = (TJob*)pShop->GetJob(i);
    fComplete[32766+pSinkNode->nID] = pSinkNode->EST();
    //fNComplete[i-1]= pSinkNode->EST();
    fLTime[32766+pSinkNode->nID]    = 0;
    fDue[32766+pSinkNode->nID]      = pJob->fDueDate;
    fWeight[32766+pSinkNode->nID]   = pJob->fWeight;
    fEWeight[32766+pSinkNode->nID]  = pJob->fEWeight;
    SinkID[32766+pSinkNode->nID]    = pSinkNode->nID;
  }

 jbX.Head();
 for(k=1;k<=jbX.Num();k++)
  {
    popTemp2 = (TOperation*)jbX.Next();
    pndTemp = Graph.GetID(popTemp2->oID);
    popTemp = pndTemp->op;
    // select machine
    fMin = fBigM;
    for(j=1;j<=popTemp->stkMachine.Num();j++)
    {
      // select machine with min complete time
      // if they available before operation -> select the fastest machine
      pmcTemp  = popTemp->GetMachine(j);
      if(MCAvail.IsMember(pmcTemp->mID))
      {
        fActProc = popTemp->fProcessTime/pmcTemp->fSpeed;
        fLength  = fActProc;
        popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
        if(fAvgSetup)
        {
          // find the index for the operation on the selected mc
          // Link previous node (popLastInSeq) with the focus operation
          if(popLastInSeq)
            sS1 = popLastInSeq->sMC_Status;
          else
            sS1 = pmcTemp->sInit;
          sS2 = popTemp->sMC_Status;
          fActSetup = pmcTemp->suSetup.Get(sS1,sS2);
        }

        fTimeTemp = max(MCAvail.Get(pmcTemp->mID)+fActSetup,popTemp2->fReleaseTime);
        if(fTimeTemp+fLength<fMin)
        {
          fMin=fTimeTemp+fLength;
          pmcSelect    = pmcTemp;
          fMin_Length  = fLength;
          fMin_Setup   = fActSetup;
          //fMin_popLast = popLastInSeq;
          if(popLastInSeq)
            pndBefore    = Graph.GetID(popLastInSeq->oID);
          else
            pndBefore    = Graph.GetID(0);
        }
      }
    }

    // find the index for the operation on the selected mc
    // Link previous node (popLastInSeq) with the focus operation

    stkPostN.Push(pndTemp,0);
    for(i=0;i<iN;i++)
      fLTime[i]=0;
    while(stkPostN.Num()>0)
    {
      fL       = stkPostN.Index(1);
      pndFrom  = (TNode*)stkPostN.Pop(1);
      for(i=1;i<=pndFrom->stkPostArc.Num();i++)
      {
        pArc    = pndFrom->GetPostArcPtr(i);
        pndTo   = pArc->pndTo;
        if(pndTo->IsSink())
        {
          fLTime[32766+pndTo->nID] = max(fLTime[32766+pndTo->nID],fL+pArc->fWeight);
        } else
        {
          stkPostN.Push(pndTo,fL+pArc->fWeight);
        }
      }
    }
  //  iNumSink=0;
    for(i=0;i<iN;i++)
    {
      //fNComplete[i]   = fComplete[i];
      fNComplete[i]   = -fBigM;
      if(fLTime[i]>0)
      {
        fTemp        = fMin;
        fTemp       += fLTime[i]-popTemp->fProcessTime;
        //fNComplete[i] = max(fTemp,fComplete[i]);
        fNComplete[i] = fTemp;
      }
    }

    // IndexTER()
//    fIndex = exp(WTeE(iN,fDue,fNComplete,fWeight,fEWeight)/fK1)/fMin_Length;
    fIndex = WTeE(iN,fDue,fNComplete,fWeight,fEWeight);

    // if there is setup

    if(fAvgSetup>0)
      fIndex *= exp(-fMin_Setup/(fK2*fAvgSetup));

    if(fIndex>fWorstIndex-fSmall)
    {
      if(equal(fIndex,fWorstIndex))
      {
        if(popTemp2->fDueDate<fMinDue)
        {
          fWorstIndex = fIndex;
          pRtn->op    = popTemp;
          pRtn->mc    = pmcSelect;
          fMinDue     = popTemp2->fDueDate;
        }
      } else
      {
        fWorstIndex = fIndex;
        pRtn->op    = popTemp;
        pRtn->mc    = pmcSelect;
        fMinDue     = popTemp2->fDueDate;
      }
    }
  }
  return pRtn;
}
*/

TRuleReturn* IndexSchrage(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double, double)
{
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popSelect, *popTemp;
    TMachine* pmcTemp;
    double fBest = fBigM, fTemp, fMinReleaseAll = fBigM;
    TArrayF fMinRelease;
    TArrayP popFirst;

    //* this part will remove the operation that has not arrived
    int i, j, iN = jbX.Num(), iFlag = 0;
    for (i = MCAvail.Num(); i >= 1; i--) fMinRelease[MCAvail.GetMC(i)->mID] = fBigM;
    for (i = 1; i <= iN; i++)
    {
        i = max(i, 1);
        popTemp = jbX.Get(i);

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fTemp = MCAvail.Get(pmcTemp->mID);
                if (popTemp->fReleaseTime > fTemp)
                {
                    if (popTemp->fReleaseTime < fMinRelease[pmcTemp->mID])
                    {
                        if (fMinRelease[pmcTemp->mID] < fBigM) iFlag = 1;
                        fMinRelease[pmcTemp->mID] = popTemp->fReleaseTime;
                        popFirst.Put(pmcTemp->mID, popTemp);
                        if (fMinReleaseAll > popTemp->fReleaseTime)
                        {
                            pRtn->op = (Graph.GetID(popTemp->oID))->op;
                            pRtn->mc = pmcTemp;
                        }
                    }
                    if (popTemp->fReleaseTime > fMinRelease[pmcTemp->mID])
                    {
                        jbX.Delete(i);
                        i--;
                        if (i < 1) i = 1;
                        iN--;
                    }
                }
            }
        }
    }
    if (iFlag == 1)
    {
        iN = jbX.Num();
        for (i = 1; i <= iN; i++)
        {
            popTemp = jbX.Get(i);
            for (j = 1; j <= popTemp->stkMachine.Num(); j++)
            {
                pmcTemp = popTemp->GetMachine(j);
                if ((MCAvail.IsMember(pmcTemp->mID)) && (popFirst[pmcTemp->mID]))
                {
                    if (popTemp->fReleaseTime > fMinRelease[pmcTemp->mID])
                    {
                        jbX.Delete(i);
                        i--;
                        if (i < 1) i = 1;
                        iN--;
                    }
                }
            }
        }
    }
    if (jbX.Num() == 0) return pRtn;

    iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        fTemp = popTemp->fDueDate;
        if (fTemp < fBest)
        {
            fBest = fTemp;
            popSelect = popTemp;
        }
    }
    pRtn->op = popSelect;
    pRtn->mc = FirstFinish(pShop, MCAvail, popSelect, Graph);
    pRtn->iDataReCalc = 1;
    return pRtn;
}

TRuleReturn* IndexELOST(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // suitable for workcenter scheduling
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TMachine *pmcTemp, *pmcSelect;
    int j, k;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fTimeTemp, fIndex,
                    fWorstIndex = fBigM, fMin, fMin_Length, fMin_Setup;
    TStatus sS1, sS2;
    TNode* pndTemp;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);

    pRtn->iDataReCalc = 1;

    if (MCAvail.fData1 == 0)  // need to determine the fAvgSetup (fAvgProcess == 0)
        fAvgSetup = pShop->AvgSetup();

    jbX.Head();
    for (k = 1; k <= jbX.Num(); k++)
    {
        popTemp2 = (TOperation*)jbX.Next();
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;
        // select machine
        fMin = fBigM;
        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            // select machine with min complete time
            // if they available before operation -> select the fastest machine
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
                fLength = fActProc;
                popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
                if (fAvgSetup)
                {
                    // find the index for the operation on the selected mc
                    // Link previous node (popLastInSeq) with the focus operation
                    if (popLastInSeq)
                        sS1 = popLastInSeq->sMC_Status;
                    else
                        sS1 = pmcTemp->sInit;
                    sS2 = popTemp->sMC_Status;
                    fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
                }

                fTimeTemp = max(MCAvail.Get(pmcTemp->mID) + fActSetup, popTemp2->fReleaseTime);
                if (fTimeTemp + fLength < fMin)
                {
                    fMin = fTimeTemp + fLength;
                    pmcSelect = pmcTemp;
                    fMin_Length = fLength;
                    fMin_Setup = fActSetup;
                }
            }
        }
        fIndex = popTemp2->fDueDate - (popTemp->fProcessTime / pmcSelect->fSpeed + fMin_Setup);
        // fIndex = popTemp2->fDueDate;
        if (fIndex < fWorstIndex)
        {
            fWorstIndex = fIndex;
            pRtn->op = popTemp;
            pRtn->mc = pmcSelect;
        }
    }
    return pRtn;
}

TRuleReturn* IndexBetterEDD(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TMachine* pmcTemp;
    int i, j, iN;

    TArrayF fMinRelease;
    TArrayP popFirst, pmcOperation;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fWorstIndex = -fBigM,
                    fMinStart = fBigM, fBestIndex;
    TStatus sS1, sS2;
    TNode* pndTemp;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM, fFirstComplete, fMinCompleteForOp, fComplete,
           fMin_Setup;
    TMachine* pmcFirstComplete;
    TOperation* popFirstComplete;
    TArrayF aIndex;

    //  pRtn->iDataReCalc = 1;

    //* this part will remove the operation that has not arrived
    fFirstComplete =
        fBigM;  // the completion time of first op, op that release after this time should not be considered
    iN = jbX.Num();

    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        fMinCompleteForOp = fBigM;

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
            fLength = fActProc;
            popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
            if (fAvgSetup)
            {
                if (popLastInSeq)
                    sS1 = popLastInSeq->sMC_Status;
                else
                    sS1 = pmcTemp->sInit;
                sS2 = popTemp->sMC_Status;
                fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
            }
            else
                fActSetup = 0;
            fComplete = MCAvail.Get(pmcTemp->mID) + fActSetup;
            if (popTemp2->fReleaseTime > fComplete) fComplete = popTemp2->fReleaseTime;
            fComplete += fActProc;
            if (fComplete < fMinCompleteForOp)
            {
                pmcOperation.Put(popTemp->oID, pmcTemp);
                fMin_Setup = fActSetup;
                fMinCompleteForOp = fComplete;

                if (fComplete < fFirstComplete)
                {
                    fFirstComplete = fComplete;
                    popFirstComplete = popTemp2;
                    pmcFirstComplete = pmcTemp;
                }
            }
        }
        // calc index put in aIndex[oID]
        if (popTemp2->fReleaseTime > fFirstComplete)
            aIndex[popTemp2->oID] = fBigM;
        else
        {  //////////////// calc index for popTemp2 //////////////
            // aIndex[popTemp2->oID] // pmcOperation[popTemp2->oID] //
            aIndex[popTemp2->oID] = popTemp2->fDueDate;
        }  //////////////////////end calc index //////////////////
    }
    // find best index that its release time<=fFirstComplete
    pRtn->op = popFirstComplete;
    pRtn->mc = pmcFirstComplete;
    fBestIndex = fBigM;
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        if ((aIndex[popTemp2->oID] < fBestIndex) && (popTemp2->fReleaseTime <= fFirstComplete))
        {
            pRtn->op = popTemp2;
            pRtn->mc = (TMachine*)pmcOperation.Get(popTemp2->oID);
            fBestIndex = aIndex[popTemp2->oID];
        }
    }
    return pRtn;
}

/*
TRuleReturn *IndexTER(TShop *pShop,TMcAvail &MCAvail,TJob &jbX,TDGraph &Graph,double fK1,double fK2)
{// suitable for workcenter scheduling
  TRuleReturn *pRtn = new TRuleReturn;
  TOperation  *popTemp,*popTemp2,
              *popLastInSeq;
  TJob     *pJob;
  TMachine *pmcSelect,
           *pmcTemp;
  int i,j,k,iN,iNumSink,
      iFlag=0;
  TArrayF fComplete,fNComplete,fDue,fNDue,fWeight,fNWeight,fEWeight,fNEWeight,fLTime;
  TArrayL SinkID;
  TStackP stkNodeRemain;
  cStack_f stkNodeRemainEST;
  double fLength,
         fAvgSetup = MCAvail.fData2,
         fActProc=0,fActSetup=0,
         fTimeTemp,
         fIndex,
         fWorstIndex=-fBigM,
         fMin,fMin_Length,fMin_Setup,
         fMinStart=fBigM,
         fTemp,
         fL,
         fMinDue;
  TStatus sS1,sS2;
  TNode  *pSinkNode,
         *pndTemp,
         *pndBefore,
         *pndFrom,
         *pndTo,
         *pndNew;
  TWorkcenter *pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
  //TDGraph NewGraph;
  TStackP stkPostN;
  TArc   *pArc;

  double fMinMCRelease=fBigM,
         fMinRelease=fBigM;

  pRtn->iDataReCalc = 1;

//* this part will remove the operation that has not arrived
  for(i=1;i<=MCAvail.Num();i++)
  {
    if(MCAvail.fTime[i]<fMinMCRelease)
    {
      fMinMCRelease = MCAvail.fTime[i];
      pRtn->mc=MCAvail.GetMC(i);
    }
  }

  iN = jbX.Num();
  for(i=1;i<=iN;i++)
  {
    popTemp = jbX.Get(i);
    if(popTemp->fReleaseTime>fMinMCRelease)
    {
      if(popTemp->fReleaseTime<fMinRelease)
      {
        if(fMinRelease<fBigM)
          iFlag = 1;
        fMinRelease = popTemp->fReleaseTime;
        pRtn->op=(Graph.GetID(popTemp->oID))->op;
      }
      if(popTemp->fReleaseTime>fMinMCRelease+pRtn->op->fProcessTime/pRtn->mc->fSpeed)
      {
        jbX.Delete(i);
        i--;
        iN--;
      }
    }
  }
  if(iFlag)
    for(i=1;i<=iN;i++)
    {
      popTemp = jbX.Get(i);
      if(popTemp->fReleaseTime>fMinMCRelease+pRtn->op->fProcessTime/pRtn->mc->fSpeed)
      {
        jbX.Delete(i);
        i--;
        iN--;
      }
    }
  if(jbX.Num()==0)
    return pRtn;


 if(MCAvail.fData1==0) // need to determine the fAvgSetup (fAvgProcess == 0)
   fAvgSetup = pShop->AvgSetup();

 iN = Graph.NumSink();
 for(i=iN;i>=1;i--)
  {
    pSinkNode   = (TNode*)Graph.GetSink(i);
    pJob        = (TJob*)pShop->GetJob(i);
    fComplete[32766+pSinkNode->nID] = pSinkNode->EST();
    //fNComplete[i-1]= pSinkNode->EST();
    fLTime[32766+pSinkNode->nID]    = 0;
    fDue[32766+pSinkNode->nID]      = pJob->fDueDate;
    fWeight[32766+pSinkNode->nID]   = pJob->fWeight;
    fEWeight[32766+pSinkNode->nID]  = pJob->fEWeight;
    SinkID[32766+pSinkNode->nID]    = pSinkNode->nID;
  }

 jbX.Head();
 for(k=1;k<=jbX.Num();k++)
  {
    popTemp2 = (TOperation*)jbX.Next();
    pndTemp = Graph.GetID(popTemp2->oID);
    popTemp = pndTemp->op;
    // select machine
    fMin = fBigM;
    for(j=1;j<=popTemp->stkMachine.Num();j++)
    {
      // select machine with min complete time
      // if they available before operation -> select the fastest machine
      pmcTemp  = popTemp->GetMachine(j);
      if(MCAvail.IsMember(pmcTemp->mID))
      {
        fActProc = popTemp->fProcessTime/pmcTemp->fSpeed;
        fLength  = fActProc;
        popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
        if(fAvgSetup)
        {
          // find the index for the operation on the selected mc
          // Link previous node (popLastInSeq) with the focus operation
          if(popLastInSeq)
            sS1 = popLastInSeq->sMC_Status;
          else
            sS1 = pmcTemp->sInit;
          sS2 = popTemp->sMC_Status;
          fActSetup = pmcTemp->suSetup.Get(sS1,sS2);
        }

        fTimeTemp = max(MCAvail.Get(pmcTemp->mID)+fActSetup,popTemp2->fReleaseTime);
        if(fTimeTemp+fLength<fMin)
        {
          fMin=fTimeTemp+fLength;
          pmcSelect    = pmcTemp;
          fMin_Length  = fLength;
          fMin_Setup   = fActSetup;
          //fMin_popLast = popLastInSeq;
          if(popLastInSeq)
            pndBefore    = Graph.GetID(popLastInSeq->oID);
          else
            pndBefore    = Graph.GetID(0);
        }
      }
    }

    // find the index for the operation on the selected mc
    // Link previous node (popLastInSeq) with the focus operation

    stkPostN.Push(pndTemp,0);
    for(i=0;i<iN;i++)
      fLTime[i]=0;
    while(stkPostN.Num()>0)
    {
      fL       = stkPostN.Index(1);
      pndFrom  = (TNode*)stkPostN.Pop(1);
      for(i=1;i<=pndFrom->stkPostArc.Num();i++)
      {
        pArc    = pndFrom->GetPostArcPtr(i);
        pndTo   = pArc->pndTo;
        if(pndTo->IsSink())
        {
          fLTime[32766+pndTo->nID] = max(fLTime[32766+pndTo->nID],fL+pArc->fWeight);
        } else
        {
          stkPostN.Push(pndTo,fL+pArc->fWeight);
        }
      }
    }
  //  iNumSink=0;
    for(i=0;i<iN;i++)
    {
      fNComplete[i]   = fComplete[i];
      if(fLTime[i]>0)
      {
        fTemp        = fMin;
        fTemp       += fLTime[i]-popTemp->fProcessTime;
        fNComplete[i] = max(fTemp,fComplete[i]);
      }
    }

///////////***
    NewGraph = Graph;
    pndNew   = NewGraph.GetID(pndTemp->nID);
    // reduce the precedence constraints
    for(i=1;i<=pndNew->stkPostArc.Num();i++)
    {
      pndTemp = pndNew->GetPostNode(i);
      if(!pndTemp->IsSink())
      {
        NewGraph.Reduce(pndTemp);
        i--;
      }
    }

    iNumSink = 0;
    for(i=0;i<iN;i++)
      fNComplete[i] = fComplete[i];
    for(i=1;i<=pndNew->stkPostArc.Num();i++)
    {
      fTemp        = fMin;
      fTemp       += pndNew->GetPostWeight(i)-popTemp->fProcessTime;
      pndTemp      = pndNew->GetPostNode(i);
      fNComplete[32766+pndTemp->nID] = max(fTemp,fComplete[32766+pndTemp->nID]);
    }
/////////////***

    fIndex = exp(WTeE(iN,fDue,fComplete,fWeight,fEWeight)/fK1)/fMin_Length;
//    fIndex = WTeE(iN,fDue,fNComplete,fWeight,fEWeight);

    // if there is setup

    if(fAvgSetup>0)
      fIndex *= exp(-fMin_Setup/(fK2*fAvgSetup));

    if(fIndex>fWorstIndex-fSmall)
    {
      if(equal(fIndex,fWorstIndex))
      {
        if(popTemp2->fDueDate<fMinDue)
        {
          fWorstIndex = fIndex;
          pRtn->op    = popTemp;
          pRtn->mc    = pmcSelect;
          fMinDue     = popTemp2->fDueDate;
        }
      } else
      {
        fWorstIndex = fIndex;
        pRtn->op    = popTemp;
        pRtn->mc    = pmcSelect;
        fMinDue     = popTemp2->fDueDate;
      }
    }
  }
  return pRtn;
}
*/

TRuleReturn* IndexTER(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TJob* pJob;
    TMachine *pmcSelect, *pmcTemp;
    int i, j, k, iN = Graph.NumSink() + 1,  // iNumSink,
        iFlag = 0, iTemp;
    TArrayF fComplete(iN), fNComplete(iN), fDue(iN), fWeight(iN), fEWeight(iN), fLTime(iN), fMinRelease(iN);
    TArrayL SinkID(iN);
    TStackP stkPostN;
    TArrayP popFirst;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fTimeTemp, fIndex,
                    fWorstIndex = -fBigM, fMin, fMin_Length, fMin_Setup, fMinStart = fBigM, fTemp, fL,
                    fMinDue = 1e10, fSum;
    TStatus sS1, sS2;
    TNode *pSinkNode, *pndTemp, *pndBefore, *pndFrom, *pndTo;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
    TArc* pArc;

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM;

    pRtn->iDataReCalc = 1;

    //* this part will remove the operation that has not arrived
    for (i = MCAvail.Num(); i >= 1; i--)
    {
        iTemp = MCAvail.GetMC(i)->mID;
        fMinRelease[iTemp] = fBigM;
        popFirst.Put(iTemp, 0);
    }
    iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        i = max(i, 1);
        popTemp = jbX.Get(i);

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fTemp = MCAvail.Get(pmcTemp->mID);
                if (popTemp->fReleaseTime > fTemp)
                {
                    if (popTemp->fReleaseTime < fMinRelease[pmcTemp->mID])
                    {
                        if (fMinRelease[pmcTemp->mID] < fBigM) iFlag = 1;
                        fMinRelease[pmcTemp->mID] = popTemp->fReleaseTime;
                        popFirst.Put(pmcTemp->mID, popTemp);
                        if (fMinReleaseAll > popTemp->fReleaseTime)
                        {
                            pRtn->op = (Graph.GetID(popTemp->oID))->op;
                            pRtn->mc = pmcTemp;
                            fMinReleaseAll = popTemp->fReleaseTime;
                        }
                    }
                    if (popTemp->fReleaseTime >
                        fMinRelease
                            [pmcTemp->mID] /*+((TOperation*)popFirst[pmcTemp->mID])->fProcessTime/pmcTemp->fSpeed*/)
                    {
                        jbX.Delete(i);
                        i--;
                        if (i < 1) i = 1;
                        iN--;
                    }
                }
            }
        }
    }
    if (iFlag == 1)
    {  // need further remove
        iN = jbX.Num();
        for (i = 1; i <= iN; i++)
        {
            popTemp = jbX.Get(i);
            for (j = 1; j <= popTemp->stkMachine.Num(); j++)
            {
                pmcTemp = popTemp->GetMachine(j);
                if ((MCAvail.IsMember(pmcTemp->mID)) && (popFirst[pmcTemp->mID]))
                {
                    if (popTemp->fReleaseTime >
                        fMinRelease[pmcTemp->mID] +
                            ((TOperation*)popFirst[pmcTemp->mID])->fProcessTime / pmcTemp->fSpeed)
                    {
                        jbX.Delete(i);
                        i--;
                        if (i < 1) i = 1;
                        iN--;
                    }
                }
            }
        }
    }
    if (jbX.Num() == 0) return pRtn;

    if (MCAvail.fData1 == 0)  // need to determine the fAvgSetup (fAvgProcess == 0)
        fAvgSetup = pShop->AvgSetup();

    iN = Graph.NumSink();
    for (i = iN; i >= 1; i--)
    {
        pSinkNode = (TNode*)Graph.GetSink(i);
        // pJob        = pShop->GetJob(i);
        pJob = pShop->GetJob(pSinkNode->nID);
        fComplete[32766 + pSinkNode->nID] = pSinkNode->EST();
        // fComplete[32766+pSinkNode->nID] = 0;
        // fNComplete[i-1]= pSinkNode->EST();
        fLTime[32766 + pSinkNode->nID] = 0;
        fDue[32766 + pSinkNode->nID] = pJob->fDueDate;
        fWeight[32766 + pSinkNode->nID] = pJob->fWeight;
        fEWeight[32766 + pSinkNode->nID] = pJob->fEWeight;
        SinkID[32766 + pSinkNode->nID] = pSinkNode->nID;
    }

    jbX.Head();
    for (k = 1; k <= jbX.Num(); k++)
    {
        popTemp2 = (TOperation*)jbX.Next();
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        // select machine
        fMin = fBigM;
        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            // select machine with min complete time
            // if they available before operation -> select the fastest machine
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
                fLength = fActProc;
                popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
                if (fAvgSetup)
                {
                    // find the index for the operation on the selected mc
                    // Link previous node (popLastInSeq) with the focus operation
                    if (popLastInSeq)
                        sS1 = popLastInSeq->sMC_Status;
                    else
                        sS1 = pmcTemp->sInit;
                    sS2 = popTemp->sMC_Status;
                    fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
                }

                fTimeTemp = max(MCAvail.Get(pmcTemp->mID) + fActSetup, popTemp2->fReleaseTime);
                if (fTimeTemp + fLength < fMin)
                {
                    fMin = fTimeTemp + fLength;
                    pmcSelect = pmcTemp;
                    fMin_Length = fLength;
                    fMin_Setup = fActSetup;
                    // fMin_popLast = popLastInSeq;
                    if (popLastInSeq)
                        pndBefore = Graph.GetID(popLastInSeq->oID);
                    else
                        pndBefore = Graph.GetID(0);
                }
            }
        }

        // find the index for the operation on the selected mc
        // Link previous node (popLastInSeq) with the focus operation

        stkPostN.Push(pndTemp, 0);
        for (i = 0; i < iN; i++) fLTime[i] = 0;
        while (stkPostN.Num() > 0)
        {
            fL = stkPostN.Index(1);
            pndFrom = (TNode*)stkPostN.Pop(1);
            for (i = 1; i <= pndFrom->stkPostArc.Num(); i++)
            {
                pArc = pndFrom->GetPostArcPtr(i);
                pndTo = pArc->pndTo;
                if (pndTo->IsSink())
                {
                    fLTime[32766 + pndTo->nID] = max(fLTime[32766 + pndTo->nID], fL + pArc->fWeight);
                }
                else
                {
                    stkPostN.Push(pndTo, fL + pArc->fWeight);
                }
            }
        }
        //  iNumSink=0;
        fSum = 0;
        for (i = 0; i < iN; i++)
        {
            fNComplete[i] = fComplete[i];
            // fNComplete[i]     = -fBigM;
            if (fLTime[i] > 0)
            {
                fTemp = fMin;
                fTemp += fLTime[i] - popTemp->fProcessTime;
                fNComplete[i] = max(fTemp, fComplete[i]);
                // fNComplete[i] = fTemp;
            }
        }

        // IndexShopTER()
        //    fIndex = exp(WTeE(iN,fDue,fNComplete,fWeight,fEWeight)/fK1)/fMin_Length;
        fIndex = WTeE(iN, fDue, fNComplete, fWeight, fEWeight);

        // if there is setup

        //    if(fAvgSetup>0)
        //      fIndex += exp(-fMin_Setup/(fK2*fAvgSetup));

        if (fIndex >= fWorstIndex - fSmall)
        {
            if (equal(fIndex, fWorstIndex))
            {
                if (popTemp2->fDueDate < fMinDue)
                {
                    fWorstIndex = fIndex;
                    pRtn->op = popTemp;
                    pRtn->mc = pmcSelect;
                    fMinDue = popTemp2->fDueDate;
                }
            }
            else
            {
                fWorstIndex = fIndex;
                pRtn->op = popTemp;
                pRtn->mc = pmcSelect;
                fMinDue = popTemp2->fDueDate;
            }
        }
    }
    return pRtn;
}

TRuleReturn* IndexTER2(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TMachine* pmcTemp;
    int i, j, iN, iM;

    TArrayF fMinRelease;
    TArrayP popFirst, pmcOperation;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fWorstIndex = -fBigM,
                    fMinStart = fBigM, fBestIndex, fMinDue = 1e10;
    TStatus sS1, sS2;
    TNode *pndTemp, *pSinkNode, *pndFrom, *pndTo;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
    TArrayF fComplete, fNComplete, fDue, fWeight, fEWeight, fLTime;
    TArrayL SinkID;
    TStackP stkPostN;

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM, fFirstComplete, fOpComplete, fMinCompleteForOp,
           fMin_Setup, fMin_ActProcess;
    TMachine* pmcFirstComplete;
    TOperation* popFirstComplete;
    TJob* pJob;
    TArrayF aIndex;
    TDGraph grhTemp;

    pRtn->iDataReCalc = 1;

    if (MCAvail.fData1 == 0)  // need to determine the fAvgSetup (fAvgProcess == 0)
        fAvgSetup = pShop->AvgSetup();

    iM = Graph.NumSink();
    for (i = iM; i >= 1; i--)
    {
        pSinkNode = (TNode*)Graph.GetSink(i);
        // pJob        = pShop->GetJob(i);
        pJob = pShop->GetJob(pSinkNode->nID);
        fComplete[32766 + pSinkNode->nID] = pSinkNode->EST();
        // fComplete[32766+pSinkNode->nID] = 0;
        // fNComplete[i-1]= pSinkNode->EST();
        fLTime[32766 + pSinkNode->nID] = 0;
        fDue[32766 + pSinkNode->nID] = pJob->fDueDate;
        fWeight[32766 + pSinkNode->nID] = pJob->fWeight;
        fEWeight[32766 + pSinkNode->nID] = pJob->fEWeight;
        SinkID[32766 + pSinkNode->nID] = pSinkNode->nID;
    }

    //* this part will remove the operation that has not arrived
    fFirstComplete =
        fBigM;  // the completion time of first op, op that release after this time should not be considered
    iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        fMinCompleteForOp = fBigM;

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
            fLength = fActProc;
            popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
            if (fAvgSetup)
            {
                if (popLastInSeq)
                    sS1 = popLastInSeq->sMC_Status;
                else
                    sS1 = pmcTemp->sInit;
                sS2 = popTemp->sMC_Status;
                fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
            }
            else
                fActSetup = 0;
            fOpComplete = MCAvail.Get(pmcTemp->mID) + fActSetup;
            if (popTemp2->fReleaseTime > fOpComplete) fOpComplete = popTemp2->fReleaseTime;
            fOpComplete += fActProc;
            if (fOpComplete < fMinCompleteForOp)
            {
                pmcOperation.Put(popTemp->oID, pmcTemp);
                fMin_Setup = fActSetup;
                fMinCompleteForOp = fOpComplete;
                fMin_ActProcess = fActProc;

                if (fOpComplete < fFirstComplete)
                {
                    fFirstComplete = fOpComplete;
                    popFirstComplete = popTemp2;
                    pmcFirstComplete = pmcTemp;
                }
            }
        }
        // calc index put in aIndex[oID]
        if (popTemp2->fReleaseTime > fFirstComplete)
            aIndex[popTemp2->oID] = -fBigM;
        else
        {  //////////////// calc index for popTemp2 //////////////
            // aIndex[popTemp2->oID] // pmcOperation[popTemp2->oID] //

            grhTemp = Graph;
            // link popTemp with other nodes those have prior links
            pndFrom = grhTemp.GetID(MCAvail.LastOp(pmcFirstComplete->mID));
            for (j = 1; j <= grhTemp.Num(); j++)
            {
                pndTo = grhTemp.Get(j);
                if ((pndTo != pndFrom) && (pndTo->stkPriorArc.Num() == 0))
                {
                    grhTemp.Link(pndFrom, pndTo, fMin_ActProcess);
                }
            }
            grhTemp.ReCalcEST();
            // calc obj
            for (j = iM; j >= 1; j--)
            {
                pSinkNode = (TNode*)grhTemp.GetSink(j);
                fNComplete[32766 + pSinkNode->nID] = pSinkNode->EST();
            }

            aIndex[popTemp2->oID] = WTeE(iN, fDue, fNComplete, fWeight, fEWeight);
            // aIndex[popTemp2->oID]  = fIndex;
        }  //////////////////////end calc index //////////////////
    }
    // find best index that its release time<=fFirstComplete
    pRtn->op = popFirstComplete;
    pRtn->mc = pmcFirstComplete;
    fBestIndex = fBigM;
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = (TOperation*)jbX.Get(i);
        if ((aIndex[popTemp2->oID] >= fBestIndex - fSmall) &&
            (popTemp2->fReleaseTime <= fFirstComplete - fSmall))
        {
            if (equal(aIndex[popTemp2->oID], fBestIndex))
            {
                if (popTemp2->fDueDate < fMinDue)
                {
                    pRtn->op = popTemp2;
                    pRtn->mc = (TMachine*)pmcOperation.Get(popTemp2->oID);
                    fBestIndex = aIndex[popTemp2->oID];
                    fMinDue = popTemp2->fDueDate;
                }
                pRtn->op = popTemp2;
                pRtn->mc = (TMachine*)pmcOperation.Get(popTemp2->oID);
                fBestIndex = aIndex[popTemp2->oID];
                fMinDue = popTemp2->fDueDate;
            }
        }
    }
    return pRtn;
}

TRuleReturn* IndexTER3(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TJob* pJob;
    TMachine *pmcSelect, *pmcTemp;
    int i, j, k, iN,  // iNumSink,
        iFlag = 0, iTemp;
    TArrayF fComplete, fNComplete, fDue, fWeight, fEWeight, fLTime, fMinRelease;
    TArrayL SinkID;
    TStackP stkPostN;
    TArrayP popFirst;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fTimeTemp, fIndex,
                    fWorstIndex = -fBigM, fMin, fMin_Length, fMin_Setup, fMinStart = fBigM, fTemp,
                    fMinDue = 1e10;
    TStatus sS1, sS2;
    TNode *pSinkNode, *pndTemp, *pndBefore, *pndFrom, *pndTo;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
    TDGraph grhTemp;

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM;

    pRtn->iDataReCalc = 1;

    //* this part will remove the operation that has not arrived
    for (i = MCAvail.Num(); i >= 1; i--)
    {
        iTemp = MCAvail.GetMC(i)->mID;
        fMinRelease[iTemp] = fBigM;
        popFirst.Put(iTemp, 0);
    }
    iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        i = max(i, 1);
        popTemp = jbX.Get(i);

        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fTemp = MCAvail.Get(pmcTemp->mID);
                if (popTemp->fReleaseTime > fTemp)
                {
                    if (popTemp->fReleaseTime < fMinRelease[pmcTemp->mID])
                    {
                        if (fMinRelease[pmcTemp->mID] < fBigM) iFlag = 1;
                        fMinRelease[pmcTemp->mID] = popTemp->fReleaseTime;
                        popFirst.Put(pmcTemp->mID, popTemp);
                        if (fMinReleaseAll > popTemp->fReleaseTime)
                        {
                            pRtn->op = (Graph.GetID(popTemp->oID))->op;
                            pRtn->mc = pmcTemp;
                        }
                    }
                    if (popTemp->fReleaseTime > fMinRelease[pmcTemp->mID])
                    {
                        jbX.Delete(i);
                        i--;
                        if (i < 1) i = 1;
                        iN--;
                    }
                }
            }
        }
    }
    if (iFlag == 1)
    {
        iN = jbX.Num();
        for (i = 1; i <= iN; i++)
        {
            popTemp = jbX.Get(i);
            for (j = 1; j <= popTemp->stkMachine.Num(); j++)
            {
                pmcTemp = popTemp->GetMachine(j);
                if ((MCAvail.IsMember(pmcTemp->mID)) && (popFirst[pmcTemp->mID]))
                {
                    if (popTemp->fReleaseTime > fMinRelease[pmcTemp->mID])
                    {
                        jbX.Delete(i);
                        i--;
                        if (i < 1) i = 1;
                        iN--;
                    }
                }
            }
        }
    }
    if (jbX.Num() == 0) return pRtn;

    if (MCAvail.fData1 == 0)  // need to determine the fAvgSetup (fAvgProcess == 0)
        fAvgSetup = pShop->AvgSetup();

    iN = Graph.NumSink();
    for (i = iN; i >= 1; i--)
    {
        pSinkNode = (TNode*)Graph.GetSink(i);
        // pJob        = pShop->GetJob(i);
        pJob = pShop->GetJob(pSinkNode->nID);
        fComplete[32766 + pSinkNode->nID] = pSinkNode->EST();
        // fComplete[32766+pSinkNode->nID] = 0;
        // fNComplete[i-1]= pSinkNode->EST();
        fLTime[32766 + pSinkNode->nID] = 0;
        fDue[32766 + pSinkNode->nID] = pJob->fDueDate;
        fWeight[32766 + pSinkNode->nID] = pJob->fWeight;
        fEWeight[32766 + pSinkNode->nID] = pJob->fEWeight;
        SinkID[32766 + pSinkNode->nID] = pSinkNode->nID;
    }

    jbX.Head();
    for (k = 1; k <= jbX.Num(); k++)
    {
        popTemp2 = (TOperation*)jbX.Next();
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        // select machine
        fMin = fBigM;
        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            // select machine with min complete time
            // if they available before operation -> select the fastest machine
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
                fLength = fActProc;
                popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
                if (fAvgSetup)
                {
                    // find the index for the operation on the selected mc
                    // Link previous node (popLastInSeq) with the focus operation
                    if (popLastInSeq)
                        sS1 = popLastInSeq->sMC_Status;
                    else
                        sS1 = pmcTemp->sInit;
                    sS2 = popTemp->sMC_Status;
                    fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
                }

                fTimeTemp = max(MCAvail.Get(pmcTemp->mID) + fActSetup, popTemp2->fReleaseTime);
                if (fTimeTemp + fLength < fMin)
                {
                    fMin = fTimeTemp + fLength;
                    pmcSelect = pmcTemp;
                    fMin_Length = fLength;
                    fMin_Setup = fActSetup;
                    // fMin_popLast = popLastInSeq;
                    if (popLastInSeq)
                        pndBefore = Graph.GetID(popLastInSeq->oID);
                    else
                        pndBefore = Graph.GetID(0);
                }
            }
        }

        // find the index for the operation on the selected mc
        // Link previous node (popLastInSeq) with the focus operation

        grhTemp = Graph;
        // link popTemp with other nodes those have prior links
        pndFrom = grhTemp.GetID(MCAvail.LastOp(pmcSelect->mID));
        for (j = 1; j <= grhTemp.Num(); j++)
        {
            pndTo = grhTemp.Get(j);
            if ((pndTo != pndFrom) && (pndTo->stkPriorArc.Num() == 0))
            {
                grhTemp.Link(pndFrom, pndTo, fMin_Length);
            }
        }
        grhTemp.ReCalcEST();
        // calc obj
        for (j = iN; j >= 1; j--)
        {
            pSinkNode = (TNode*)grhTemp.GetSink(j);
            fNComplete[32766 + pSinkNode->nID] = pSinkNode->EST();
        }

        // IndexShopTER()
        //    fIndex = exp(WTeE(iN,fDue,fNComplete,fWeight,fEWeight)/fK1)/fMin_Length;
        fIndex = WTeE(iN, fDue, fNComplete, fWeight, fEWeight);

        // if there is setup

        //    if(fAvgSetup>0)
        //      fIndex += exp(-fMin_Setup/(fK2*fAvgSetup));

        if (fIndex >= fWorstIndex - fSmall)
        {
            if (equal(fIndex, fWorstIndex))
            {
                if (popTemp2->fDueDate < fMinDue)
                {
                    fWorstIndex = fIndex;
                    pRtn->op = popTemp;
                    pRtn->mc = pmcSelect;
                    fMinDue = popTemp2->fDueDate;
                }
            }
            else
            {
                fWorstIndex = fIndex;
                pRtn->op = popTemp;
                pRtn->mc = pmcSelect;
                fMinDue = popTemp2->fDueDate;
            }
        }
    }
    return pRtn;
}

/*
TRuleReturn *IndexWTail2(TShop *pShop,TMcAvail &MCAvail,TJob &jbX,TDGraph &Graph,double fK1,double fK2)
{// change -- include every job in the index
  TRuleReturn *pRtn = new TRuleReturn;
  TOperation  *popTemp,*popTemp2,
              *popLastInSeq;
  TMachine *pmcTemp;
  int       i,j,k,iN,iM;

  TArrayF  fMinRelease;
  TArrayP  popFirst,
            pmcOperation;
  double fLength,
         fAvgSetup = MCAvail.fData2,
         fActProc=0,fActSetup=0,
         fIndex,
         fWorstIndex=-fBigM,
         fMinStart=fBigM,
         fBestIndex;
  TStatus sS1,sS2;
  TNode  *pndTemp;
  TWorkcenter *pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);


  double fMinMCRelease=fBigM,
         fMinReleaseAll=fBigM,
         fFirstComplete,
         fMinCompleteForOp,
         fComplete,
         fMin_Setup;
  TMachine *pmcFirstComplete;
  TOperation *popFirstComplete;
  TArrayF aIndex;

//  pRtn->iDataReCalc = 1;

//* this part will remove the operation that has not arrived
  fFirstComplete = fBigM; // the completion time of first op, op that release after this time should not be
considered iN = jbX.Num();

  for(i=1;i<=iN;i++)
  {
    popTemp2 = (TOperation*)jbX.Get(i);
    pndTemp = Graph.GetID(popTemp2->oID);
    popTemp = pndTemp->op; //*****
    fMinCompleteForOp = fBigM;

    for(j=1;j<=popTemp->stkMachine.Num();j++)
    {
      pmcTemp = popTemp->GetMachine(j);
      fActProc = popTemp->fProcessTime/pmcTemp->fSpeed;
      fLength  = fActProc;
      popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
      if(fAvgSetup)
      {
        if(popLastInSeq)
          sS1 = popLastInSeq->sMC_Status;
        else
          sS1 = pmcTemp->sInit;
        sS2 = popTemp->sMC_Status;
        fActSetup = pmcTemp->suSetup.Get(sS1,sS2);
      }
      else
        fActSetup = 0;
      fComplete = MCAvail.Get(pmcTemp->mID)+fActSetup;
      if(popTemp2->fReleaseTime>fComplete)
        fComplete = popTemp2->fReleaseTime;
      fComplete += fActProc;
      if(fComplete<fMinCompleteForOp)
      {
        pmcOperation.Put(popTemp->oID,pmcTemp);
        fMin_Setup = fActSetup;
        fMinCompleteForOp = fComplete;


        if(fComplete<fFirstComplete)
        {
          fFirstComplete = fComplete;
          popFirstComplete = popTemp2;
          pmcFirstComplete = pmcTemp;
        }
      }
    }
    // calc index put in aIndex[oID]
    if(popTemp2->fReleaseTime>fFirstComplete)
      aIndex[popTemp2->oID]=fBigM;
    else
    { //////////////// calc index for popTemp2 //////////////
      // aIndex[popTemp2->oID] // pmcOperation[popTemp2->oID] //
      // set tails = 0
      iM = Graph.NumSink();
      for(k=1;k<=iM;k++)
      {
        fTail[32766+pSinkNode->nID]=0;
      }

      for(k=1;k<=pndTemp->stkPostArc.Num();k++)
      {
        paTo = pndTemp->GetPostArcPtr(k);
        pndTo = paTo->pndTo;
        if(pndTo->IsSink())
          if(paTo->fWeight>fTail[(pndTo->nID+32766)])
            fTail[(pndTo->nID+32766)] = paTo->fWeight-popTemp->fProcessTime/pmcSelect->fSpeed;
        else
          stkPostN.Push(pndTo,paTo->fWeight);
      }
      while(stkPostN.Num())
      {
        fCTime = stkPostN.Index(1);
        pndFrom = (TNode*)stkPostN.Pop(1);
        for(k=1;k<=pndFrom->stkPostArc.Num();k++)
        {
          paTo = pndFrom->GetPostArcPtr(k);
          pndTo = paTo->pndTo;
          if(pndTo->IsSink())
            if((fCTime+paTo->fWeight)>fTail[(pndTo->nID+32766)])
              fTail[(pndTo->nID+32766)] = fCTime+paTo->fWeight;
          else
            stkPostN.Push(pndTo,(fCTime+paTo->fWeight));
        }
      }

      fIndex = 0;
      for(k=iN;k>=1;k--)
      {
        pSinkNode   = (TNode*)Graph.GetSink(k);
        pJob        = pShop->GetJob(pSinkNode->nID);
        fIndex     += (pJob->fWeight*fTail[(32766+pSinkNode->nID)]);
      }
      aIndex[popTemp2->oID] = fIndex;
    } //////////////////////end calc index //////////////////

  }
  // find best index that its release time<=fFirstComplete
  pRtn->op = popFirstComplete;
  pRtn->mc = pmcFirstComplete;
  fBestIndex = fBigM;
  for(i=1;i<=iN;i++)
  {
    popTemp2 = (TOperation*)jbX.Get(i);
    if((aIndex[popTemp2->oID]<fBestIndex)&&(popTemp2->fReleaseTime<=fFirstComplete))
    {
      pRtn->op = popTemp2;
      pRtn->mc = (TMachine*) pmcOperation.Get(popTemp2->oID);
      fBestIndex = aIndex[popTemp2->oID];
    }
  }
  return pRtn;
}
*/

TRuleReturn* IndexWTail(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{  // change -- include every job in the index
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TJob* pJob;
    TMachine *pmcSelect, *pmcTemp;
    int i, j, k, iN,  // iNumSink,
        iFlag = 0, iTemp;
    TArrayF fTail,  // fNComplete,fDue,fWeight,fEWeight,fLTime,
        fMinRelease;
    TArrayL SinkID;
    TStackP stkPostN;
    TArrayP popFirst, pmcOperation;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fIndex, fWorstIndex = -fBigM,
                    fMinStart = fBigM, fCTime, fTemp, fMin, fTimeTemp, fMin_Length, fMin_Setup,
                    fMinDue = 1e10;
    TStatus sS1, sS2;
    TNode *pSinkNode, *pndTemp, *pndFrom, *pndTo, *pndBefore;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
    TArc* paTo;

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM;
    TArrayF aIndex;

    pRtn->iDataReCalc = 1;

    //* this part will remove the operation that has not arrived
    for (i = MCAvail.Num(); i >= 1; i--)
    {
        iTemp = MCAvail.GetMC(i)->mID;
        fTemp = MCAvail.Get(i);
        if (fTemp < fMinMCRelease)
        {
            fMinMCRelease = fTemp;
            pRtn->mc = MCAvail.GetMC(i);
        }
    }
    iN = jbX.Num();
    for (i = 1; i <= iN; i++)
    {
        i = max(i, 1);
        popTemp = jbX.Get(i);
        if (popTemp->fReleaseTime < fMinReleaseAll)
        {
            fMinReleaseAll = popTemp->fReleaseTime;
            pRtn->op = (Graph.GetID(popTemp->oID))->op;
        }
        if (popTemp->fReleaseTime > fMinMCRelease)
        {
            jbX.Delete(i);
            i--;
            if (i < 1) i = 1;
            iN--;
        }
    }

    if (jbX.Num() == 0) return pRtn;

    for (k = 1; k <= jbX.Num(); k++)
    {
        popTemp2 = jbX.Get(k);
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;  //*****
        // select machine
        fMin = fBigM;
        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            // select machine with min complete time
            // if they available before operation -> select the fastest machine
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
                fLength = fActProc;
                popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
                if (fAvgSetup)
                {
                    // find the index for the operation on the selected mc
                    // Link previous node (popLastInSeq) with the focus operation
                    if (popLastInSeq)
                        sS1 = popLastInSeq->sMC_Status;
                    else
                        sS1 = pmcTemp->sInit;
                    sS2 = popTemp->sMC_Status;
                    fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
                }

                fTimeTemp = max(MCAvail.Get(pmcTemp->mID) + fActSetup, popTemp2->fReleaseTime);
                if (fTimeTemp + fLength < fMin)
                {
                    fMin = fTimeTemp + fLength;
                    pmcSelect = pmcTemp;
                    fMin_Length = fLength;
                    fMin_Setup = fActSetup;
                    // fMin_popLast = popLastInSeq;
                    if (popLastInSeq)
                        pndBefore = Graph.GetID(popLastInSeq->oID);
                    else
                        pndBefore = Graph.GetID(0);
                }
            }
        }

        // set tails = 0
        iN = Graph.NumSink();
        for (i = 1; i <= iN; i++)
        {
            pSinkNode = (TNode*)Graph.GetSink(i);
            fTail[32766 + pSinkNode->nID] = 0;
        }

        for (i = 1; i <= pndTemp->stkPostArc.Num(); i++)
        {
            paTo = pndTemp->GetPostArcPtr(i);
            pndTo = paTo->pndTo;
            if (pndTo->IsSink())
                if (paTo->fWeight > fTail[(pndTo->nID + 32766)])
                    fTail[(pndTo->nID + 32766)] = paTo->fWeight - popTemp->fProcessTime / pmcSelect->fSpeed;
                else
                    stkPostN.Push(pndTo, paTo->fWeight);
        }
        while (stkPostN.Num())
        {
            fCTime = stkPostN.Index(1);
            pndFrom = (TNode*)stkPostN.Pop(1);
            for (i = 1; i <= pndFrom->stkPostArc.Num(); i++)
            {
                paTo = pndFrom->GetPostArcPtr(i);
                pndTo = paTo->pndTo;
                if (pndTo->IsSink())
                    if ((fCTime + paTo->fWeight) > fTail[(pndTo->nID + 32766)])
                        fTail[(pndTo->nID + 32766)] = fCTime + paTo->fWeight;
                    else
                        stkPostN.Push(pndTo, (fCTime + paTo->fWeight));
            }
        }

        fIndex = 0;
        for (i = iN; i >= 1; i--)
        {
            pSinkNode = (TNode*)Graph.GetSink(i);
            pJob = pShop->GetJob(pSinkNode->nID);
            fIndex += (pJob->fWeight * fTail[(32766 + pSinkNode->nID)]);
        }

        if (fIndex >= fWorstIndex - fSmall)
        {
            if (equal(fIndex, fWorstIndex))
            {
                if (popTemp2->fDueDate < fMinDue)
                {
                    fWorstIndex = fIndex;
                    pRtn->op = popTemp;
                    pRtn->mc = pmcSelect;
                    fMinDue = popTemp2->fDueDate;
                }
            }
            else
            {
                fWorstIndex = fIndex;
                pRtn->op = popTemp;
                pRtn->mc = pmcSelect;
                fMinDue = popTemp2->fDueDate;
            }
        }
    }
    return pRtn;
}

TRuleReturn* IndexATC(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{
    /* parameters

        Tau = 1 - avg(d)/Cmax
        R   = (dmax-dmin)/Cmax
        Neu = avg(s)/avg(p)
       Cmax ~ sum(p) + n*avg(s)
        k1  = 4.5 + R    if R<= 0.5
            = 6 - 2*R    otherwise
        k2  = Tau/(2*sqrt(Neu))
      Index = w/p*exp(-max(d-p-t,0)/(k1*avg(p)))*exp(-s/(k2*avg(s)))

      Note:d:due date, s:setup, p:process time, n:#of operations in the job
    */
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TMachine *pmcSelect, *pmcTemp;
    int i, j;
    double  // fComplete[iMaxJob],fDue[iMaxJob],fWeight[iMaxJob],fEWeight[iMaxJob],
        fLength,
        fAvgProcessTime = MCAvail.fData1, fAvgSetup = MCAvail.fData2, fActProc, fActSetup, fTimeTemp, fIndex,
        fWorstIndex = -fBigM, fMin, fMin_Length, fMin_Setup;
    TStatus sS1, sS2;
    TNode *pndTemp, *pndBefore;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);

    if (MCAvail.fData1 == 0)  // need to determine the fAvgSetup & fAvgProcessTime
    {
        fAvgProcessTime = pShop->AvgProcessTime();
        fAvgSetup = pShop->AvgSetup();
    }

    for (i = 1; i <= jbX.Num(); i++)
    {
        popTemp2 = jbX.Get(i);
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;
        // select machine
        fMin = fBigM;
        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            // select machine with min complete time
            // if they available before operation -> select the fastest machine
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
                fLength = fActProc;
                popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
                if (fAvgSetup)
                {
                    // find the index for the operation on the selected mc
                    // Link previous node (popLastInSeq) with the focus operation
                    if (popLastInSeq)
                        sS1 = popLastInSeq->sMC_Status;
                    else
                        sS1 = pmcTemp->sInit;
                    sS2 = popTemp->sMC_Status;
                    fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
                }

                fTimeTemp = max(MCAvail.Get(pmcTemp->mID) + fActSetup, popTemp->fReleaseTime);
                if (fTimeTemp + fLength < fMin)
                {
                    fMin = fTimeTemp + fLength;
                    pmcSelect = pmcTemp;
                    fMin_Length = fLength;
                    fMin_Setup = fActSetup;
                    // fMin_popLast = popLastInSeq;
                    if (popLastInSeq)
                        pndBefore = Graph.GetID(popLastInSeq->oID);
                    else
                        pndBefore = Graph.GetID(0);
                }
            }
        }

        // find the index for the operation on the selected mc
        // Link previous node (popLastInSeq) with the focus operation

        fIndex = popTemp->pJob->fWeight / fMin_Length;
        fIndex *= exp((-max(popTemp2->fDueDate - fMin_Length - MCAvail.Get(pmcTemp->mID), 0)) /
                      (fK1 * fAvgProcessTime));
        // if there is setup
        if (fAvgSetup > 0) fIndex *= exp((-fMin_Setup) / (fK2 * fAvgSetup));

        if (fIndex > fWorstIndex)
        {
            fWorstIndex = fIndex;
            pRtn->op = popTemp;
            pRtn->mc = pmcSelect;
        }
    }
    pRtn->iDataReCalc = 1;
    return pRtn;
}

TRuleReturn* IndexMATC(TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2)
{
    /* parameters

        Tau = 1 - avg(d)/Cmax
        R   = (dmax-dmin)/Cmax
        Neu = avg(s)/avg(p)
       Cmax ~ sum(p) + n*avg(s)
        k1  = 4.5 + R    if R<= 0.5
            = 6 - 2*R    otherwise
        k2  = Tau/(2*sqrt(Neu))
      Index = w/p*exp(-max(d-p-t,0)/(k1*avg(p)))*exp(-s/(k2*avg(s)))

      Note:d:due date, s:setup, p:process time, n:#of operations in the job
    */
    TRuleReturn* pRtn = new TRuleReturn;
    TOperation *popTemp, *popTemp2, *popLastInSeq;
    TJob* pJob;
    TMachine *pmcSelect, *pmcTemp;
    int i, j, k, iN,  // iNumSink,
        iFlag = 0;
    TArrayF fComplete, fNComplete, fDue, fWeight, fEWeight, fLTime, fMinRelease;
    TArrayL SinkID;
    TStackP stkPostN;
    TArrayP popFirst;
    double fLength, fAvgSetup = MCAvail.fData2, fActProc = 0, fActSetup = 0, fTimeTemp, fIndex,
                    fWorstIndex = -fBigM, fMin, fMin_Length, fMin_Setup, fMinStart = fBigM, fTemp, fL,
                    fMinDue = 1e10, fSum, fAvgProcessTime;
    TStatus sS1, sS2;
    TNode *pSinkNode, *pndTemp, *pndBefore, *pndFrom, *pndTo;
    TWorkcenter* pWkCenter = pShop->WkCenter.Getw_mID(MCAvail.mID[1]);
    TArc* pArc;

    double fMinMCRelease = fBigM, fMinReleaseAll = fBigM;

    pRtn->iDataReCalc = 1;

    if (MCAvail.fData1 == 0)  // need to determine the fAvgSetup (fAvgProcess == 0)
        fAvgSetup = pShop->AvgSetup();

    fAvgProcessTime = pShop->AvgProcessTime(pWkCenter);

    iN = Graph.NumSink();
    for (i = iN; i >= 1; i--)
    {
        pSinkNode = (TNode*)Graph.GetSink(i);
        // pJob        = pShop->GetJob(i);
        pJob = pShop->GetJob(pSinkNode->nID);
        fComplete[32766 + pSinkNode->nID] = pSinkNode->EST();
        // fNComplete[i-1]= pSinkNode->EST();
        fLTime[32766 + pSinkNode->nID] = 0;
        fDue[32766 + pSinkNode->nID] = pJob->fDueDate;
        fWeight[32766 + pSinkNode->nID] = pJob->fWeight;
        fEWeight[32766 + pSinkNode->nID] = pJob->fEWeight;
        SinkID[32766 + pSinkNode->nID] = pSinkNode->nID;
    }

    jbX.Head();
    for (k = 1; k <= jbX.Num(); k++)
    {
        popTemp2 = (TOperation*)jbX.Next();
        pndTemp = Graph.GetID(popTemp2->oID);
        popTemp = pndTemp->op;
        // select machine
        fMin = fBigM;
        for (j = 1; j <= popTemp->stkMachine.Num(); j++)
        {
            // select machine with min complete time
            // if they available before operation -> select the fastest machine
            pmcTemp = popTemp->GetMachine(j);
            if (MCAvail.IsMember(pmcTemp->mID))
            {
                fActProc = popTemp->fProcessTime / pmcTemp->fSpeed;
                fLength = fActProc;
                popLastInSeq = Graph.GetID(MCAvail.LastOp(pmcTemp->mID))->op;
                if (fAvgSetup)
                {
                    // find the index for the operation on the selected mc
                    // Link previous node (popLastInSeq) with the focus operation
                    if (popLastInSeq)
                        sS1 = popLastInSeq->sMC_Status;
                    else
                        sS1 = pmcTemp->sInit;
                    sS2 = popTemp->sMC_Status;
                    fActSetup = pmcTemp->suSetup.Get(sS1, sS2);
                }

                fTimeTemp = max(MCAvail.Get(pmcTemp->mID) + fActSetup, popTemp2->fReleaseTime);
                if (fTimeTemp + fLength < fMin)
                {
                    fMin = fTimeTemp + fLength;
                    pmcSelect = pmcTemp;
                    fMin_Length = fLength;
                    fMin_Setup = fActSetup;
                    // fMin_popLast = popLastInSeq;
                    if (popLastInSeq)
                        pndBefore = Graph.GetID(popLastInSeq->oID);
                    else
                        pndBefore = Graph.GetID(0);
                }
            }
        }

        // find the index for the operation on the selected mc
        // Link previous node (popLastInSeq) with the focus operation

        stkPostN.Push(pndTemp, 0);
        for (i = 0; i < iN; i++) fLTime[i] = 0;
        while (stkPostN.Num() > 0)
        {
            fL = stkPostN.Index(1);
            pndFrom = (TNode*)stkPostN.Pop(1);
            for (i = 1; i <= pndFrom->stkPostArc.Num(); i++)
            {
                pArc = pndFrom->GetPostArcPtr(i);
                pndTo = pArc->pndTo;
                if (pndTo->IsSink())
                {
                    fLTime[32766 + pndTo->nID] = max(fLTime[32766 + pndTo->nID], fL + pArc->fWeight);
                }
                else
                {
                    stkPostN.Push(pndTo, fL + pArc->fWeight);
                }
            }
        }
        //  iNumSink=0;
        fSum = 0;
        for (i = 0; i < iN; i++)
        {
            fNComplete[i] = fComplete[i];
            // fNComplete[i]     = -fBigM;
            if (fLTime[i] > 0)
            {
                fTemp = fMin;
                fTemp += fLTime[i] - popTemp->fProcessTime;
                fNComplete[i] = max(fTemp, fComplete[i]);
                // fNComplete[i] = fTemp;
            }
        }
        fIndex = 0;
        for (i = 0; i < iN; i++)
        {
            fTemp = popTemp->fReleaseTime - MCAvail.Get(pmcSelect->mID);
            fTemp = max(0, fTemp);
            fTemp =
                fWeight[i] / fMin_Length * exp((fMin_Length - fLTime[i] - fTemp) / (fK1 * fAvgProcessTime));
            fIndex += fTemp;
        }

        // if there is setup

        //    if(fAvgSetup>0)
        //      fIndex += exp(-fMin_Setup/(fK2*fAvgSetup));

        if (fIndex >= fWorstIndex - fSmall)
        {
            if (equal(fIndex, fWorstIndex))
            {
                if (popTemp2->fDueDate < fMinDue)
                {
                    fWorstIndex = fIndex;
                    pRtn->op = popTemp;
                    pRtn->mc = pmcSelect;
                    fMinDue = popTemp2->fDueDate;
                }
            }
            else
            {
                fWorstIndex = fIndex;
                pRtn->op = popTemp;
                pRtn->mc = pmcSelect;
                fMinDue = popTemp2->fDueDate;
            }
        }
    }
    return pRtn;
}

/*  objective functions

    These functions are used to determine the performance of the shop.
    It is one of the parameter in Workcenter Sequencing function (TTool).
    It may or may not affect the sequencing outcome depending on the
    Workcenter Sequencing function which is one of the parameters in SB().

    User can create his own objective functions. He may develop his own
    Workcenter Sequencing function or use the provided functions. This way,
    he can develop his own version of Shifting Bottleneck.
        iNum      = number of jobs
        fDue      = array of job duedates
        fComplete = array of job completion time
        fWeight   = array of job priorities
*/

double WC(int iNum, double*, double* fComplete, double* fWeight, double*)
{
    double fRtn = 0;
    for (int i = 0; i < iNum; i++)
    {
        fRtn += fWeight[i] * fComplete[i];
    }
    return fRtn;
}

double Cmax(int iNum, double*, double* fComplete, double*, double*)
{
    double fRtn = 0;
    for (int i = 0; i < iNum; i++)
    {
        if (fRtn < fComplete[i]) fRtn = fComplete[i];
    }
    return fRtn;
}

double WLmax(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight)
{
    double fLate, fTemp, fRtn = 0;
    for (int i = 0; i < iNum; i++)
    {
        fLate = fComplete[i] - fDue[i];
        fTemp = fWeight[i] * fLate;
        if (fTemp > fRtn) fRtn = fTemp;
    }
    return fRtn;
}

double WT(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight)
{
    double fLate, fSumWT = 0;  // fSumTardy=0,fSumEarly=0;
    // double fParameter = 100;
    for (int i = 0; i < iNum; i++)
    {
        fLate = fComplete[i] - fDue[i];
        if (fLate > 0) fSumWT += fWeight[i] * fLate;
    }
    return fSumWT;
}

double WTE(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight)
{
    double fLate, fSumWTE = 0;  // fSumTardy=0,fSumEarly=0;
    // double fParameter = 100;
    for (int i = 0; i < iNum; i++)
    {
        fLate = fComplete[i] - fDue[i];
        if (fLate > 0)
            fSumWTE += fWeight[i] * fLate;
        else
            fSumWTE += fEWeight[i] * fLate;
    }
    return fSumWTE;
}

double WTeE(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight)
{
    double fLate, fSumWTeE = 0;  // fSumTardy=0,fSumEarly=0;
    // double fParameter = 100;
    for (int i = 0; i < iNum; i++)
    {
        fLate = fComplete[i] - fDue[i];
        if (fLate > 0)
            fSumWTeE += fWeight[i] * (fLate + 1);
        else
            fSumWTeE += fWeight[i] * (exp(fLate));
    }
    return fSumWTeE;
}

double WU(int iNum, double* fDue, double* fComplete, double* fWeight, double*)
{
    double fLate, fRtn = 0;
    for (int i = 0; i < iNum; i++)
    {
        fLate = fComplete[i] - fDue[i];
        if (fLate > 0) fRtn += fWeight[i];
    }
    return fRtn;
}

double U(int iNum, double* fDue, double* fComplete, double* fWeight, double*)
{
    double fLate, fRtn = 0;
    for (int i = 0; i < iNum; i++)
    {
        fLate = fComplete[i] - fDue[i];
        if (fLate > 0) fRtn++;
    }
    return fRtn;
}

/*  Workcenter Sequecing

*/
double SUB_WTE(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WTE);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            if (WkTool.Seq.Num() > 0) sq = *WkTool.Seq.Get(1);
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WTE);
            if (sq.Num() > 0)
                SWk.Append(sq);
            else
                SWk.PriorityRule(IndexOpDueDate);
            while (SWk.CPI())
                ;
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            double fMin, fTemp;
            TSequenceList sqlMin;
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WTE);
            BWk.PriorityRuleS(IndexBetterEDD);
            fMin = BWk.Obj();
            sqlMin = BWk.Seq;
            BWk.PriorityRule(IndexFCFS);
            fTemp = BWk.Obj();
            if (fTemp < fMin)
            {
                sqlMin = BWk.Seq;
                fMin = fTemp;
            }
            BWk.PriorityRule(IndexBetterEDD);
            fTemp = BWk.Obj();
            if (fTemp > fMin)
            {
                BWk.ClearSequence();
                BWk.Append(sqlMin);
            }
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_WTE_Setup(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter /w setup
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WTE);
        // MWk.BeamSearch(3,50);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter /w setup
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WTE);
            SWk.PriorityRule(IndexTER);
            SWk.CPI();
            SWk.Local(70);
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter /w setup
            double fMin, fTemp;
            TSequenceList sqlMin;
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WTE);
            BWk.PriorityRuleS(IndexBetterEDD);
            fMin = BWk.Obj();
            sqlMin = BWk.Seq;
            BWk.PriorityRule(IndexFCFS);
            fTemp = BWk.Obj();
            if (fTemp < fMin)
            {
                sqlMin = BWk.Seq;
                fMin = fTemp;
            }
            BWk.PriorityRule(IndexBetterEDD);
            fTemp = BWk.Obj();
            if (fTemp > fMin)
            {
                BWk.ClearSequence();
                BWk.Append(sqlMin);
            }
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_Cmax(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(Cmax);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            if (WkTool.Seq.Num() > 0) sq = *WkTool.Seq.Get(1);
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(Cmax);
            if (sq.Num() > 0)
                SWk.Append(sq);
            else
                SWk.PriorityRule(IndexOpDueDate);
            while (SWk.CPI())
                ;
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(Cmax);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_Cmax_Setup(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter /w setup
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(Cmax);
        MWk.BeamSearch(3, 50);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter /w setup
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(Cmax);
            SWk.PriorityRule(IndexOpDueDate);
            SWk.CPI();
            SWk.Local(70);
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter /w setup
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(Cmax);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_WC(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WC);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            if (WkTool.Seq.Num() > 0) sq = *WkTool.Seq.Get(1);
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WC);
            if (sq.Num() > 0)
                SWk.Append(sq);
            else
                SWk.PriorityRule(IndexOpDueDate);
            while (SWk.CPI())
                ;
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WC);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_WC_Setup(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter /w setup
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WC);
        MWk.BeamSearch(3, 50);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter /w setup
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WC);
            SWk.PriorityRule(IndexOpDueDate);
            SWk.CPI();
            SWk.Local(70);
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter /w setup
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WC);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_WT(TTool& WkTool)
{  // obj = WT
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WT);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            if (WkTool.Seq.Num() > 0) sq = *WkTool.Seq.Get(1);
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WT);
            if (sq.Num() > 0)
                SWk.Append(sq);
            else
                SWk.PriorityRule(IndexTER);
            SWk.CPI();
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WT);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_WLmax(TTool& WkTool)
{  // obj = WT
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WLmax);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            if (WkTool.Seq.Num() > 0) sq = *WkTool.Seq.Get(1);
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WLmax);
            if (sq.Num() > 0)
                SWk.Append(sq);
            else
                SWk.PriorityRule(IndexOpDueDate);
            while (SWk.CPI())
                ;
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WLmax);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_WLmax_Setup(TTool& WkTool)
{  // obj = WT
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WLmax);
        MWk.BeamSearch(3);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            if (WkTool.Seq.Num() > 0) sq = *WkTool.Seq.Get(1);
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WLmax);
            if (sq.Num() > 0)
                SWk.Append(sq);
            else
                SWk.PriorityRule(IndexOpDueDate);
            SWk.CPI();
            SWk.Local(70);
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WLmax);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.CPI();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_ATC(TTool& WkTool)
{
    TSequenceList sqList;
    TMachine* pMC;

    WkTool.fK1 = .1;
    WkTool.fK2 = 1;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        WkTool.PriorityRule(IndexMATC);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            WkTool.fK1 = 0.1;
            WkTool.PriorityRule(IndexMATC);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WT);
            BWk.PriorityRule(IndexMATC);
            BWk.Batching();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_EDD_O(TTool& WkTool)
{
    TSequenceList sqList;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        WkTool.PriorityRule(IndexOpDueDate);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            WkTool.PriorityRule(IndexBetterEDD);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WkTool.ObjFnc);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_TER(TTool& WkTool)
{
    TSequenceList sqList;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.SetObj(WkTool.ObjFnc);
        // MWk.BeamSearch(2);
        MWk.PriorityRule(IndexBetterEDD);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            WkTool.PriorityRule(IndexTER);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WkTool.ObjFnc);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

double SUB_TER_Setup(TTool& WkTool)
{
    TSequenceList sqList;
    TSequence sq;
    TMachine* pMC;

    if (WkTool.NumMachine() > 1)
    {  // Parallel machines workcenter
        WkTool.ClearSequence();
        TMMC_Center MWk = WkTool;
        MWk.PriorityRule(IndexOpDueDate);
        // MWk.Local(70);
        MWk.GetSequence(sqList);
        WkTool.Append(sqList);
    }
    else
    {  // Single machine workcenter
        pMC = WkTool.pWkCenter->Get(1);
        if (pMC->iBatchSize == 1)
        {
            WkTool.ClearSequence();
            T1MC_Center SWk = WkTool;
            SWk.SetObj(WTE);
            SWk.PriorityRule(IndexTER);
            SWk.Local(70);
            SWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
        else
        {  // Batch machine workcenter
            WkTool.ClearSequence();
            TBMC_Center BWk = WkTool;
            BWk.SetObj(WTE);
            BWk.PriorityRule(IndexBetterEDD);
            BWk.Batching();
            BWk.GetSequence(sqList);
            WkTool.Append(sqList);
        }
    }
    return WkTool.Obj();
}

void LocalOpt_4Iter(TShop& Sh,
    TStackP& WkcSchedList,
    double (*OptSeq)(TTool& WkTool),
    double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight))
{
    TWorkcenter* pWk;
    TSequenceList sqOld;
    // TTool wkTool;
    // TDGraph Grh;
    TSequenceList sqL;
    double fBest = Sh.Objective(Obj), fTemp;
    int i;
    for (i = 1; i <= 4; i++)
    {
        if (i >= WkcSchedList.Num()) break;
        pWk = (TWorkcenter*)WkcSchedList[WkcSchedList.Num() - i];
        Sh.GetSequence(pWk, sqOld);
        Sh.RemoveSeq(pWk);
        // assign tlDGraph1
        Sh.GraphOnWkc(pWk, grhDGraph1_buf);
        tlDGraph1_buf.SetData(&Sh, pWk, &grhDGraph1_buf);
        tlDGraph1_buf.SetObj(Obj);
        OptSeq(tlDGraph1_buf);
        fTemp = tlDGraph1_buf.Obj();
        if (fTemp < fBest)
        {
            Sh.Append(tlDGraph1_buf.Seq);
            fBest = fTemp;
        }
        else
            Sh.Append(sqOld);
    }
}

void LocalOpt(TShop& Sh,
    TStackP& WkcSchedList,
    double (*OptSeq)(TTool& WkTool),
    double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight))
{
    TWorkcenter* pWk;
    TSequenceList sqOld;
    // TTool wkTool;
    // TDGraph Grh;
    TSequenceList sqL;
    double fBest = Sh.Objective(Obj), fTemp;
    int i;
    for (i = 1; i < WkcSchedList.Num(); i++)
    {
        pWk = (TWorkcenter*)WkcSchedList[WkcSchedList.Num() - i];
        Sh.GetSequence(pWk, sqOld);
        Sh.RemoveSeq(pWk);
        // assign tlDGraph1
        Sh.GraphOnWkc(pWk, grhDGraph1_buf);
        tlDGraph1_buf.SetData(&Sh, pWk, &grhDGraph1_buf);
        tlDGraph1_buf.SetObj(Obj);
        OptSeq(tlDGraph1_buf);
        fTemp = tlDGraph1_buf.Obj();
        if (fTemp < fBest)
        {
            Sh.Append(tlDGraph1_buf.Seq);
            fBest = fTemp;
        }
        else
            Sh.Append(sqOld);
    }
}

void BYPASS(
    TShop&, TStackP&, double (*OptSeq)(TTool&), double (*Obj)(int, double*, double*, double*, double*))
{}
