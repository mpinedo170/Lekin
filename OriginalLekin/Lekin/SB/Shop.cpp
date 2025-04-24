#include "StdAfx.h"

#include "Shop.h"
#include "Tool.h"

void Trace(LPCSTR szName, TShop& sh)
{
    ofstream myfile(szName);
    myfile << sh;
    myfile.close();
}

ostream& operator<<(ostream& os, TShop& spX)
{
    // cDate dt1,dt2;
    char szTemp[6];  // szTemp1[20],szTemp2[20];
    os << "*** Shop Structure ***" << endl;
    os << spX.WkCenter << endl;
    os << "*** Job List ***" << endl;
    os << spX.Job << endl;
    os << "*** Sequence ***" << endl;
    os << spX.Seq << endl;
    os.precision(6);
    os << "*** Sequence details ***" << endl;
    int i, j, k, iN, iM, iL;
    TOperation* pOperation;
    TMachine* pMachine;
    TWorkcenter* pWorkcenter;
    TSequence* pSequence;
    iN = spX.WkCenter.Num();
    for (i = 1; i <= iN; i++)
    {
        pWorkcenter = spX.WkCenter.Get(i);
        iM = pWorkcenter->Num();
        for (j = 1; j <= iM; j++)
        {
            pMachine = pWorkcenter->Get(j);
            if (pMachine)
            {
                pSequence = spX.Seq.Gets_mID(pMachine->mID);
                if (pSequence)
                {
                    mIDtoa(szTemp, pMachine->mID, 5);
                    os << "Machine " << szTemp << " :" << endl;
                    iL = pSequence->Num();
                    for (k = 1; k <= iL; k++)
                    {
                        pOperation = spX.Job.Geto_oID((*pSequence)[k]);
                        // dt1.Set(pOperation->fStartTime);
                        // dt1.Char(szTemp1);
                        // dt2.Set(pOperation->fFinishTime);
                        // dt2.Char(szTemp2);
                        if (pOperation)
                        {
                            oIDtoa(szTemp, pOperation->oID, 5);
                            os << "\t  setup " << (pOperation->fSetupTime) << endl;
                            // os<<"\t O"<<szTemp<<"\t"<<szTemp1<<" ("<<pOperation->fStartTime<<") -
                            // "<<szTemp2<<" ("<<pOperation->fFinishTime<<")
                            // ["<<(pOperation->fActProcessTime*12*60)<<" min.("<<pOperation->fProcessTime<<")]
                            // "<<endl;
                            os << "\t O" << szTemp << "\t" << pOperation->fStartTime << " ("
                               << pOperation->fStartTime << ") - " << pOperation->fFinishTime << " ("
                               << pOperation->fFinishTime << ")   [" << (pOperation->fActProcessTime)
                               << " min.(" << pOperation->fProcessTime << ")] " << endl;
                        }
                    }
                }
            }
        }
    }
    return os;
}

//**************************** TMcAvail ******************************//
TMcAvail::TMcAvail()
{
    iLastMC = 0;
    /*
      mID = new TMachineID[iMaxMachine];
      pMC = new TMachine*[iMaxMachine];
      iLastOp = new int[iMaxMachine];
      fTime = new double[iMaxMachine];
    */
}

TMcAvail::~TMcAvail()
{
    /*
      delete mID; delete pMC;
      delete iLastOp; delete fTime;
    */
}

void TMcAvail::Set(TMachineID mIDX, TMachine* pMCI, double fTimeX, int iLastOpI)
{
    for (int i = 1; i <= iLastMC; i++)
        if (mIDX == mID[i])
        {
            fTime[i] = fTimeX;
            iLastOp[i] = iLastOpI;
            return;
        }
    mID[++iLastMC] = mIDX;
    pMC[iLastMC] = pMCI;
    fTime[iLastMC] = fTimeX;
    iLastOp[iLastMC] = iLastOpI;
}

double TMcAvail::Get(TMachineID mIDX)
{
    for (int i = 1; i <= iLastMC; i++)
        if (mIDX == mID[i]) return fTime[i];
    return fBigM;
}

int TMcAvail::IsMember(int mIDI)
{
    for (int i = 1; i <= iLastMC; i++)
        if (mID[i] == mIDI) return 1;
    return 0;
}

int TMcAvail::LastOp(TMachineID mIDX)
{
    for (int i = 1; i <= iLastMC; i++)
        if (mIDX == mID[i]) return iLastOp[i];
    return 0;
}

//**************************** TShop *********************************//

TShop::TShop() : DGraph(2), SGraph(2)
{  // add weekly
    TPeriod prd;
    for (int i = 0; i <= 6; i++)
    {
        prd.fStart = i;
        prd.fEnd = i + 1;
        Avail.Week.Append(prd);
    }
}

TShop::TShop(TShop& sh)
{
    int i, iN = sh.WkCenter.Num();
    for (i = 1; i <= iN; i++) Append((sh.WkCenter)[i]);
    iN = sh.Job.Num();
    for (i = 1; i <= iN; i++) Append((sh.Job)[i]);
    Append(sh.Seq);
    Avail = sh.Avail;
}

TShop& TShop::operator=(TShop& sh)
{
    Clear();
    int i, iN = sh.WkCenter.Num();
    for (i = 1; i <= iN; i++) Append(sh.WkCenter[i]);
    iN = sh.Job.Num();
    for (i = 1; i <= iN; i++) Append(sh.Job[i]);
    Append(sh.Seq);
    Avail = sh.Avail;
    return *this;
}
void TShop::UpdateLink(TNode* pNd, double fTime, double fOldTime)
{
    double fTemp;
    if (fTime == fOldTime) return;
    TNode* pNdTo;
    int i, iN = pNd->stkPostArc.Num();
    for (i = 1; i <= iN; i++)
    {
        pNdTo = pNd->GetPostNode(i);
        fTemp = max(pNd->GetPostWeight(i) - fOldTime, 0) + fTime;
        SGraph.RLink(pNd, pNdTo, fTemp);
    }
}

void TShop::UpdateLink(TNodeID nIDx, double fTime, double fOldTime)
{
    TNode* pNode = SGraph.GetID(nIDx);
    UpdateLink(pNode, fTime, fOldTime);
}

int TShop::AddLink(TJob& jbX)
{
    int i, j, iM, iN;
    double fLength, fTemp;
    TOperation *popTemp, *popFrom, *popTo;
    TNode* pNode;
    //***** add nodes to DGraph & SGraph *****//
    iN = jbX.Num();
    jbX.Head();
    for (i = 1; i <= iN; i++)
    {
        popTemp = (TOperation*)jbX.Next();  // Get(i);
        pNode = DGraph.AddNode(popTemp->oID, 0, popTemp->fReleaseTime, fBigM, popTemp->fDueDate);
        pNode->op = popTemp;
        pNode = SGraph.AddNode(popTemp->oID, 0, popTemp->fReleaseTime, fBigM, popTemp->fDueDate);
        pNode->op = popTemp;
        if (popTemp->stkPostOp.Num() == 0)
        {
            DGraph.AddSink(popTemp->oID, popTemp->fProcessTime, jbX.fDueDate);
            SGraph.AddSink(popTemp->oID, popTemp->fProcessTime, jbX.fDueDate);
            SinkJob.Push(&jbX);
        }
        if (popTemp->stkPriorOp.Num() == 0)
        {
            fTemp = max(popTemp->fReleaseTime, jbX.fReleaseTime);
            DGraph.Link(0, popTemp->oID, fTemp);
            SGraph.Link(0, popTemp->oID, fTemp);
        }
    }

    //***** add links to DGraph & SGraph *****//
    jbX.Head();
    for (i = 1; i <= iN; i++)
    {
        popFrom = (TOperation*)jbX.Next();             // Get(i);
        if (popFrom->fReleaseTime > jbX.fReleaseTime)  //***** add release time *****//
        {
            DGraph.Link(0, popFrom->oID, popFrom->fReleaseTime);
            SGraph.Link(0, popFrom->oID, popFrom->fReleaseTime);
        }
        iM = popFrom->stkPostOp.Num();
        popFrom->stkPostOp.Head();
        for (j = 1; j <= iM; j++)
        {
            popTo = (TOperation*)popFrom->stkPostOp.Next();  // GetPost(j);
            fLength = popFrom->fProcessTime;
            DGraph.Link(popFrom->oID, popTo->oID, fLength);
            SGraph.Link(popFrom->oID, popTo->oID, fLength);
        }
    }
    return 1;
}

int TShop::AddLink(TMachineID mIDI, TNodeID nID1, TNodeID nID2)
{
    TMachine* pMachine = WkCenter.Getm_mID(mIDI);
    TNode *pNode1 = SGraph.GetID(nID1), *pNode2 = SGraph.GetID(nID2);
    if ((pNode1 == 0) || (pNode2 == 0)) return 0;
    return AddLink(pMachine, pNode1, pNode2);
}

int TShop::AddLink(TMachine* pMachine, TNode* pNode1, TNode* pNode2)
{
    double fLength;
    TNode* pndTemp;
    int i;
    TStatus sFrom, sTo;
    TOperation *popFrom = pNode1->op, *popTo = pNode2->op;

    if ((pNode1->nID != 0) && (popFrom) && (popTo) && (pMachine))
    {
        sFrom = popFrom->sMC_Status;
        sTo = popTo->sMC_Status;
        popFrom->fActProcessTime = (popFrom->fProcessTime) / (pMachine->fSpeed);
        popFrom->mID = pMachine->mID;
        // operation nID2 will be process on mIDI machine
        popTo->fActProcessTime = (popTo->fProcessTime) / (pMachine->fSpeed);
        popTo->mID = pMachine->mID;
        popTo->fSetupTime = pMachine->suSetup.Get(sFrom, sTo);
        fLength = popFrom->fActProcessTime + popTo->fSetupTime;

        //***** add links in SGraph & update link in SGraph *****//
        // AddLink(pNode1,pNode2,fLength,popFrom->fActProcessTime);

        SGraph.Link(pNode1, pNode2, fLength);
        if (pMachine->fSpeed != 1)
            for (i = 1; i <= pNode2->stkPostArc.Num(); i++)
            {
                pndTemp = pNode2->GetPostNode(i);
                SGraph.RLink(pNode2, pndTemp, popTo->fActProcessTime);
            }
        return 1;
    }
    else
    {
        // the first operation on machine mIDI
        if ((pNode1->nID == 0) && (popTo) && (pMachine))
        {
            sFrom = pMachine->sInit;
            sTo = popTo->sMC_Status;
            popTo->fSetupTime = pMachine->suSetup.Get(sFrom, sTo);
            popTo->fActProcessTime = (popTo->fProcessTime) / (pMachine->fSpeed);
            popTo->mID = pMachine->mID;
            fLength = pMachine->fAvailTime + popTo->fSetupTime;
            SGraph.Link(pNode1, pNode2, fLength);  // max(op release,mc release)
            if (pMachine->fSpeed != 1)
                for (i = 1; i <= pNode2->stkPostArc.Num(); i++)
                {
                    pndTemp = pNode2->GetPostNode(i);
                    SGraph.RLink(pNode2, pndTemp, popTo->fActProcessTime);
                }
            return 1;
        }
    }

    return 0;
}

int TShop::AddLink(TNode* pNd1, TNode* pNd2, double fLength, double fProcessTime)
{  // fLength = fProcessTime + setup
    int iReturn = 1, iN, i;
    TNode* pndTo;
    iN = pNd1->stkPostArc.Num();
    for (i = 1; i <= iN; i++)
    {
        pndTo = pNd1->GetPostNode(i);
        SGraph.RLink(pNd1, pndTo, fProcessTime);
    }
    SGraph.Link(pNd1, pNd2, fLength);
    return iReturn;
}

int TShop::AddLink(TNodeID nID1, TNodeID nID2, double fLength, double fProcessTime)
{
    TNode* pndFrom = SGraph.GetID(nID1);
    TNode* pndTo = SGraph.GetID(nID2);
    return AddLink(pndFrom, pndTo, fLength, fProcessTime);
}

int TShop::AddLink(TSequence& sqX)
{
    int i, iN;
    TStackP BatchList;
    TMachine* pMachine = WkCenter.Getm_mID(sqX.mID);
    // add links in SGraph
    if (!pMachine) return 0;
    if (pMachine->iBatchSize == 1)
    {
        iN = sqX.Num();
        if (iN)
        {
            if (AddLink(sqX.mID, 0, sqX[1]) == 0) return 0;
            for (i = 2; i <= iN; i++)
                if (AddLink(sqX.mID, sqX[i - 1], sqX[i]) == 0) return 0;
            /*
            popTemp = (SGraph.GetID(sqX[iN]))->op;
            popTemp->fActProcessTime = (popTemp->fProcessTime)/(pMachine->fSpeed);
            popTemp->mID = pMachine->mID;
            UpdateLink(popTemp->oID,popTemp->fActProcessTime,popTemp->fProcessTime);
            */
        }
    }
    else
    {
        TStack oIDList;
        int bID, bID_old = 0;

        if (sqX[sqX.Num()] != 0) sqX << 0;

        for (i = 1; i <= sqX.Num(); i++)
        {
            if (sqX[i] != 0)
                oIDList.Push(sqX[i]);
            else
            {
                bID = Batch(oIDList, pMachine);
                BatchLink(bID_old, bID);
                bID_old = bID;
            }
        }
        /*
            cArray_l  bListSize(iNumBatch+1);
            cArray_l  bListID(iNumBatch+1);
            for(i=1;i<=sqX.Num();i++)
            {
              iCBatch++;
              while(sqX[i]!=0)
              {
                oIDList.Push(sqX[i]);
                iTemp = ++(bListSize[iCBatch]);
              }
              bID = Batch(oIDList,pMachine);
              bListID[iCBatch] = bID;
            }

            // link batches
            for(i=1;i<iCBatch;i++)
            {
              LinkBatch(bListID[i],bListID[i+1],pMachine);
            }

            //******* old
            iN=sqX.Num();
            if(iN)
            {
              pndPrevious = SGraph.GetID(0);
              sPrevious   = pMachine->sInit;
              fAvail      = pMachine->fAvailTime;
              for(i=1;i<=iN;i++)
              {
                // store to operations that will be processeed on the same batch in
                // the list
                if(sqX[i]!=0)
                {
                  BatchList.Push(SGraph.GetID(sqX[i]));//Job.Geto_oID(sqX[i]));
                } else
                {
                  // find the longest fSetup & ActProcessTime in the batch
                  fSetup = 0;
                  fProcessTime = 0;
                  iM = BatchList.Num();
                  for(j=1;j<=iM;j++)
                  {
                    popTemp = ((TNode*)BatchList[j])->op;//(TOperation*) BatchList[j];
                    fSetup2 = pMachine->suSetup.Get(sPrevious,popTemp->sMC_Status);
                    if(fSetup2>=fSetup)
                    {
                      fSetup = fSetup2;
                      sPrevious2 = popTemp->sMC_Status;
                    }
                    fProcessTime2 = (popTemp->fProcessTime)/(pMachine->fSpeed);
                    if(fProcessTime2>=fProcessTime)
                fProcessTime = fProcessTime2;
                  }

                  // links
                  pNode = SGraph.AddDummy();
                  SGraph.RLink(pndPrevious,pNode,(fAvail+fSetup));
                  for(j=1;j<=iM;j++)
                  {
                    pNode2  = (TNode*)BatchList[j];
                    popTemp = pNode2->op;//(TOperation*) BatchList[j];
                    //pNode2  = SGraph.GetID(popTemp->oID);
                    popTemp->fActProcessTime = fProcessTime;
                    popTemp->fSetupTime      = fSetup;
                    popTemp->mID             = pMachine->mID;
                    SGraph.RLink(pNode2,pNode,0);
                    iL=pNode2->stkPostArc.Num();//popTemp->stkPostOp.Num();
                    for(k=1;k<=iL;k++)
                    {
                      pNode3 = pNode2->GetPostNode(k);//SGraph.GetID(popTemp->GetPost(k)->oID);
                      SGraph.RLink(pNode,pNode3,fProcessTime); // from dummy node
                    }
                  }
                  // update
                  sPrevious = sPrevious2;
                  pndPrevious = pNode;
                  fAvail      = fProcessTime;
                }
              }

              // end of the sequence
              if(BatchList.Num())
              {
                // find fSetup & ActProcessTime
                fSetup = 0;
                fProcessTime = 0;
                iM = BatchList.Num();
                for(j=1;j<=iM;j++)
                {
                  popTemp = ((TNode*)BatchList[i])->op;//(TOperation*) BatchList[i];
                  fSetup2 = pMachine->suSetup.Get(sPrevious,popTemp->sMC_Status);
                  if(fSetup2>=fSetup)
                  {
                    fSetup = fSetup2;
                    sPrevious2 = popTemp->sMC_Status;
                  }
                  fProcessTime2 = (popTemp->fProcessTime)/(pMachine->fSpeed);
                  if(fProcessTime2>=fProcessTime)
                    fProcessTime = fProcessTime2;
                }
                // links
                pNode = SGraph.AddDummy();
                SGraph.Link(pndPrevious,pNode,(fAvail+fSetup));
                for(j=1;j<=iM;j++)
                {
                  pNode2  = (TNode*)BatchList.Pop();
                  popTemp = pNode2->op;//(TOperation*) BatchList.Pop();
                  //pNode2  = SGraph.GetID(popTemp->oID);
                  popTemp->fActProcessTime = fProcessTime;
                  popTemp->fSetupTime      = fSetup;
                  popTemp->mID             = pMachine->mID;
            SGraph.RLink(pNode2,pNode,0);
                  iL=pNode2->stkPostArc.Num();//popTemp->stkPostOp.Num();
                  for(k=1;k<=iL;k++)
                  {
                    pNode3 = pNode2->GetPostNode(k);//SGraph.GetID(popTemp->GetPost(k)->oID);
                    SGraph.RLink(pNode,pNode3,fProcessTime); // from dummy node
                  }
                }
              }
            } // end if sqX.Num()>0
            */
    }
    return 1;
}

int TShop::DLink(TNode* pNode1, TNode* pNode2, double fProcessTime)
{  // not use
    int iReturn, iN, i;
    TNode* pndTo;
    if (pNode1->op->stkPostOp.Member(pNode2->op))  // if pNode2 follows 1 in the same job
        iReturn = SGraph.RLink(pNode1, pNode2, fProcessTime);
    else
        iReturn = SGraph.DLink(pNode1, pNode2);
    if (iReturn)
    {  // update the processing time
        iN = pNode1->stkPostArc.Num();
        pNode1->stkPostArc.Head();
        for (i = 1; i <= iN; i++)
        {
            // pndTo = pNode1->GetPostNode(i);
            pndTo = ((TArc*)(pNode1->stkPostArc.Next()))->pndTo;
            SGraph.RLink(pNode1, pndTo, fProcessTime);
        }
        pNode1->op->fActProcessTime = 0;
        pNode1->op->mID = 0;
    }
    return iReturn;
}
/*
int TShop::DLink(TNodeID nID)
{
  TNode *pNodeS = SGraph.GetID(nID),
        *pNodeD = DGraph.GetID(nID),
        *pndToD,
        *pndToS;
  if(pNodeS)
  {
    i=1;
    while(i<=pNodeS->stkPostArc.Num())
    {
      pndToS = pNodeS->GetPostNode(i);
      pndToD = DGraph.GetID(pndToS->nID);
      if(pndToD==0)
      {// remove link pNodeS->pndToS
        SGraph.DLink(pNodeS,pndToS);
      } else
      {
        fLength = pNodeD->GetPostWeight(pndToD);
        SGraph.RLink(pNodeS,pndToS,fLength);
        i++
      }
    }
  }
}
*/
int TShop::DLink(TNodeID nID1, TNodeID nID2)
{
    // compare with links from nID1 in DGraph
    TNode *pNode1 = SGraph.GetID(nID1), *pNode2 = SGraph.GetID(nID2), *pNodeD1 = DGraph.GetID(nID1),
          *pNodeD2 = DGraph.GetID(nID2), *pndDTo, *pndSTo;
    TArc* paDTo;
    int i;
    double fLength = pNodeD1->GetPostWeight(pNodeD2);
    if (fLength < fBigM)
        SGraph.RLink(pNode1, pNode2, fLength);
    else
        SGraph.DLink(pNode1, pNode2);
    // reset assignment
    pNode2->op->fActProcessTime = 0;
    pNode2->op->mID = 0;
    // update links follow nID2; remove link if it doesn't exist in DGraph
    for (i = 1; i <= pNodeD2->stkPostArc.Num(); i++)
    {
        paDTo = pNodeD2->GetPostArcPtr(i);
        pndDTo = paDTo->pndTo;
        pndSTo = SGraph.GetID(pndDTo->nID);
        fLength = paDTo->fWeight;
        SGraph.RLink(pNode2, pndSTo, fLength);
    }
    return 1;
}

void TShop::DLink(TSequence& sqX)
{
    int i, j, iN;
    TStack stkBatchList;
    TMachine* pMachine = WkCenter.Getm_mID(sqX.mID);

    // remove links in SGraph
    if (!pMachine) return;
    if (pMachine->iBatchSize == 1)
    {
        iN = sqX.Num();
        if (iN)
        {
            DLink(0, sqX[1]);
            for (i = 2; i <= iN; i++) DLink(sqX[i - 1], sqX[i]);
        }
    }
    else
    {
        // batch size > 1
        int iTemp;
        TNode *pndOp_D, *pndOp_S, *pndTo_D, *pndTo_S, *pndBatch;
        TArc* paTemp;
        iN = sqX.Num();
        if (sqX[iN] != 0) sqX << 0;
        iN = sqX.Num();
        for (i = 1; i <= iN; i++)
        {
            // Find the dummy node
            iTemp = sqX[i];
            if (iTemp > 0)
            {
                // link
                pndOp_S = SGraph.GetID(iTemp);
                pndOp_D = DGraph.GetID(iTemp);
                for (j = 1; j <= pndOp_D->stkPostArc.Num(); j++)
                {
                    paTemp = pndOp_D->GetPostArcPtr(j);
                    pndTo_D = paTemp->pndTo;
                    pndTo_S = SGraph.GetID(pndTo_D->nID);
                    SGraph.RLink(pndOp_S, pndTo_S, paTemp->fWeight);
                }
                pndBatch = pndOp_S->GetPostDummy();
            }
            else
            {  // remove dummy
                BatchInfo.Remove(pndBatch->op);
                SGraph.RemoveDummy(pndBatch);
            }
        }
    }
}

void TShop::UpdateOperation()
{
    int i, iN;
    TNode* pNode;
    SGraph.ReCalc();

    iN = SGraph.Num();
    for (i = 1; i <= iN; i++)
    {
        pNode = SGraph.Get(i);
        if ((pNode->IsOperation()) || (pNode->IsDummy()))
        {
            pNode->op->fStartTime = pNode->EST();
            pNode->op->fDueDate = pNode->DueDate();
            if (pNode->op->fActProcessTime)
                pNode->op->fFinishTime = pNode->op->fStartTime + pNode->op->fActProcessTime;
            else
                pNode->op->fFinishTime = pNode->op->fStartTime + pNode->op->fProcessTime;
        }
    }
}

void TShop::UpdateProcessTime()
{
    int i, j;
    TSequence* pSeq;
    TOperation* pOper;
    TMachine* pMC;
    for (i = 1; i <= Seq.Num(); i++)
    {
        pSeq = Seq.Get(i);
        pMC = WkCenter.Getm_mID(pSeq->mID);
        for (j = 1; j <= pSeq->Num(); j++)
        {
            pOper = DGraph.GetID((*pSeq)[j])->op;
        }
    }
}

/*
void TShop::PriorityRule(TIndexFunc pIndex)
{
  int        i,j;
  TRuleReturn *pSelect;
  TOperation *popSelect,*popTemp,*popTemp2;
  TMachine   *pmcSelect,
             *pmcTemp;
  TWorkcenter wkTemp;
  TJob       jbTemp;
  TNode      *pNode1,
             *pNode2,
             *pNode1N,
             *pNode2N,
             *pndTemp,
             *pndO;
  TMcAvail   MCAvail;
  TSequence  *pSeq;
  int        iReCalcLST;
  double     fMinRelease;
  ClearSequence();
  TDGraph    Graph;
  Graph.Copy(DGraph); // reset EST/LST
  Graph.Sequence(0);
  Graph.ReCalc();
  //Trace("Graph1.grh",Graph);

  pndO = Graph.GetID(0);
  for(i=1;i<=Graph.Num();i++)
  {
    pndTemp = Graph.Get(i);
    if(pndTemp->IsOperation())
    {
       fMinRelease = pndTemp->op->GetMachine(1)->fAvailTime;
       for(j=2;j<=pndTemp->op->stkMachine.Num();j++)
         fMinRelease = min(fMinRelease,pndTemp->op->GetMachine(j)->fAvailTime);
       if(fMinRelease>pndTemp->EST())
         Graph.Link(pndO,pndTemp,fMinRelease);
    }
  }
  Graph.ReCalc();

  // set machine available time MCAvail;
  for(i=1;i<=WkCenter.Num();i++)
  {
    for(j=1;j<=WkCenter[i].Num();j++)
    {
      pmcTemp = WkCenter[i].Get(j);
      MCAvail.Set(pmcTemp->mID,pmcTemp,pmcTemp->fAvailTime,0);
    }
  }
  MCAvail.fData1 = MCAvail.fData2 = 0;

  // determine active operations
  while(Graph.stkActive.Num())
  {
    jbTemp.Clear();
    Graph.stkActive.Head();
    for(i=1;i<=Graph.stkActive.Num();i++)
    {
      pndTemp = (TNode*)Graph.stkActive.Next();
      if(pndTemp->IsOperation())
      {
        popTemp  = (pndTemp)->op;
        popTemp2 = jbTemp.Append(*popTemp);
        popTemp2->fReleaseTime = pndTemp->EST();
        popTemp2->fDueDate     = pndTemp->DueDate();
        popTemp2->fWeight      = popTemp->pJob->fWeight;
        popTemp2->fEWeight     = popTemp->pJob->fEWeight;
        popTemp2->pJob = popTemp->pJob;
      }
    }
    //jbTemp.Append(*(Graph.Active(i)->op));

    pSelect = pIndex(this,MCAvail,jbTemp,Graph,fK1,fK2);
    popSelect = pSelect->op;
    pmcSelect = pSelect->mc;
    iReCalcLST = pSelect->iDataReCalc;
    delete pSelect;

    //*** put it in the sequence list
    pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
    pNode2 = SGraph.GetID(popSelect->oID);
    AddLink(pmcSelect,pNode1,pNode2);
    pNode2->op->fActProcessTime = (pNode2->op->fProcessTime)/(pmcSelect->fSpeed);
    pNode2->op->mID = pmcSelect->mID;
    pNode1N = Graph.GetID(pNode1->nID);
    pNode2N = Graph.GetID(pNode2->nID);
    Graph.RLink(pNode1N,pNode2N,pNode1->GetPostWeight(pNode2));
    Graph.Sequence(pNode1N);
    Graph.Sequence(pNode2N);
    Seq.Append(pmcSelect->mID,popSelect->oID);
    if(iReCalcLST)
      Graph.ReCalc();

    //UpdateOperation();
    //*** update MCAvail & find the next active list
    MCAvail.Set(pmcSelect->mID,pmcSelect,pNode2N->EST()+popSelect->fProcessTime/pmcSelect->fSpeed,popSelect->oID);
  }
  //********** removed
  // updatelink on last operations in the sequence
  Seq.Head();
  for(i=1;i<=Seq.Num();i++)
  {
    pSeq = (TSequence*)Seq.Next();
    if(pSeq->Num()>0)
    {
      pNode2 = SGraph.GetID((*pSeq)[pSeq->Num()]);
      UpdateLink(pNode2->op->oID,pNode2->op->fActProcessTime,pNode2->op->fProcessTime);
    }
  }
  //**********
  ReCalc();
}
*/

/*
void TShop::PriorityRuleB(TIndexFunc pIndex)
{
  int        i,j;
  TRuleReturn *pSelect;
  TOperation *popSelect,*popTemp,*popTemp2;
  TMachine   *pmcSelect,
             *pmcTemp;
  TWorkcenter wkTemp;
  TJob       jbTemp,
             jbTemp2,
             jbTemp3;
  TNode      *pNode1,
             *pNode2,
             *pNode1N,
             *pNode2N,
             *pndTemp,
             *pndO;
  TMcAvail   MCAvail;
  TSequence  *pSeq;
  int        iReCalcLST,
             iLastDummy=0,
             iDummy;
  double     fMinRelease;
//  int        iLastMC = 0;
  ClearSequence();
  TDGraph    Graph;
  Graph.Copy(DGraph); // reset EST/LST
  Graph.Sequence(0);
  Graph.ReCalc();
  //Trace("Graph1.grh",Graph);

  pndO = Graph.GetID(0);
  for(i=1;i<=Graph.Num();i++)
  {
    pndTemp = Graph.Get(i);
    if(pndTemp->IsOperation())
    {
       fMinRelease = pndTemp->op->GetMachine(1)->fAvailTime;
       for(j=2;j<=pndTemp->op->stkMachine.Num();j++)
         fMinRelease = min(fMinRelease,pndTemp->op->GetMachine(j)->fAvailTime);
       if(fMinRelease>pndTemp->EST())
         Graph.Link(pndO,pndTemp,fMinRelease);
    }
  }
  Graph.ReCalc();

  // set machine available time MCAvail;
  for(i=1;i<=WkCenter.Num();i++)
  {
    for(j=1;j<=WkCenter[i].Num();j++)
    {
      pmcTemp = WkCenter[i].Get(j);
      MCAvail.Set(pmcTemp->mID,pmcTemp,pmcTemp->fAvailTime,0);
    }
  }
  MCAvail.fData1 = MCAvail.fData2 = 0;

  // determine active operations
  while(Graph.stkActive.Num())
  {
    jbTemp.Clear();
    Graph.stkActive.Head();
    for(i=1;i<=Graph.stkActive.Num();i++)
    {
      pndTemp = (TNode*)Graph.stkActive.Next();
      if(pndTemp->IsOperation())
      {
        popTemp  = (pndTemp)->op;
        popTemp2 = jbTemp.Append(*popTemp);
        popTemp2->fReleaseTime = pndTemp->EST();
        popTemp2->fDueDate     = pndTemp->DueDate();
        popTemp2->fWeight      = popTemp->pJob->fWeight;
        popTemp2->fEWeight     = popTemp->pJob->fEWeight;
        popTemp2->pJob = popTemp->pJob;
      }
    }
    //jbTemp.Append(*(Graph.Active(i)->op));
    jbTemp2 = jbTemp;
    pSelect = pIndex(this,MCAvail,jbTemp,Graph,fK1,fK2);
    popSelect = pSelect->op;
    pmcSelect = pSelect->mc;
    iReCalcLST = pSelect->iDataReCalc;
    delete pSelect;

    // find other operation in the batch
    TStack stkBatch;
    if(pmcSelect->iBatchSize>1)
    {
      TStackP stkSort;
      double   fLastT;

      // check the rest of jbTemp for op with same sMC_Status
      jbTemp2.Head();
      for(i=1;i<=jbTemp2.Num();i++)
      {
        popTemp = (TOperation*)jbTemp2.Next();
        if((popTemp->stkMachine.Member(pmcSelect))&&
           (popTemp->sMC_Status == popSelect->sMC_Status)
          jbTemp3.Append(*popSelect);
      }

      // batch it!
      stkBatch.Push(popSelect->oID);

      while((stkBatch.Num()<pmcSelect->iBatchSize)&&
            (jbTemp3.Num()>0))
      {
        jbTemp = jbTemp3;
        pSelect = pIndex(this,MCAvail,jbTemp,Graph,fK1,fK2);
        if(pSelect->mc==pmcSelect)
        { // add pSelect->op to stkBatch
          stkBatch.Push(pSelect->op->oID);
        }
        jbTemp3.RemoveID(pSelect->op->oID);
        delete pSelect;
      }
      stkBatch2 = stkBatch;
      iDummy = Batch(stkBatch2,pmcSelect);

      // put it in the sequence list
      pNode1 = SGraph.GetID(iLastDummy);
      pNode2 = SGraph.GetID(iDummy);
      LinkBatch(iLastDummy,iDummy,pmcSelect);
      BSetActProcessTime(pNode2,BatchProcessTime(pNode2)/pmcSelect->fSpeed,pmcSelect->mID);
      pNode1N = Graph.GetID(pNode1->nID);
      pNode2N = Graph.AddNode(pNode2->nID);
      fTemp   = pNode1->GetPostWeight(pNode2);
      Graph.Link(pNode1N,pNode2N,fTemp);
      Graph.Remove(pNode1N);

      for(i=1;i<=pNode2->stkPriorArc.Num();i++)
      {
        pnd2From  = pNode2->GetPriorNode(i);
        pnd2FromN = Graph.GetID(pnd2From->nID);
        fLength   = pNode2->GetPriorWeight(i);
        fTemp     = pnd2FromN->fEST+fLength;
        pNode2N->fEST = max(pNode2N->fEST,fTemp);
        Graph.Remove(pnd2FromN);
      }
      for(i=1;i<=pNode2->stkPostArc.Num();i++)
      {
        pnd2To  = pNode2->GetPostNode(i);
        pnd2ToN = Graph.GetID(pnd2To->nID);
        fLength = pNode2->GetPostWeight(i);
        fTemp   = pNode2N->fEST+fLength;
        pNode2ToN->fEST = max(pNode2ToN->fEST,fTemp);
      }
      Graph.Remove(pNode2N);

      //sequence prior nodes -> post nodes
      for(i=1;i<=stkBatch.Num();i++)
      {
        iTemp = stkBatch.PopFirst();
        Seq.Append(pmcSelect->mID,iTemp);
      }
      Seq.Append(pmcSelect->mID,0);
      if(iReCalcLST)
        Graph.ReCalc();
      iLastDummy = iDummy;

      //UpdateOperation();
      //*** update MCAvail & find the next active list
      MCAvail.Set(pmcSelect->mID,pmcSelect,pNode2N->EST()+pNode2N->op->fProcessTime/pmcSelect->fSpeed,popSelect->oID);

    } else
    {
      //*** put it in the sequence list
      pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
      pNode2 = SGraph.GetID(popSelect->oID);
      AddLink(pmcSelect,pNode1,pNode2);
      pNode2->op->fActProcessTime = (pNode2->op->fProcessTime)/(pmcSelect->fSpeed);
      pNode2->op->mID = pmcSelect->mID;
      pNode1N = Graph.GetID(pNode1->nID);
      pNode2N = Graph.GetID(pNode2->nID);
      Graph.RLink(pNode1N,pNode2N,pNode1->GetPostWeight(pNode2));
      Graph.Sequence(pNode1N);
      Graph.Sequence(pNode2N);
      Seq.Append(pmcSelect->mID,popSelect->oID);
      if(iReCalcLST)
        Graph.ReCalc();

      //UpdateOperation();
      //*** update MCAvail & find the next active list
      MCAvail.Set(pmcSelect->mID,pmcSelect,pNode2N->EST()+popSelect->fProcessTime/pmcSelect->fSpeed,popSelect->oID);
    }
  }
  // updatelink on last operations in the sequence
  Seq.Head();
  for(i=1;i<=Seq.Num();i++)
  {
    pSeq = (TSequence*)Seq.Next();
    if(pSeq->Num()>0)
    {
      pNode2 = SGraph.GetID((*pSeq)[pSeq->Num()]);
      UpdateLink(pNode2->op->oID,pNode2->op->fActProcessTime,pNode2->op->fProcessTime);
    }
  }
  ReCalc();
}
*/

void TShop::PriorityRule(TIndexFunc pIndex)
{
    int i, j, iTemp;
    TRuleReturn* pSelect;
    TOperation *popSelect, *popTemp, *popTemp2;
    TMachine *pmcSelect, *pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp, jbTemp2, jbTemp3;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp, *pndO;
    TMcAvail MCAvail;
    TSequence* pSeq;
    int iReCalcLST, iLastDummy = 0;
    double fMinRelease, fTemp, fOpMinRelease, fMinMC;

    TStack stkBatch;
    TNode *pndDummy, *pndDummyG, *pndPreDummy, *pndPreDummyG, *pndFrom, *pndFromS, *pndFromG, *pndTo, *pndTo2,
        *pndMinRelease;
    TArc* paTemp;
    TDGraph gTemp;

    //  int        iLastMC = 0;
    ClearSequence();
    TDGraph Graph;
    Graph.Copy(DGraph);  // reset EST/LST
    Graph.Sequence(0);
    Graph.ReCalc();
    // Trace("Graph1.grh",Graph);

    pndO = Graph.GetID(0);
    for (i = 1; i <= Graph.Num(); i++)
    {
        pndTemp = Graph.Get(i);
        if (pndTemp->IsOperation())
        {
            fMinRelease = pndTemp->op->GetMachine(1)->fAvailTime;
            for (j = 2; j <= pndTemp->op->stkMachine.Num(); j++)
                fMinRelease = min(fMinRelease, pndTemp->op->GetMachine(j)->fAvailTime);
            if (fMinRelease > pndTemp->EST()) Graph.Link(pndO, pndTemp, fMinRelease);
        }
    }
    Graph.ReCalc();

    // set machine available time MCAvail;
    for (i = 1; i <= WkCenter.Num(); i++)
    {
        for (j = 1; j <= WkCenter[i].Num(); j++)
        {
            pmcTemp = WkCenter[i].Get(j);
            MCAvail.Set(pmcTemp->mID, pmcTemp, pmcTemp->fAvailTime, 0);
        }
    }
    MCAvail.fData1 = MCAvail.fData2 = 0;

    // determine active operations
    while (Graph.stkActive.Num())
    {
        jbTemp.Clear();
        Graph.stkActive.Head();
        fOpMinRelease = fBigM;
        pndMinRelease = 0;
        for (i = 1; i <= Graph.stkActive.Num(); i++)
        {
            pndTemp = (TNode*)Graph.stkActive.Next();
            if (pndTemp->IsOperation())
            {
                popTemp = (pndTemp)->op;

                // determine min release time op
                if (pndTemp->EST() < fOpMinRelease)
                {
                    pndMinRelease = pndTemp;
                    fOpMinRelease = pndTemp->EST();
                }
                // find MC release time
                fMinMC = fBigM;
                for (j = 1; j <= popTemp->stkMachine.Num(); j++)
                {
                    pmcTemp = popTemp->GetMachine(j);
                    if (MCAvail.Get(pmcTemp->mID) < fMinMC) fMinMC = MCAvail.Get(pmcTemp->mID);
                }
                // add popTemp to jbTemp
                if (pndTemp->EST() <= fMinMC)
                {
                    popTemp2 = jbTemp.Append(*popTemp);
                    popTemp2->fReleaseTime = pndTemp->EST();
                    popTemp2->fDueDate = pndTemp->DueDate();
                    popTemp2->fWeight = popTemp->pJob->fWeight;
                    popTemp2->fEWeight = popTemp->pJob->fEWeight;
                    popTemp2->pJob = popTemp->pJob;
                }
            }
        }
        if (jbTemp.Num() == 0)
        {
            popTemp = pndMinRelease->op;
            popTemp2 = jbTemp.Append(*popTemp);
            popTemp2->fReleaseTime = pndMinRelease->EST();
            popTemp2->fDueDate = pndMinRelease->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
        }
        // jbTemp.Append(*(Graph.Active(i)->op));
        jbTemp2 = jbTemp;
        pSelect = pIndex(this, MCAvail, jbTemp, Graph, fK1, fK2);
        popSelect = pSelect->op;
        pmcSelect = pSelect->mc;
        iReCalcLST = pSelect->iDataReCalc;
        delete pSelect;

        // find if the operation can be put in the previous batch
        if (pmcSelect->iBatchSize > 1)
        {
            // find last batch id
            iTemp = MCAvail.LastOp(pmcSelect->mID);
            if (iTemp > 0)
            {
                pndTemp = SGraph.GetID(iTemp);
                for (i = 1; i <= pndTemp->stkPostArc.Num(); i++)
                {
                    pndDummy = pndTemp->GetPostNode(i);
                    if (pndDummy->IsDummy()) break;
                }
                if ((pndDummy->stkPriorArc.Num() <= pmcSelect->iBatchSize) &&
                    (pndDummy->op->sMC_Status == popSelect->sMC_Status))
                {  // Last batch can hold this operation
                    // check feasibility
                    gTemp = SGraph;
                    pndFrom = gTemp.GetID(popSelect->oID);
                    pndFromS = SGraph.GetID(popSelect->oID);
                    pndTo = gTemp.GetID(pndDummy->nID);
                    fTemp = max(pndDummy->op->fProcessTime, popSelect->fProcessTime) / pmcSelect->fSpeed;
                    for (j = 1; j <= pndFromS->stkPostArc.Num(); j++)
                    {
                        paTemp = pndFromS->GetPostArcPtr(j);
                        iTemp = paTemp->pndTo->nID;
                        pndTo2 = gTemp.GetID(iTemp);
                        gTemp.Link(pndTo, pndTo2, fTemp);
                        gTemp.DLink(pndFrom, pndTo2);
                    }
                    gTemp.Link(pndFrom, pndTo, 0);
                    iTemp = gTemp.ReCalcEST();
                    if (iTemp)  // feasible
                    {
                        // add this op to last batch
                        BatchAdd(pndDummy->nID, popSelect->oID);

                        pndDummyG = Graph.GetID(pndTo->nID);
                        pndFromG = Graph.GetID(popSelect->oID);
                        for (j = 1; j <= pndTo->stkPostArc.Num(); j++)
                        {
                            paTemp = pndTo->GetPostArcPtr(j);
                            iTemp = paTemp->pndTo->nID;
                            fTemp = paTemp->fWeight;
                            pndTo2 = Graph.GetID(iTemp);
                            Graph.Link(pndDummyG, pndTo2, fTemp);
                            Graph.DLink(pndFromG, pndTo2);
                        }
                        Graph.Link(pndFromG, pndDummyG, 0);
                        Graph.Sequence(pndFromG);
                        Graph.Sequence(pndDummyG);

                        pSeq = Seq.Gets_mID(pmcSelect->mID);
                        pSeq->Pop();
                        pSeq->Append(popSelect->oID);
                        pSeq->Append(0);
                    }
                    else
                    {
                        // add this op to new batch
                        pndPreDummy = pndDummy;
                        stkBatch.Push(popSelect->oID);
                        iTemp = Batch(stkBatch, pmcSelect);
                        BatchLink(pndPreDummy->nID, iTemp);

                        pndPreDummyG = Graph.GetID(pndPreDummy->nID);
                        pndFromG = Graph.GetID(popSelect->oID);
                        pndDummyG = Graph.AddNode(iTemp);
                        pndDummy = SGraph.GetID(iTemp);
                        for (j = 1; j <= pndDummy->stkPostArc.Num(); j++)
                        {
                            paTemp = pndDummy->GetPostArcPtr(j);
                            iTemp = paTemp->pndTo->nID;
                            fTemp = paTemp->fWeight;
                            pndTo2 = Graph.GetID(iTemp);
                            Graph.Link(pndDummyG, pndTo2, fTemp);
                            Graph.DLink(pndFromG, pndTo2);
                        }
                        Graph.Link(pndFromG, pndDummyG, 0);
                        Graph.Link(pndPreDummyG, pndDummyG, pndPreDummy->GetPostWeight(pndDummy));
                        Graph.Sequence(pndPreDummyG);
                        Graph.Sequence(pndFromG);
                        Graph.Sequence(pndDummyG);

                        Seq.Append(pmcSelect->mID, popSelect->oID);
                        Seq.Append(pmcSelect->mID, 0);
                    }
                }
                else
                {
                    // add this op to new batch
                    pndPreDummy = pndDummy;
                    stkBatch.Push(popSelect->oID);
                    iTemp = Batch(stkBatch, pmcSelect);
                    BatchLink(pndPreDummy->nID, iTemp);

                    pndPreDummyG = Graph.GetID(pndPreDummy->nID);
                    pndFromG = Graph.GetID(popSelect->oID);
                    pndDummyG = Graph.AddNode(iTemp);
                    pndDummy = SGraph.GetID(iTemp);
                    for (j = 1; j <= pndDummy->stkPostArc.Num(); j++)
                    {
                        paTemp = pndDummy->GetPostArcPtr(j);
                        iTemp = paTemp->pndTo->nID;
                        fTemp = paTemp->fWeight;
                        pndTo2 = Graph.GetID(iTemp);
                        Graph.Link(pndDummyG, pndTo2, fTemp);
                        Graph.DLink(pndFromG, pndTo2);
                    }
                    Graph.Link(pndFromG, pndDummyG, 0);
                    Graph.Link(pndPreDummyG, pndDummyG, pndPreDummy->GetPostWeight(pndDummy));
                    Graph.Sequence(pndPreDummyG);
                    Graph.Sequence(pndFromG);
                    Graph.Sequence(pndDummyG);

                    Seq.Append(pmcSelect->mID, popSelect->oID);
                    Seq.Append(pmcSelect->mID, 0);
                }
            }
            else
            {
                // add this op to new batch (first batch)
                pndPreDummy = SGraph.GetID(0);
                stkBatch.Push(popSelect->oID);
                iTemp = Batch(stkBatch, pmcSelect);
                BatchLink(0, iTemp);

                pndPreDummyG = Graph.GetID(0);
                pndFromG = Graph.GetID(popSelect->oID);
                pndDummyG = Graph.AddNode(iTemp);
                pndDummy = SGraph.GetID(iTemp);
                for (j = 1; j <= pndDummy->stkPostArc.Num(); j++)
                {
                    paTemp = pndDummy->GetPostArcPtr(j);
                    iTemp = paTemp->pndTo->nID;
                    fTemp = paTemp->fWeight;
                    pndTo2 = Graph.GetID(iTemp);
                    Graph.Link(pndDummyG, pndTo2, fTemp);
                    Graph.DLink(pndFromG, pndTo2);
                }
                Graph.Link(pndFromG, pndDummyG, 0);
                Graph.Link(pndPreDummyG, pndDummyG, pndPreDummy->GetPostWeight(pndDummy));
                Graph.Sequence(pndPreDummyG);
                Graph.Sequence(pndFromG);
                Graph.Sequence(pndDummyG);

                Seq.Append(pmcSelect->mID, popSelect->oID);
                Seq.Append(pmcSelect->mID, 0);
            }
            if (iReCalcLST) Graph.ReCalc();

            MCAvail.Set(pmcSelect->mID, pmcSelect,
                pndDummyG->EST() + pndDummy->op->fProcessTime / pmcSelect->fSpeed, popSelect->oID);
        }
        else
        {
            //*** put it in the sequence list
            pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
            pNode2 = SGraph.GetID(popSelect->oID);
            AddLink(pmcSelect, pNode1, pNode2);
            pNode2->op->fActProcessTime = (pNode2->op->fProcessTime) / (pmcSelect->fSpeed);
            pNode2->op->mID = pmcSelect->mID;
            pNode1N = Graph.GetID(pNode1->nID);
            pNode2N = Graph.GetID(pNode2->nID);
            Graph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
            Graph.Sequence(pNode1N);
            Graph.Sequence(pNode2N);
            Seq.Append(pmcSelect->mID, popSelect->oID);
            if (iReCalcLST) Graph.ReCalc();

            // UpdateOperation();
            //*** update MCAvail & find the next active list
            MCAvail.Set(pmcSelect->mID, pmcSelect,
                pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed, popSelect->oID);
        }
    }

    ReCalc();
}

void TShop::PriorityRule2(TIndexFunc pIndex)
{
    int i, j, iTemp;
    TRuleReturn* pSelect;
    TOperation *popSelect, *popTemp, *popTemp2, *popTemp3;
    TMachine *pmcSelect, *pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp, jbTemp2, jbTemp3;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp, *pndO;
    TMcAvail MCAvail;
    TSequence* pSeq;
    int iReCalcLST, iLastDummy = 0;
    double fMinRelease, fTemp;

    TStack stkBatch;
    TNode *pndDummy, *pndDummyG, *pndPreDummy, *pndPreDummyG, *pndFrom, *pndFromS, *pndFromG, *pndTo, *pndTo2;
    TArc* paTemp;
    TDGraph gTemp;

    //  int        iLastMC = 0;
    ClearSequence();
    TDGraph Graph;
    Graph.Copy(DGraph);  // reset EST/LST
    Graph.Sequence(0);
    Graph.ReCalc();
    // Trace("Graph1.grh",Graph);

    pndO = Graph.GetID(0);
    for (i = 1; i <= Graph.Num(); i++)
    {
        pndTemp = Graph.Get(i);
        if (pndTemp->IsOperation())
        {
            fMinRelease = pndTemp->op->GetMachine(1)->fAvailTime;
            for (j = 2; j <= pndTemp->op->stkMachine.Num(); j++)
                fMinRelease = min(fMinRelease, pndTemp->op->GetMachine(j)->fAvailTime);
            if (fMinRelease > pndTemp->EST()) Graph.Link(pndO, pndTemp, fMinRelease);
        }
    }
    Graph.ReCalc();

    // set machine available time MCAvail;
    for (i = 1; i <= WkCenter.Num(); i++)
    {
        for (j = 1; j <= WkCenter[i].Num(); j++)
        {
            pmcTemp = WkCenter[i].Get(j);
            MCAvail.Set(pmcTemp->mID, pmcTemp, pmcTemp->fAvailTime, 0);
        }
    }
    MCAvail.fData1 = MCAvail.fData2 = 0;

    // determine active operations
    while (Graph.stkActive.Num())
    {
        jbTemp.Clear();
        jbTemp2.Clear();
        Graph.stkActive.Head();
        for (i = 1; i <= Graph.stkActive.Num(); i++)
        {
            pndTemp = (TNode*)Graph.stkActive.Next();
            if (pndTemp->IsOperation())
            {
                popTemp = (pndTemp)->op;
                popTemp2 = jbTemp.Append(*popTemp);
                popTemp3 = jbTemp2.Append(*popTemp);
                popTemp2->fReleaseTime = pndTemp->EST();
                popTemp2->fDueDate = pndTemp->DueDate();
                popTemp2->fWeight = popTemp->pJob->fWeight;
                popTemp2->fEWeight = popTemp->pJob->fEWeight;
                popTemp2->pJob = popTemp->pJob;
                popTemp3->fReleaseTime = pndTemp->EST();
                popTemp3->fDueDate = pndTemp->DueDate();
                popTemp3->fWeight = popTemp->pJob->fWeight;
                popTemp3->fEWeight = popTemp->pJob->fEWeight;
                popTemp3->pJob = popTemp->pJob;
            }
        }
        pSelect = pIndex(this, MCAvail, jbTemp, Graph, fK1, fK2);
        popSelect = pSelect->op;
        pmcSelect = pSelect->mc;
        iReCalcLST = pSelect->iDataReCalc;
        delete pSelect;

        // find if the operation can be put in the previous batch
        if (pmcSelect->iBatchSize > 1)
        {
            // find last batch id
            iTemp = MCAvail.LastOp(pmcSelect->mID);
            if (iTemp > 0)
            {
                pndTemp = SGraph.GetID(iTemp);
                for (i = 1; i <= pndTemp->stkPostArc.Num(); i++)
                {
                    pndDummy = pndTemp->GetPostNode(i);
                    if (pndDummy->IsDummy()) break;
                }
                if ((pndDummy->stkPriorArc.Num() <= pmcSelect->iBatchSize) &&
                    (pndDummy->op->sMC_Status == popSelect->sMC_Status))
                {  // Last batch can hold this operation
                    // check feasibility
                    gTemp = SGraph;
                    pndFrom = gTemp.GetID(popSelect->oID);
                    pndFromS = SGraph.GetID(popSelect->oID);
                    pndTo = gTemp.GetID(pndDummy->nID);
                    fTemp = max(pndDummy->op->fProcessTime, popSelect->fProcessTime) / pmcSelect->fSpeed;
                    for (j = 1; j <= pndFromS->stkPostArc.Num(); j++)
                    {
                        paTemp = pndFromS->GetPostArcPtr(j);
                        iTemp = paTemp->pndTo->nID;
                        pndTo2 = gTemp.GetID(iTemp);
                        gTemp.Link(pndTo, pndTo2, fTemp);
                        gTemp.DLink(pndFrom, pndTo2);
                    }
                    gTemp.Link(pndFrom, pndTo, 0);
                    iTemp = gTemp.ReCalcEST();
                    if (iTemp)  // feasible
                    {
                        // add this op to last batch
                        BatchAdd(pndDummy->nID, popSelect->oID);

                        pndDummyG = Graph.GetID(pndTo->nID);
                        pndFromG = Graph.GetID(popSelect->oID);
                        for (j = 1; j <= pndTo->stkPostArc.Num(); j++)
                        {
                            paTemp = pndTo->GetPostArcPtr(j);
                            iTemp = paTemp->pndTo->nID;
                            fTemp = paTemp->fWeight;
                            pndTo2 = Graph.GetID(iTemp);
                            Graph.Link(pndDummyG, pndTo2, fTemp);
                            Graph.DLink(pndFromG, pndTo2);
                        }
                        Graph.Link(pndFromG, pndDummyG, 0);
                        Graph.Sequence(pndFromG);
                        Graph.Sequence(pndDummyG);

                        pSeq = Seq.Gets_mID(pmcSelect->mID);
                        pSeq->Pop();
                        pSeq->Append(popSelect->oID);
                        pSeq->Append(0);
                    }
                    else
                    {
                        // add this op to new batch (precedent op is in the previous batch)
                        pndPreDummy = pndDummy;
                        stkBatch.Clear();
                        stkBatch.Push(popSelect->oID);
                        iTemp = Batch(stkBatch, pmcSelect);
                        BatchLink(pndPreDummy->nID, iTemp);

                        pndPreDummyG = Graph.GetID(pndPreDummy->nID);
                        pndFromG = Graph.GetID(popSelect->oID);
                        pndDummyG = Graph.AddNode(iTemp);
                        pndDummy = SGraph.GetID(iTemp);
                        for (j = 1; j <= pndDummy->stkPostArc.Num(); j++)
                        {
                            paTemp = pndDummy->GetPostArcPtr(j);
                            iTemp = paTemp->pndTo->nID;
                            fTemp = paTemp->fWeight;
                            pndTo2 = Graph.GetID(iTemp);
                            Graph.Link(pndDummyG, pndTo2, fTemp);
                            Graph.DLink(pndFromG, pndTo2);
                        }
                        Graph.Link(pndFromG, pndDummyG, 0);
                        Graph.Link(pndPreDummyG, pndDummyG, pndPreDummy->GetPostWeight(pndDummy));
                        Graph.Sequence(pndPreDummyG);
                        Graph.Sequence(pndFromG);
                        Graph.Sequence(pndDummyG);

                        Seq.Append(pmcSelect->mID, popSelect->oID);
                        Seq.Append(pmcSelect->mID, 0);
                    }
                }
                else
                {
                    // add this op to new batch (status is not the same as last batch)
                    pndPreDummy = pndDummy;
                    stkBatch.Clear();
                    stkBatch.Push(popSelect->oID);
                    iTemp = Batch(stkBatch, pmcSelect);
                    BatchLink(pndPreDummy->nID, iTemp);

                    pndPreDummyG = Graph.GetID(pndPreDummy->nID);
                    pndFromG = Graph.GetID(popSelect->oID);
                    pndDummyG = Graph.AddNode(iTemp);
                    pndDummy = SGraph.GetID(iTemp);
                    for (j = 1; j <= pndDummy->stkPostArc.Num(); j++)
                    {
                        paTemp = pndDummy->GetPostArcPtr(j);
                        iTemp = paTemp->pndTo->nID;
                        fTemp = paTemp->fWeight;
                        pndTo2 = Graph.GetID(iTemp);
                        Graph.Link(pndDummyG, pndTo2, fTemp);
                        Graph.DLink(pndFromG, pndTo2);
                    }
                    Graph.Link(pndFromG, pndDummyG, 0);
                    Graph.Link(pndPreDummyG, pndDummyG, pndPreDummy->GetPostWeight(pndDummy));
                    Graph.Sequence(pndPreDummyG);
                    Graph.Sequence(pndFromG);
                    Graph.Sequence(pndDummyG);

                    Seq.Append(pmcSelect->mID, popSelect->oID);
                    Seq.Append(pmcSelect->mID, 0);
                }
            }
            else
            {
                // add this op to new batch (first batch)
                pndPreDummy = SGraph.GetID(0);
                stkBatch.Clear();
                stkBatch.Push(popSelect->oID);
                iTemp = Batch(stkBatch, pmcSelect);
                BatchLink(0, iTemp);

                pndPreDummyG = Graph.GetID(0);
                pndFromG = Graph.GetID(popSelect->oID);
                pndDummyG = Graph.AddNode(iTemp);
                pndDummy = SGraph.GetID(iTemp);
                for (j = 1; j <= pndDummy->stkPostArc.Num(); j++)
                {
                    paTemp = pndDummy->GetPostArcPtr(j);
                    iTemp = paTemp->pndTo->nID;
                    fTemp = paTemp->fWeight;
                    pndTo2 = Graph.GetID(iTemp);
                    Graph.Link(pndDummyG, pndTo2, fTemp);
                    Graph.DLink(pndFromG, pndTo2);
                }
                Graph.Link(pndFromG, pndDummyG, 0);
                Graph.Link(pndPreDummyG, pndDummyG, pndPreDummy->GetPostWeight(pndDummy));
                Graph.Sequence(pndPreDummyG);
                Graph.Sequence(pndFromG);
                Graph.Sequence(pndDummyG);

                Seq.Append(pmcSelect->mID, popSelect->oID);
                Seq.Append(pmcSelect->mID, 0);
            }
            if (iReCalcLST) Graph.ReCalc();

            MCAvail.Set(pmcSelect->mID, pmcSelect,
                pndDummyG->EST() + pndDummy->op->fProcessTime / pmcSelect->fSpeed, popSelect->oID);
        }
        else
        {  // Batch size == 1
            //*** put it in the sequence list
            pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
            pNode2 = SGraph.GetID(popSelect->oID);
            AddLink(pmcSelect, pNode1, pNode2);
            pNode2->op->fActProcessTime = (pNode2->op->fProcessTime) / (pmcSelect->fSpeed);
            pNode2->op->mID = pmcSelect->mID;
            pNode1N = Graph.GetID(pNode1->nID);
            pNode2N = Graph.GetID(pNode2->nID);
            Graph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
            Graph.Sequence(pNode1N);
            Graph.Sequence(pNode2N);
            Seq.Append(pmcSelect->mID, popSelect->oID);
            if (iReCalcLST) Graph.ReCalc();

            // UpdateOperation();
            //*** update MCAvail & find the next active list
            MCAvail.Set(pmcSelect->mID, pmcSelect,
                pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed, popSelect->oID);
        }
    }

    ReCalc();
}

void TShop::Clear()
{
    DGraph.Clear();
    SGraph.Clear();
    SinkJob.Clear();
    Job.Clear();
    WkCenter.Clear();
    Seq.Clear();
    Avail.Clear();
    BatchInfo.Clear();
}

void TShop::ClearJob()
{
    WkCenter.ClearTask();
    DGraph.Clear();
    SGraph.Clear();
    SinkJob.Clear();
    Job.Clear();
    Seq.Clear();
    BatchInfo.Clear();
}

void TShop::ClearSequence()
{
    int j, iM;
    TJob* pjbTemp;
    TOperation* pOp;
    SGraph = DGraph;
    Seq.Clear();
    BatchInfo.Clear();
    int iN = Job.Num();
    Job.Head();
    for (int i = 1; i <= iN; i++)
    {
        pjbTemp = (TJob*)Job.Next();  // Get(i);
        iM = pjbTemp->Num();
        pjbTemp->Head();
        for (j = 1; j <= iM; j++)
        {
            pOp = (TOperation*)pjbTemp->Next();  // Get(j);
            pOp->fActProcessTime = 0;
            pOp->fSetupTime = 0;
        }
    }
}

TJob* TShop::Append(TJob& jbX)
{
    TJob* pjbTemp = Job.Append(jbX);
    //*** add the tasks to machines
    TOperation* popTemp;
    TMachine* pmcTemp;
    TWorkcenter* pwkTemp;
    int i, j, iM, iN = pjbTemp->Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = pjbTemp->Get(i);
        iM = popTemp->stkMachine.Num();
        for (j = 1; j <= iM; j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            pmcTemp->AddTask(popTemp);
            pwkTemp = WkCenter.Getw_mID(pmcTemp->mID);
            pwkTemp->AddTask(popTemp);
        }
    }

    AddLink(*pjbTemp);
    //***** add sink?
    return pjbTemp;
}

TJob* TShop::AppendAutoID(TJob& jbX)
{
    // find to last job and operation id
    int iLastJobID = 0, iLastOperationID = 0, i, j;
    TJob *pJob, jbNew;
    TOperation* pOperation;
    for (i = 1; i <= Job.Num(); i++)
    {
        pJob = Job.Get(i);
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOperation = pJob->Get(j);
            iLastOperationID = max(iLastOperationID, pOperation->oID);
        }
        iLastJobID = max(iLastJobID, pJob->jID);
    }

    jbNew = jbX;
    jbNew.jID = ++iLastJobID;
    for (i = 1; i <= jbNew.Num(); i++)
    {
        pOperation = jbNew.Get(i);
        pOperation->oID = ++iLastOperationID;
    }

    TJob* pjbTemp = Job.Append(jbNew);
    //*** add the tasks to machines
    TOperation* popTemp;
    TMachine* pmcTemp;
    TWorkcenter* pwkTemp;
    int iM, iN = pjbTemp->Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp = pjbTemp->Get(i);
        iM = popTemp->stkMachine.Num();
        for (j = 1; j <= iM; j++)
        {
            pmcTemp = popTemp->GetMachine(j);
            pmcTemp->AddTask(popTemp);
            pwkTemp = WkCenter.Getw_mID(pmcTemp->mID);
            pwkTemp->AddTask(popTemp);
        }
    }

    AddLink(*pjbTemp);
    //***** add sink?
    return pjbTemp;
}

void TShop::Remove(TJob* pJob)
{
    TWorkcenterList wkl = WkCenter;
    TSequenceList sql = Seq;
    // remove operations from sequence list
    TOperation* pop;
    TSequence* pseq;
    int i;
    if (Seq.Num() > 0)
    {
        for (i = 1; i <= pJob->Num(); i++)
        {
            pop = pJob->Get(i);
            pseq = sql.Gets_oID(pop->oID);
            if (pseq)
            {
                pseq->Locate(pop->oID);
                pseq->Delete();
            }
        }
    }

    TJobList jbl = Job;
    jbl.RemoveID(pJob->jID);
    ClearJob();
    for (i = 1; i <= jbl.Num(); i++) Append(jbl[i]);
    if (jbl.Num() > 0) Append(sql);
}

TWorkcenter* TShop::Append(TWorkcenter& wkX)
{
    return (WkCenter.Append(wkX));
}

void TShop::Remove(TMachine* pmc)
{
    TOperation* pop;
    TJob* pjb;
    TWorkcenter* pwk;
    int i, bReGraph = 0;
    // remove machine assignment on operations
    WkCenter.Head();
    for (i = 1; i <= WkCenter.Num(); i++)
    {
        pwk = (TWorkcenter*)WkCenter.Next();
        if (pwk->Locate(pmc) > 0)
        {
            while (pmc->stkTask.Num() > 0)
            {
                pop = pmc->GetTask(1);
                if (pop->stkMachine.Num() <= 1)
                {  // remove operation
                    pjb = Job.Getj_oID(pop->oID);
                    if (pjb->Num() <= 1)
                        Job.Remove(pjb);
                    else
                        pjb->Remove(pop);
                    bReGraph = 1;
                }
                else
                {  // remove machine
                    pop->stkMachine.Locate(pmc);
                    pop->stkMachine.Delete();
                }
                pmc->RemoveTask(pop);
            }
            pwk->Remove(pmc);
            break;
        }
    }
    ClearSequence();
    if (bReGraph) ReGraph();
}

void TShop::Remove(TWorkcenter* pwk)
{
    TMachine* pmc;
    WkCenter.Head();
    int j;
    for (int i = 1; i <= WkCenter.Num(); i++)
    {
        if (WkCenter.Locate(pwk) > 0)
        {
            for (j = 1; j <= pwk->Num(); j++)
            {
                pmc = pwk->Get(1);
                Remove(pmc);
            }
            WkCenter.Locate(pwk);
            WkCenter.Delete();
        }
    }
}

TSequence* TShop::Append(TSequence& sqX)
{
    TSequence* pRtn = Seq.Append(sqX);
    AddLink(sqX);  // add link to SGraph
    ReCalc();
    return pRtn;
}

void TShop::Append(TSequenceList& sqlX)
{
    int iN = sqlX.Num();
    for (int i = 1; i <= iN; i++) Append(sqlX[i]);
    //  ReCalc();
}

void TShop::Append(TPeriod& prdX)
{
    if (prdX.fStart > 7)
        Avail.Holiday.Append(prdX);
    else
        Avail.Week.Append(prdX);
}

void TShop::Remove(TSequence* psqX)
{
    DLink(*psqX);
    Seq.Remove(psqX);
}

void TShop::Remove(TSequenceList* psqlX)
{
    int iN = psqlX->Num();
    for (int i = 1; i <= iN; i++)
    {
        Remove(psqlX->Get(i));
    }
}

void TShop::RemoveSeq(TWorkcenter* pWk)
{
    int i, iN;
    TSequence* pSeq;
    iN = pWk->Num();
    for (i = 1; i <= iN; i++)
    {
        pSeq = Seq.Gets_mID((*pWk)[i].mID);
        if (pSeq)
        {
            Remove(pSeq);
        }
    }
}

void TShop::RandomStatusSingle(TCHAR cFrom, TCHAR cTo)
{
    int i, j;
    TJob* pjbTemp;
    TOperation* popTemp;
    TMachine* pmcTemp;
    // chech if it is the single m/c workcenter
    for (i = 1; i <= Job.Num(); i++)
    {
        pjbTemp = Job.Get(i);
        for (j = 1; j <= pjbTemp->Num(); j++)
        {
            popTemp = pjbTemp->Get(j);
            if (popTemp->stkMachine.Num() == 1)
            {
                pmcTemp = popTemp->GetMachine(1);
                if (pmcTemp->iBatchSize == 1) popTemp->sMC_Status = TCHAR(Random((int)cFrom, (int)cTo));
            }
        }
    }
}

void TShop::SetStatusBatch(char cFrom, int iProcessTime)
{
    int i, j;
    TJob* pjbTemp;
    TOperation* popTemp;
    TMachine* pmcTemp;
    // chech if it is the single m/c workcenter
    for (i = 1; i <= Job.Num(); i++)
    {
        pjbTemp = Job.Get(i);
        for (j = 1; j <= pjbTemp->Num(); j++)
        {
            popTemp = pjbTemp->Get(j);
            if (popTemp->stkMachine.Num() == 1)
            {
                pmcTemp = popTemp->GetMachine(1);
                if (pmcTemp->iBatchSize > 1)
                {
                    popTemp->sMC_Status = cFrom;
                    popTemp->fProcessTime = iProcessTime;
                }
            }
        }
    }
}

void TShop::RandomStatusParallel(TCHAR cFrom, TCHAR cTo)
{
    int i, j;
    TJob* pjbTemp;
    TOperation* popTemp;
    // chech if it is the single m/c workcenter
    for (i = 1; i <= Job.Num(); i++)
    {
        pjbTemp = Job.Get(i);
        for (j = 1; j <= pjbTemp->Num(); j++)
        {
            popTemp = pjbTemp->Get(j);
            if (popTemp->stkMachine.Num() > 1) popTemp->sMC_Status = TCHAR(Random((int)cFrom, (int)cTo));
        }
    }
}

void TShop::RandomStatusBatch(TCHAR cFrom, TCHAR cTo, int iFrom, int iTo)
{
    int i, j;
    TJob* pjbTemp;
    TOperation* popTemp;
    TMachine* pmcTemp;
    // chech if it is the single m/c workcenter
    for (i = 1; i <= Job.Num(); i++)
    {
        pjbTemp = Job.Get(i);
        for (j = 1; j <= pjbTemp->Num(); j++)
        {
            popTemp = pjbTemp->Get(j);
            if (popTemp->stkMachine.Num() == 1)
            {
                pmcTemp = popTemp->GetMachine(1);
                if (pmcTemp->iBatchSize > 1)
                {
                    popTemp->sMC_Status = TCHAR(Random((int)cFrom, (int)cTo));
                    popTemp->fProcessTime = Random(iFrom, iTo);
                }
            }
        }
    }
}

TJob* TShop::GetJob(int iSink)
{
    int i, iN;
    if (iSink > 0)  // if iSink is the rank
        return (TJob*)SinkJob[iSink];
    if (iSink < 0)  // if iSink is the node ID
    {               // find the rank of sink node
        iN = DGraph.NumSink();
        for (i = 1; i <= iN; i++)
        {
            if (DGraph.GetSink(i)->nID == iSink) return (TJob*)SinkJob[i];
        }
    }
    return 0;
}

TWorkcenter* TShop::GetWkc(TWorkcenterID wID)
{
    return WkCenter.GetID(wID);
}

TMachine* TShop::GetMC(TMachineID mID)
{
    return WkCenter.Getm_mID(mID);
}

void TShop::GetSequence(TWorkcenter* pWk, TSequenceList& sqList)
{
    int i;
    TMachine* pMachine;
    TSequence* pSeq;
    sqList.Clear();
    for (i = 1; i <= pWk->Num(); i++)
    {
        pMachine = pWk->Get(i);
        pSeq = Seq.Gets_mID(pMachine->mID);
        if (pSeq) sqList.Append(*pSeq);
    }
}

int TShop::NumMachine()
{
    int iTotal = 0;
    TWorkcenter* pWk;
    WkCenter.Head();
    for (int i = 1; i <= WkCenter.Num(); i++)
    {
        pWk = (TWorkcenter*)WkCenter.Next();
        iTotal += pWk->Num();
    }
    return iTotal;
}

int TShop::NumSequence()
{
    int iTotal = 0;
    TSequence* pSeq;
    Seq.Head();
    for (int i = 1; i <= Seq.Num(); i++)
    {
        pSeq = (TSequence*)Seq.Next();
        iTotal += pSeq->Num();
    }
    return iTotal;
}

void TShop::Assign(TOperationID oIDI, TWorkcenterID wIDI)
{
    int iN;
    TWorkcenter* pwkTemp;
    TMachine* pmcTemp;
    TOperation* popTemp;
    pwkTemp = WkCenter.GetID(wIDI);
    popTemp = Job.Geto_oID(oIDI);
    if ((pwkTemp) && (popTemp))
    {
        pwkTemp->AddTask(popTemp);
        iN = pwkTemp->Num();
        for (int i = 1; i <= iN; i++)
        {
            pmcTemp = pwkTemp->Get(i);
            pmcTemp->AddTask(popTemp);
            popTemp->AddMachine(pmcTemp);
        }
    }
}

int TShop::DAssign_wk(TOperationID oIDI, TWorkcenterID wIDI)
{
    int iN, i;
    TWorkcenter* pwkTemp;
    TMachine* pmcTemp;
    TOperation* popTemp;
    pwkTemp = WkCenter.GetID(wIDI);
    popTemp = Job.Geto_oID(oIDI);
    if ((pwkTemp) && (popTemp))
    {
        iN = pwkTemp->Num();
        for (i = 1; i <= iN; i++)
        {
            pmcTemp = pwkTemp->Get(i);
            pmcTemp->RemoveTask(popTemp);
            popTemp->RemoveMachine(pmcTemp);
        }
        return (pwkTemp->RemoveTask(popTemp));
    }
    return 0;
}

int TShop::DAssign_mc(TOperationID oIDI, TMachineID mIDI)
{
    TMachine* pmcTemp;
    TOperation* popTemp;

    pmcTemp = WkCenter.Getm_mID(mIDI);
    popTemp = Job.Geto_oID(oIDI);
    TWorkcenter* pwk = WkCenter.Getw_mID(mIDI);
    if (pwk) pwk->RemoveTask(popTemp);
    if ((pmcTemp) && (popTemp))
    {
        if (popTemp->stkMachine.Locate(pmcTemp) > 0) popTemp->stkMachine.Delete();
        return (pmcTemp->RemoveTask(popTemp));
    }
    return 0;
}

int TShop::LoadWk(istream& is)
{
    Clear();
    is >> WkCenter;
    return 1;
}

int TShop::LoadAvl(istream& is)
{
    Avail.Clear();
    is >> Avail;
    return 1;
}

int TShop::LoadJob(istream& is)
{
    int i, j, k, iL, iM, iN;
    CHAR szTemp[200];
    TMachineID mIDI;
    TOperation* popTemp;
    TJob* pjbTemp;
    TWorkcenter* pwkTemp;
    TMachine* pmcTemp;

    WkCenter.ClearTask();
    Job.Clear();
    SinkJob.Clear();
    DGraph.Clear();
    SGraph.Clear();
    Seq.Clear();

    std::streampos pos = is.tellg();
    is >> Job;
    is.clear();
    is.seekg(pos, ios::beg);

    //****** load process machine & task
    iN = Job.Num();
    for (i = 1; i <= iN; i++)
    {
        is.getline(szTemp, 200);  // Job: xxxxx
        is.getline(szTemp, 200);  // release
        is.getline(szTemp, 200);  // duedate
        is.getline(szTemp, 200);  // priority
        is.getline(szTemp, 200);  // Operations
        pjbTemp = Job.Get(i);
        iM = pjbTemp->Num();
        for (j = 1; j <= iM; j++)
        {
            is.getline(szTemp, 200);  // #xxxxx
            is.getline(szTemp, 200);  // process time
            is.getline(szTemp, 200);  // release time
            is.getline(szTemp, 200);  // prior op
            is.ignore(19);            // process machine
            is.getline(szTemp, 200);
            iL = strcount(szTemp, ',') + 1;
            popTemp = pjbTemp->Get(j);
            for (k = 1; k <= iL; k++)
            {
                mIDI = strextc_MachineID(szTemp, ',');
                if (mIDI)
                {
                    //***** add process machine
                    pwkTemp = WkCenter.Getw_mID(mIDI);
                    if (!pwkTemp)
                    {
                        Job.Clear();
                        return 0;  // workcenter not found
                    }
                    pmcTemp = pwkTemp->GetID(mIDI);
                    if (pmcTemp)
                    {
                        popTemp->AddMachine(pmcTemp);
                        pmcTemp->AddTask(popTemp);
                        pwkTemp->AddTask(popTemp);
                    }
                    else
                    {
                        Job.Clear();
                        return 0;
                    }
                }
            }
            is.getline(szTemp, 200);  // machine status
            is.getline(szTemp, 200);  // user
            is.ignore(1);
        }
    }
    //***** AddLink to DGraph & SGraph *****//
    for (i = 1; i <= iN; i++)
    {
        pjbTemp = Job.Get(i);
        AddLink(*pjbTemp);
    }
    return 1;
}

int TShop::LoadSeq(istream& is)
{
    int i, j;
    TMachine* pmcTemp;
    TSequence* psqTemp;
    TSequenceList sqlTemp;

    is >> sqlTemp;

    // check sequence
    for (i = 1; i <= sqlTemp.Num(); i++)
    {
        psqTemp = sqlTemp.Get(i);
        pmcTemp = WkCenter.Getm_mID(psqTemp->mID);
        if (pmcTemp == 0) return 0;
        for (j = 1; j <= psqTemp->Num(); j++)
        {
            if (pmcTemp->IsTask((*psqTemp)[j]) == 0) return 0;
        }
    }
    //***** AddLink for SGraph *****//
    ClearSequence();
    Append(sqlTemp);
    return 1;
}

int TShop::SaveAll(LPCTSTR szName)
{
    CString name = std_filename(szName, szFileExtWkc);
    ofstream oFile(name);
    if (!oFile) return 0;
    oFile << WkCenter;
    oFile.close();

    name = std_filename(szName, szFileExtAvl);
    oFile.open(name);
    if (!oFile) return 0;
    oFile << Avail;
    oFile.close();

    name = std_filename(szName, szFileExtJob);
    oFile.open(name);
    if (!oFile) return 0;
    oFile << Job;
    oFile.close();

    name = std_filename(szName, szFileExtSeq);
    oFile.open(name);
    if (!oFile) return 0;
    oFile << Seq;
    oFile.close();
    return 1;
}

//*******
int TShop::ReCalc(int fl)
{
    // fl = 1  force recalc
    // fl = 0  ReCalc
    if (fl)
    {
        TSequenceList sql = Seq;
        ClearSequence();
        Append(sql);
    }
    TOperation* popTemp;
    TNode* pndTemp;
    int i, j, iN, iM, iRecalc = SGraph.ReCalc();
    /*
      SGraph = DGraph;
      // load links from Seq
      iN = Seq.Num();
      for(i=1;i<=iN;i++)
      {
        AddLink(Seq[i]);
      }
    */
    if (iRecalc == 2) return 0;  // infeasible sequence
    if (iRecalc == 1)
    {
        TNode* pndTemp2;
        UpdateOperation();
        // in the case that there is dummy nodes
        iN = SGraph.NumDummy();
        for (i = 1; i <= iN; i++)
        {
            pndTemp = SGraph.GetDummy(i);  // GetID(-1-i);
            iM = pndTemp->stkPriorArc.Num();
            for (j = 1; j <= iM; j++)
            {
                pndTemp2 = pndTemp->GetPriorNode(j);
                if (pndTemp2->IsOperation())
                {
                    popTemp = pndTemp2->op;
                    popTemp->fStartTime = pndTemp->EST();
                    popTemp->fFinishTime = popTemp->fStartTime + pndTemp->op->fActProcessTime;
                    // setup time is assigned in "BatchLink"
                    popTemp->fDueDate = pndTemp->DueDate();
                    popTemp->mID = pndTemp->op->mID;
                }
            }
        }
    }
    return 1;
}

double TShop::JobCompletion(TJobID jIDI)
{
    TJob* pJob;
    TNode* pNode;
    TOperation* pOper;
    double fHighest = 0;
    int i, j, iN = SinkJob.Num();
    for (i = 1; i <= iN; i++)
    {
        pJob = (TJob*)SinkJob[i];
        if (pJob->jID == jIDI)
        {
            pNode = SGraph.GetSink(i);
            for (j = 1; j <= pNode->stkPriorArc.Num(); j++)
            {
                pOper = (pNode->GetPriorNode(j))->op;
                if (pOper->fFinishTime > fHighest) fHighest = pOper->fFinishTime;
            }
            return fHighest;
        }
    }
    return 0;
}

void TShop::ReGraph()
{
    DGraph.Clear();
    SGraph.Clear();
    SinkJob.Clear();
    for (int i = 1; i <= Job.Num(); i++) AddLink(Job[i]);
    TSequenceList sql = Seq;
    ClearSequence();
    Append(sql);
}

double TShop::Cmax()
{
    TJob* pJob;
    TOperation* pOper;
    double fHighest = 0;
    int i, j;
    Job.Head();
    for (i = 1; i <= Job.Num(); i++)
    {
        pJob = (TJob*)Job.Next();
        pJob->Head();
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOper = (TOperation*)pJob->Next();
            if (fHighest < pOper->fFinishTime) fHighest = pOper->fFinishTime;
        }
    }
    return fHighest;
}

double TShop::Lmax()
{
    double fLateness, fBest = -fBigM;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = JobCompletion(jbTemp.jID) - jbTemp.fDueDate;
        if (fBest < fLateness) fBest = fLateness;
    }
    return fBest;
}

double TShop::WLmax()
{
    double fLateness, fWLj, fBest = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = (JobCompletion(jbTemp.jID) - jbTemp.fDueDate);

        fWLj = (fLateness * jbTemp.fWeight);
        if (fBest < fWLj) fBest = fWLj;
    }
    return fBest;
}

double TShop::WTmax()
{
    double fLateness, fWTj, fBest = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = (JobCompletion(jbTemp.jID) - jbTemp.fDueDate);
        if (fLateness > 0)
        {
            fWTj = (fLateness * jbTemp.fWeight);
            if (fBest < fWTj) fBest = fWTj;
        }
    }
    return fBest;
}

double TShop::WC()
{
    double fCj, fSum = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fCj = JobCompletion(jbTemp.jID);
        fSum += (fCj * jbTemp.fWeight);
    }
    return fSum;
}

double TShop::WFlow()
{
    double fCj, fSum = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fCj = JobCompletion(jbTemp.jID) - jbTemp.fReleaseTime;
        fSum += (fCj * jbTemp.fWeight);
    }
    return fSum;
}

double TShop::WT()
{
    double fLateness, fSum = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = (JobCompletion(jbTemp.jID) - jbTemp.fDueDate);
        if (fLateness > 0) fSum += (fLateness * jbTemp.fWeight);
    }
    return fSum;
}

double TShop::WTE()
{
    double fLateness, fSum = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = (JobCompletion(jbTemp.jID) - jbTemp.fDueDate);
        if (fLateness > 0)
            fSum += (fLateness * jbTemp.fWeight);
        else
            fSum += (fLateness * jbTemp.fEWeight);
    }
    return fSum;
}

double TShop::WU()
{
    double fLateness, fSum = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = (JobCompletion(jbTemp.jID) - jbTemp.fDueDate);
        if (fLateness > 0) fSum += jbTemp.fWeight;
    }
    return fSum;
}

double TShop::U()
{
    double fLateness, fSum = 0;
    TJob jbTemp;
    int iN = Job.Num();
    for (int i = 1; i <= iN; i++)
    {
        jbTemp = Job[i];
        fLateness = (JobCompletion(jbTemp.jID) - jbTemp.fDueDate);
        if (fLateness > 0) fSum++;
    }
    return fSum;
}

double TShop::Objective(
    double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight))
{  // calc the job completion time from SGraph.
    TArrayF fNComplete, fNDue, fNWeight, fNEWeight;
    double fSum = 0;
    TNode* pskNode;
    TJob jbTemp;
    int iN = Job.Num();
    SGraph.ReCalc();
    for (int i = 0; i < iN; i++)
    {
        jbTemp = Job[i + 1];
        pskNode = SGraph.GetSink(i + 1);
        fNComplete[i] = pskNode->EST();
        fNDue[i] = jbTemp.fDueDate;
        fNWeight[i] = jbTemp.fWeight;
        fNEWeight[i] = jbTemp.fEWeight;
    }
    return Obj(iN, fNDue, fNComplete, fNWeight, fNEWeight);
}

double TShop::AvgSetup(TWorkcenter* pWkCenter)
{
    // check all operations on that workcenter
    double fSum = 0;
    int i, k, l, iCount = 0;
    TMachine* pMachine;
    TOperation *popFrom, *popTo;
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pMachine = pWkCenter->Get(i);
        for (k = 1; k <= pMachine->stkTask.Num(); k++)
        {
            popFrom = pMachine->GetTask(k);
            for (l = 1; l <= pMachine->stkTask.Num(); l++)
            {
                popTo = pMachine->GetTask(l);
                if (k != l)
                {
                    fSum += pMachine->suSetup.Get(popFrom->sMC_Status, popTo->sMC_Status);
                    iCount++;
                }
            }
        }
    }
    return (fSum / iCount);
}

double TShop::AvgSetup()
{
    // check all operations on the shop
    double fSum = 0;
    int i, j, k, l, iCount = 0;
    TWorkcenter* pWkCenter;
    TMachine* pMachine;
    TOperation *popFrom, *popTo;
    for (j = 1; j <= WkCenter.Num(); j++)
    {
        pWkCenter = WkCenter.Get(j);
        for (i = 1; i <= pWkCenter->Num(); i++)
        {
            pMachine = pWkCenter->Get(i);
            for (k = 1; k <= pMachine->stkTask.Num(); k++)
            {
                popFrom = pMachine->GetTask(k);
                for (l = 1; l <= pMachine->stkTask.Num(); l++)
                {
                    popTo = pMachine->GetTask(l);
                    if (k != l)
                    {
                        fSum += pMachine->suSetup.Get(popFrom->sMC_Status, popTo->sMC_Status);
                        iCount++;
                    }
                }
            }
        }
    }
    return (fSum / iCount);
}

double TShop::AvgProcessTime(TWorkcenter* pWkCenter)
{
    // check all operations on that workcenter
    double fSum = 0;
    int i;
    TOperation* popTemp;
    for (i = 1; i <= pWkCenter->stkTask.Num(); i++)
    {
        popTemp = pWkCenter->GetTask(i);
        fSum += popTemp->fProcessTime;
    }
    return (fSum / pWkCenter->stkTask.Num());
}

double TShop::AvgProcessTime()
{
    // check all operations on that workcenter
    double fSum = 0;
    int i, j;
    TOperation* popTemp;
    TWorkcenter* pWkCenter;

    for (j = 1; j <= WkCenter.Num(); j++)
    {
        pWkCenter = WkCenter.Get(j);
        for (i = 1; i <= pWkCenter->stkTask.Num(); i++)
        {
            popTemp = pWkCenter->GetTask(i);
            fSum += popTemp->fProcessTime;
        }
    }
    return (fSum / pWkCenter->stkTask.Num());
}

void TShop::FindPrecedent(TJob& jbX, TOperationID oIDI, TOperation* popCurrent, TStackP stkX)
{  // not use
    int i, j, iM, iN = popCurrent->stkPriorOp.Num();
    TStackP stkNextCall;
    TOperation *popPrior, *popTemp;
    for (i = 1; i <= iN; i++)
    {
        stkNextCall.Clear();
        popPrior = popCurrent->GetPrior(i);
        if (stkX.Member(popPrior))
        { /* found precedent */
            jbX.Link(popPrior->oID, oIDI);
        }
        else
        {
            iM = stkX.Num();
            for (j = 1; j < iM; j++)
            {
                popTemp = (TOperation*)stkX[j];
                if (popTemp->fStartTime < popPrior->fStartTime) stkNextCall.Push(popTemp);
            }
            if (stkNextCall.Num())
            {
                FindPrecedent(jbX, oIDI, popPrior, stkNextCall);
            }
        }
    }
}

TJob& TShop::OperationOnWkc(TWorkcenterID wIDI)
{  // not use
    TWorkcenter* pwkTemp;
    int iN;
    TOperation* popTemp;
    TJob* pjbReturn = new TJob;
    TStackP stkList;
    ReCalc();
    pwkTemp = WkCenter.GetID(wIDI);
    /*** add operations to return list ***/
    iN = pwkTemp->stkTask.Num();
    for (int i = 1; i <= iN; i++)
    {
        popTemp = pwkTemp->GetTask(i);
        stkList.Push(popTemp);
        pjbReturn->Append(*popTemp);
        /*
            // find the sink node and sum process time from
            // the current node to the sink
            pndCurrent = SGraph.GetID(popTemp->oID);
            nIDNext = pndCurrent->GetPostID(1);
            pndCurrent = SGraph.GetID(nIDNext);
            while(nIDNext>SGraph.nLastSink)
            {
              nIDNext = pndCurrent->GetPostID(1);
              pndCurrent = SGraph.GetID(nIDNext);
            }
        */
    }
    /*** check precedent relationship ***/
    iN = stkList.Num();
    for (int i = 1; i <= iN; i++)
    {
        popTemp = (TOperation*)stkList.Pop();
        FindPrecedent(*pjbReturn, popTemp->oID, popTemp, stkList);
        stkList.Push(popTemp);
    }
    return *pjbReturn;
}

void TShop::GraphOnWkc(TWorkcenter* pWkCenter, TDGraph& Graph)
{
    int i = 1;
    TNode* pNode;
    Graph = SGraph;
    while (i <= Graph.Num())  // loop but skip the not related nodes
    {
        pNode = Graph.Get(i);
        if (((pNode->IsOperation()) && (!pWkCenter->IsTask(pNode->nID))) || (pNode->IsDummy()))
            Graph.Reduce(pNode);
        else
            i++;
    }
    Graph.ReCalc();
}

void TShop::ATC(double _fK1, double _fK2)
{
    fK1 = _fK1;
    fK2 = _fK2;
    PriorityRule(IndexATC);
}

void TShop::EDD_J()
{
    PriorityRule(IndexJobDueDate);
}

void TShop::EDD_O()
{
    PriorityRule(IndexOpDueDate);
}

void TShop::SPT()
{
    PriorityRule(IndexSPT);
}

void TShop::LPT()
{
    PriorityRule(IndexLPT);
}

void TShop::FCFS()
{
    PriorityRule(IndexFCFS);
}

void TShop::TER(double _fK1, double _fK2)
{
    fK1 = _fK1;
    fK2 = _fK2;
    PriorityRule(IndexTER);
}

void TShop::SB(TSequenceList& (*WkcSeq)(TTool& WkTool),
    double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight),
    int iLocal)
{
    //***** Initialize
    TStackP WkUnschedList, WkcSchedList, WkListTemp;
    TWorkcenter *pWkNeck, *pWk;
    TSequenceList sqlWorst;
    TTool WkTool;
    TDGraph GraphTemp;
    double fWorst = -fBigM, fTemp, fPreObj = fBigM;
    int i, iN;

    ClearSequence();
    WkTool.pShop = this;
    WkTool.SetObj(Obj);
    iN = WkCenter.Num();
    WkCenter.Head();
    for (i = 1; i <= iN; i++) WkUnschedList.Push((TWorkcenter*)WkCenter.Next());
    TDGraph Grh;
    while (iN)
    {
        //***** Find bottleneck workcenter
        fWorst = -fBigM;
        for (i = 1; i <= iN; i++)
        {
            pWk = (TWorkcenter*)WkUnschedList[i];
            GraphOnWkc(pWk, Grh);
            WkTool.SetData(this, pWk, &Grh);
            WkTool.ClearSequence();
            WkTool.Append(WkcSeq(WkTool));
            fTemp = WkTool.Obj();
            if (fTemp > fWorst)
            {
                fWorst = fTemp;
                pWkNeck = pWk;
                sqlWorst = WkTool.Seq;
            }
        }

        //***** Sequence the bottleneck
        Append(sqlWorst);
        WkUnschedList.Locate(pWkNeck);
        pWk = (TWorkcenter*)WkUnschedList.Current();
        WkUnschedList.Delete();
        WkcSchedList.Push(pWk);

        //***** Local re-optimization
        for (i = 1; i <= iLocal; i++)
        {
            if (i >= WkcSchedList.Num()) break;
            pWk = (TWorkcenter*)WkcSchedList[WkcSchedList.Num() - i];
            RemoveSeq(pWk);
            ReCalc();
            GraphOnWkc(pWk, GraphTemp);
            WkTool = TTool(this, pWk, &GraphTemp, Obj);
            Append(WkcSeq(WkTool));
            cout << "ReOpt wkc:" << pWk->wID << "  new value =" << WkTool.Obj() << endl;
        }

        iN = WkUnschedList.Num();
    }

    //***** Final re-optimization
}

int TShop::IsValid()
{
    // check machine-operation assignment
    // check completion
    int i, j, iN, iM, iNumMachine = 0, iNumOperation = 0, iNumOperationInSeq = 0, iOp;
    TSequence seqTemp;
    TOperation opTemp;
    TMachine* pmcTemp;
    iN = WkCenter.Num();
    for (i = 1; i <= iN; i++) iNumMachine += WkCenter[i].Num();
    iN = Job.Num();
    if (iN == 0) return 0;
    for (i = 1; i <= iN; i++) iNumOperation += Job[i].Num();
    iN = Seq.Num();
    for (i = 1; i <= iN; i++)
    {
        seqTemp = Seq[i];
        pmcTemp = WkCenter.Getm_mID(seqTemp.mID);
        iM = seqTemp.Num();
        for (j = 1; j <= iM; j++)
        {
            iOp = seqTemp[j];
            if (iOp > 0)
            {
                if (!(pmcTemp->IsTask(iOp))) return 0;
                iNumOperationInSeq++;
            }
        }
    }
    if ((Seq.Num() != iNumMachine) || (iNumOperationInSeq != iNumOperation)) return 0;

    // check deadlock
    if (SGraph.ReCalc() == 2)
        return 0;
    else
        return 1;
}

int TShop::IsValid(TSequence& sq)
{
    if (!WkCenter.Getw_mID(sq.mID)) return 0;
    sq.Head();
    for (int i = 1; i <= sq.Num(); i++)
    {
        if (!Job.Getj_oID(sq.Next())) return 0;
    }
    return 1;
}

int TShop::IsValid(TSequenceList& sql)
{
    TSequence* psq;
    if (sql.Num() == 0) return 0;
    sql.Head();
    for (int i = 1; i <= sql.Num(); i++)
    {
        psq = (TSequence*)sql.Next();
        if (!IsValid(*psq)) return 0;
    }
    return 1;
}

int TShop::IsComplete()
{
    int i, j;
    TJob* pjb;
    TOperation* pop;
    // check operations if there is a machine to process them
    Job.Head();
    for (i = 1; i <= Job.Num(); i++)
    {
        pjb = (TJob*)Job.Next();
        pjb->Head();
        for (j = 1; j <= pjb->Num(); j++)
        {
            pop = (TOperation*)pjb->Next();
            if (pop->stkMachine.Num() == 0) return 0;
        }
    }
    return 1;
}

int TShop::Batch(TStack& oIDList, TMachine* pMC)
{
    int i, j, iN = oIDList.Num();
    double fR = 0, fP = 0, fTemp;
    TNodeID nID;
    TNode *pndTemp, *pndDummy = SGraph.AddDummy(), *pndTo;
    TArc* paTemp;
    TOperation opTemp;

    for (i = 1; i <= iN; i++)
    {  // find process time, release time
        nID = oIDList[i];
        pndTemp = SGraph.GetID(nID);
        SGraph.Link(pndTemp, pndDummy, 0);
        fR = max(fR, pndTemp->op->fReleaseTime);
        fP = max(fP, pndTemp->op->fProcessTime);
    }
    fTemp = fP / pMC->fSpeed;
    for (i = 1; i <= iN; i++)
    {
        nID = oIDList.PopFirst();
        pndTemp = SGraph.GetID(nID);
        pndTemp->op->fActProcessTime = fTemp;
        pndTemp->op->mID = pMC->mID;
        for (j = 1; j <= pndTemp->stkPostArc.Num(); j++)
        {
            paTemp = pndTemp->GetPostArcPtr(j);
            pndTo = paTemp->pndTo;
            if (pndDummy != pndTo)
            {  // reroute the links through dummy node
                SGraph.Link(pndDummy, pndTo, fTemp);
                SGraph.DLink(pndTemp, pndTo);
            }
        }
    }
    opTemp.oID = pndDummy->nID;
    opTemp.mID = pMC->mID;
    opTemp.fReleaseTime = fR;
    opTemp.fProcessTime = fP;
    opTemp.fActProcessTime = fTemp;
    opTemp.sMC_Status = pndTemp->op->sMC_Status;
    pndDummy->op = BatchInfo.Append(opTemp);
    return pndDummy->nID;  // -2,-3,...
}

int TShop::BatchAdd(int iBatch, int nID)
{  // add operation to batch -- doesn't change setup time, check sMC_Status before add
    TNode *pDummy = SGraph.GetID(iBatch), *pNode = SGraph.GetID(nID), *pndTo;
    if (!(pDummy && pNode)) return 0;
    TMachine* pMC = WkCenter.Getm_mID(pDummy->op->mID);
    double fR = pDummy->op->fReleaseTime, fP = pDummy->op->fProcessTime, fAP = fP / pMC->fSpeed;
    int i;
    fR = max(fR, pNode->op->fReleaseTime);
    fP = max(fP, pNode->op->fProcessTime);
    SGraph.Link(pNode, pDummy, 0);
    for (i = 1; i <= pNode->stkPostArc.Num(); i++)
    {
        pndTo = pNode->GetPostNode(i);
        if (pndTo != pDummy)
        {
            SGraph.Link(pDummy, pndTo, fAP);
            SGraph.DLink(pNode, pndTo);
        }
    }
    // update
    pDummy->op->fReleaseTime = fR;
    BSetProcessTime(pDummy, fP);
    pNode->op->fSetupTime = pDummy->op->fSetupTime;
    return 1;
}

double TShop::BGetProcessTime(int bID)
{
    TNode* pNode = SGraph.GetID(bID);
    return BGetProcessTime(pNode);
}

double TShop::BGetProcessTime(TNode* pNode)
{
    return pNode->op->fProcessTime;
}

double TShop::BGetReleaseTime(int bID)
{
    TNode* pNode = SGraph.GetID(bID);
    return BGetReleaseTime(pNode);
}

double TShop::BGetReleaseTime(TNode* pNode)
{
    return pNode->op->fReleaseTime;
}

void TShop::BSetProcessTime(TNode* pDummy, double fP)
{  // for batch:
    double fTemp;
    int i;
    TNode *pndFrom, *pndTo;
    TMachine* pMC = WkCenter.Getm_mID(pDummy->op->mID);
    // update the prior nodes
    // time = current weight - (p-actual p)
    fTemp = fP / pMC->fSpeed;
    pDummy->op->fProcessTime = fP;
    pDummy->op->fActProcessTime = fTemp;
    for (i = 1; i <= pDummy->stkPriorArc.Num(); i++)
    {
        pndFrom = pDummy->GetPriorNode(i);
        if (pndFrom->IsOperation()) pndFrom->op->fActProcessTime = fTemp;
    }
    for (i = 1; i < pDummy->stkPostArc.Num(); i++)
    {
        pndTo = pDummy->GetPostNode(i);
        SGraph.RLink(pDummy, pndTo, fTemp);
    }
}

void TShop::BSetSetupTime(TNode* pDummy, double fTime)
{  // for batch
    TNode* pndTemp;
    pDummy->op->fSetupTime = fTime;
    for (int i = 1; i <= pDummy->stkPriorArc.Num(); i++)
    {
        pndTemp = pDummy->GetPriorNode(i);
        if (pndTemp->IsOperation()) pndTemp->op->fSetupTime = fTime;
    }
}

TStatus TShop::BGetStatus(int bID)
{
    // ruturn machine status of the first operation in the batch
    TNode* pNode = SGraph.GetID(bID);
    return BGetStatus(pNode);
}

TStatus TShop::BGetStatus(TNode* pNode)
{
    return pNode->op->sMC_Status;
}

void TShop::BatchLink(int bID1, int bID2)
{
    TStatus sS1, sS2;
    double fPTime, fSetup;
    TNode *pNd1 = SGraph.GetID(bID1), *pNd2 = SGraph.GetID(bID2);
    TMachine* pMachine = WkCenter.Getm_mID(pNd2->op->mID);
    if (!((pNd1) && (pNd2) && (pMachine))) return;
    if (pNd1->nID != 0)
    {
        sS1 = BGetStatus(pNd1);
        sS2 = BGetStatus(pNd2);
        fPTime = BGetProcessTime(pNd1) / pMachine->fSpeed;
        pNd1->op->fActProcessTime = fPTime;

        fSetup = pMachine->suSetup.Get(sS1, sS2);
        BSetSetupTime(pNd2, fSetup);

        //*** add links in SGraph & update Graph
        SGraph.Link(pNd1, pNd2, fPTime + fSetup);
    }
    else
    {  // the first operation
        sS1 = pMachine->sInit;
        sS2 = BGetStatus(pNd2);
        fSetup = pMachine->suSetup.Get(sS1, sS2);
        BSetSetupTime(pNd2, fSetup);
        fPTime = pMachine->fAvailTime + fSetup;
        SGraph.Link(pNd1, pNd2, fPTime);
    }
}

void TShop::AdjustHoliday()
{
    // read operation start-end time
    // and replace with the time that includes working shift & holidays
    return;

    int i, j;
    TOperation* pOper;
    TJob* pJob;

    // find min starting time
    double fMinStart = fBigM;
    Job.Head();
    for (i = 1; i <= Job.Num(); i++)
    {
        pJob = (TJob*)Job.Next();
        pJob->Head();
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOper = (TOperation*)pJob->Next();
            if (pOper->fStartTime < fMinStart) fMinStart = pOper->fStartTime;
        }
    }
    if (fMinStart == fBigM) return;  // no operations
    Avail.Init(fMinStart);

    // convert the time
    Job.Head();
    for (i = 1; i <= Job.Num(); i++)
    {
        pJob = (TJob*)Job.Next();
        pJob->Head();
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOper = (TOperation*)pJob->Next();
            pOper->fStartTime = Avail.Convert(pOper->fStartTime);
            pOper->fFinishTime = Avail.Convert(pOper->fFinishTime);
        }
    }
}

//*******

void TShop::SB(double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight),
    double (*BNSeq)(TTool& WkTool),
    double (*OptSeq)(TTool& WkTool),
    void (*LocalOpt)(TShop& Shop,
        TStackP& WkcSchedList,
        double (*OptSeq)(TTool& WkTool),
        double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight)),
    void (*FinalOpt)(TShop& Shop,
        TStackP& WkcSchedList,
        double (*OptSeq)(TTool& WkTool),
        double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight)))
{
    // SB(bottleneckdetermination,bottlenecksequencing,localreoptimization,finalreoptimization)
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
    const int DISPLAY = 0,  // 2=display all 1=display w/o seq
        SAVE = 0;

    ofstream of("sb.out");
    ClearSequence();
    WkTool.SetObj(Obj);
    iN = WkCenter.Num();
    WkCenter.Head();
    for (i = 1; i <= iN; i++) WkUnschedList.Push((TWorkcenter*)WkCenter.Next());

    TDGraph Grh;

    while (WkUnschedList.Num())
    {
        //*****
        WkUnschedList.Head();
        if (DISPLAY)
        {
            cout << "Unsched : ";
            for (i = 1; i <= WkUnschedList.Num(); i++)
                cout << ((TWorkcenter*)WkUnschedList.Next())->wID << ", ";
            cout << endl;
            WkcSchedList.Head();
            cout << "Sched   : ";
            for (i = 1; i <= WkcSchedList.Num(); i++)
                cout << ((TWorkcenter*)WkcSchedList.Next())->wID << ", ";
            cout << endl;
        }
        if (SAVE)
        {
            of << "Unsched : ";
            WkUnschedList.Head();
            for (i = 1; i <= WkUnschedList.Num(); i++)
                of << ((TWorkcenter*)WkUnschedList.Next())->wID << ", ";
            of << endl;
            WkcSchedList.Head();
            of << "Sched   : ";
            for (i = 1; i <= WkcSchedList.Num(); i++) of << ((TWorkcenter*)WkcSchedList.Next())->wID << ", ";
            of << endl;
        }
        //*****

        //***** Find bottleneck workcenter
        fWorst = -fBigM;
        for (i = 1; i <= WkUnschedList.Num(); i++)
        {
            pWk = (TWorkcenter*)WkUnschedList[i];
            GraphOnWkc(pWk, Grh);
            WkTool.SetData(this, pWk, &Grh);
            WkTool.SetObj(Obj);
            fTemp = BNSeq(WkTool);  // rough sequencing to find a bottleneck wkc
            if (fTemp > fWorst)
            {
                fWorst = fTemp;
                WkToolBN = WkTool;
            }
        }

        //***** Sequence the bottleneck
        if (DISPLAY)
        {
            cout << "BN Wkc:" << WkToolBN.pWkCenter->wID << endl;
            cout << "   Obj: (before opt) " << fWorst << endl;
            if (DISPLAY == 2) cout << "   Seq: " << endl << WkToolBN.Seq;
        }
        if (SAVE)
        {
            of << "BN Wkc:" << WkToolBN.pWkCenter->wID << endl;
            of << "   Obj: (before opt) " << fWorst << endl;
            TSequenceList sql = WkToolBN.Seq;
            WkToolBN.ClearSequence();
            WkToolBN.Append(sql);
            of << "   Obj:     (reload) " << WkToolBN.Obj() << endl;
            of << "   Seq: " << endl << WkToolBN.Seq;
        }
        if (fabs(fPrevObj - fWorst) > fSmall) OptSeq(WkToolBN);
        if (DISPLAY)
        {
            cout << "Obj: (after opt) " << WkToolBN.Obj() << endl;
            if (DISPLAY == 2) cout << "Seq: " << endl << WkToolBN.Seq;
        }
        if (SAVE)
        {
            of << "Obj: (after opt) " << WkToolBN.Obj() << endl;
            of << "Seq: " << endl << WkToolBN.Seq;
            TSequenceList sql = WkToolBN.Seq;
            WkToolBN.ClearSequence();
            WkToolBN.Append(sql);
            of << "Obj:    (reload) " << WkToolBN.Obj() << endl;
            of << "Seq: " << endl << WkToolBN.Seq;
        }
        Append(WkToolBN.Seq);

        WkUnschedList.Delete(WkUnschedList.Locate(WkToolBN.pWkCenter));
        WkcSchedList.Push(WkToolBN.pWkCenter);
        fPrevObj = Objective(Obj);

        //***** Local re-optimization
        if (DISPLAY)
        {
            cout << "Local reopt!  Before obj = " << Objective(Obj) << endl;
            if (DISPLAY == 2) cout << "Summary:" << endl << Seq << endl << endl;
        }
        if (SAVE)
        {
            of << "Local reopt!  Before obj = " << Objective(Obj) << endl;
            of << "Summary:" << endl << Seq << endl << endl;
        }
        LocalOpt(*this, WkcSchedList, OptSeq, Obj);
        if (DISPLAY)
        {
            cout << "              After  obj = " << Objective(Obj) << endl;
            if (DISPLAY == 2) cout << "Summary:" << endl << Seq << endl << endl;
        }
        if (SAVE)
        {
            of << "              After  obj = " << Objective(Obj) << endl;
            of << "Summary:" << endl << Seq << endl << endl;
        }
    }

    //***** Final re-optimization
    if (DISPLAY) cout << "Final reopt!" << endl;
    FinalOpt(*this, WkcSchedList, OptSeq, Obj);
    if (DISPLAY) cout << "Objective = " << Objective(Obj) << endl;
    if (SAVE)
    {
        of << "Final reopt!" << endl;
        of << "Objective = " << Objective(Obj) << endl;
    }
    of.close();
}

void TShop::SB1()
{
    SB(::WTE, SUB_TER, SUB_WTE, LocalOpt_4Iter, LocalOpt);
}

void TShop::SB2()
{
    SB(::WTE, SUB_TER, SUB_WTE, BYPASS, BYPASS);
}
