#include "StdAfx.h"

#include "Parameter.h"
#include "Shop.h"
#include "Tool.h"

//**************** overload iostream
ostream& operator<<(ostream& os, TTool& tlX)
{
    os << "CTOOL object\n============\n";
    os << "Workcenter : " << tlX.pWkCenter->wID << "  with " << tlX.pWkCenter->Num() << " machines" << endl;
    os << tlX.SGraph;
    os << tlX.Seq;
    return os;
}

void Trace(LPCSTR szName, TTool& tl)
{
    ofstream myfile(szName);
    myfile << tl;
    myfile.close();
}

//**************** TTool

TTool::TTool(TShop* _pShop,
    TWorkcenter* _pWkCenter,
    TDGraph* _pGraph,
    double (*_Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight))
{
    pShop = _pShop;
    pWkCenter = _pWkCenter;
    DGraph = *_pGraph;
    SGraph = *_pGraph;
    ObjFnc = _Obj;
}

TTool::TTool(TTool& Tl)
{
    pShop = Tl.pShop;
    pWkCenter = Tl.pWkCenter;
    DGraph = Tl.DGraph;
    SGraph = Tl.DGraph;
    ObjFnc = Tl.ObjFnc;
    Append(Tl.Seq);
}

TTool& TTool::operator=(TTool& Tl)
{
    ClearSequence();
    pShop = Tl.pShop;
    pWkCenter = Tl.pWkCenter;
    DGraph = Tl.DGraph;
    SGraph = Tl.DGraph;
    ObjFnc = Tl.ObjFnc;
    Append(Tl.Seq);
    return *this;
}

int TTool::NumOp()
{
    int i, iTotal = 0;
    TNode* pndTemp;
    for (i = 1; i <= DGraph.Num(); i++)
    {
        pndTemp = DGraph.Get(i);
        if (pndTemp->IsOperation()) iTotal++;
    }
    return iTotal;
}

void TTool::SetData(TShop* pShopI, TWorkcenter* pWkcI, TDGraph* pGrhI)
{
    pShop = pShopI;
    pWkCenter = pWkcI;
    DGraph = *pGrhI;
    SGraph = *pGrhI;
    Seq.Clear();
}

int TTool::BatchSize(int iMachine)
{
    int iTemp;
    if (iMachine) return (pWkCenter->Get(iMachine))->iBatchSize;
    int iBest = 1;
    pWkCenter->Head();
    for (int i = 1; i <= pWkCenter->Num(); i++)
    {
        iTemp = ((TMachine*)pWkCenter->Next())->iBatchSize;
        iBest = max(iBest, iTemp);
    }
    return iBest;
}

double TTool::Obj()
{
    double fComplete[iMaxJob];
    double fDue[iMaxJob];
    double fWeight[iMaxJob];
    double fEWeight[iMaxJob];

    TJob* pJob;
    TNode* pSinkNode;
    int i, iN;
    if (SGraph.ReCalc() == 2) return fBigM;  // invalid sequence
    // Trace("Sgraph2.grh",SGraph);
    iN = SGraph.NumSink();
    for (i = 1; i <= iN; i++)
    {
        pSinkNode = SGraph.GetSink(i);
        // pJob        = (TJob*)pShop->SinkJob[i];
        pJob = pShop->GetJob(pSinkNode->nID);
        fComplete[i - 1] = pSinkNode->EST();
        fDue[i - 1] = pJob->fDueDate;
        fWeight[i - 1] = pJob->fWeight;
        fEWeight[i - 1] = pJob->fEWeight;
    }
    return ObjFnc(iN, fDue, fComplete, fWeight, fEWeight);
}

void TTool::UpdateLink(TNode* pNode, double fPTime, double fOldPTime)
{                                     // not used
    if (fPTime == fOldPTime) return;  // doesn't need update
    int iN, i;
    TNode* pNdTo;
    double fPreviousW;
    iN = pNode->stkPostArc.Num();
    for (i = 1; i <= iN; i++)
    {
        pNdTo = pNode->GetPostNode(i);  // sink
        fPreviousW = pNode->GetPostWeight(pNdTo) - fOldPTime + fPTime;
        SGraph.RLink(pNode, pNdTo, fPreviousW);
    }
}

void TTool::UpdateLink(TNodeID nID, TMachine* pMachine)
{
    if (pMachine->fSpeed == 1) return;
    TNode *pNode = SGraph.GetID(nID), *pNodeD = DGraph.GetID(nID), *pndTemp1, *pndTemp2;
    double fTemp, fOTime = pNode->op->fProcessTime, fCTime = fOTime / pMachine->fSpeed,
                  fDiff = fOTime - fCTime;
    pNode->op->fActProcessTime = fCTime;
    // check all pNode suceeding nodes
    for (int i = 1; i <= pNodeD->stkPostArc.Num(); i++)
    {
        pndTemp1 = pNodeD->GetPostNode(i);
        fTemp = pNodeD->GetPostWeight(i);
        pndTemp2 = SGraph.GetID(pndTemp1->nID);
        pNode->AddPost(pndTemp2, fTemp - fDiff);
    }
    // UpdateLink(pNode,fCTime,fOTime);
}

void TTool::Link(TNodeID nID1, TNodeID nID2, TMachine* pMachine)
{
    TNode *pNd1 = SGraph.GetID(nID1), *pNd2 = SGraph.GetID(nID2);
    Link(pNd1, pNd2, pMachine);
}

void TTool::Link(TNode* pNd1, TNode* pNd2, TMachine* pMachine)
{
    int i, iN;
    TOperation *pOp1, *pOp2;
    TNode *pNdTo, *pNdToD, *pNd2D;
    TStatus sS1, sS2;
    double fPTime, fPreviousW, fSetup, fDiff;

    if (!((pNd1) && (pNd2) && (pMachine))) return;
    if (pNd1->nID != 0)
    {
        pOp1 = pNd1->op;
        pOp2 = pNd2->op;
        sS1 = pOp1->sMC_Status;
        sS2 = pOp2->sMC_Status;
        fPTime = pOp1->fProcessTime / pMachine->fSpeed;
        fSetup = pMachine->suSetup.Get(sS1, sS2);
        SGraph.Link(pNd1, pNd2, fPTime + fSetup);

        //*** add links in SGraph & update Graph

        // update the other links if the machine speed != 100%
        if (pMachine->fSpeed != 1)
        {
            fDiff = (1 - 1 / pMachine->fSpeed) * pOp2->fProcessTime;
            pNd2D = DGraph.GetID(pNd2->nID);
            iN = pNd2->stkPostArc.Num();
            for (i = 1; i <= iN; i++)
            {
                pNdTo = pNd2->GetPostNode(i);
                pNdToD = DGraph.GetID(pNdTo->nID);
                fPreviousW = pNd2D->GetPostWeight(pNdToD);
                fPreviousW -= fDiff;
                SGraph.RLink(pNd2, pNdTo, fPreviousW);
            }
        }
        // After processing, the machine needs setup
    }
    else
    {  // the first operation
        pOp2 = pNd2->op;
        sS1 = pMachine->sInit;
        sS2 = pOp2->sMC_Status;
        fSetup = pMachine->suSetup.Get(sS1, sS2);
        fPTime = pMachine->fAvailTime + fSetup;
        SGraph.Link(pNd1, pNd2, fPTime);

        // update links for pNd2
        if (pMachine->fSpeed != 1)
        {
            fDiff = (1 - 1 / pMachine->fSpeed) * pOp2->fProcessTime;
            pNd2D = DGraph.GetID(pNd2->nID);
            iN = pNd2->stkPostArc.Num();
            for (i = 1; i <= iN; i++)
            {
                pNdTo = pNd2->GetPostNode(i);
                pNdToD = DGraph.GetID(pNdTo->nID);
                fPreviousW = pNd2D->GetPostWeight(pNdToD);
                fPreviousW -= fDiff;
                SGraph.RLink(pNd2, pNdTo, fPreviousW);
            }
        }
    }
}

void TTool::CancelLink(TNodeID nID1, TNodeID nID2)
{
    TNode *pnd1 = SGraph.GetID(nID1), *pnd2 = SGraph.GetID(nID2);
    CancelLink(pnd1, pnd2);
}

void TTool::CancelLink(TNode* pNd1, TNode* pNd2)
{
    // it doesn't remove the operations in the Seq
    TNode *pndD1 = DGraph.GetID(pNd1->nID), *pndD2 = DGraph.GetID(pNd2->nID);
    double fTemp = pndD1->GetPostWeight(pndD2);
    if (fTemp < fBigM)
        SGraph.RLink(pNd1, pNd2, fTemp);
    else
        SGraph.DLink(pNd1, pNd2);
}

/*
void TTool::DLink(TNode *pNdFrom,TNode *pNdTo)
{

   // update the other links if the machine speed != 100%
    if(pMachine->fSpeed!=1)
    {
      iN = pNd1->stkPostArc.Num();
      for(i=1;i<=iN;i++)
      {
        pNdTo = pNd1->GetPostNode(i);
        fPreviousW = pNd1->GetPostWeight(pNdTo);
        fPreviousW -= pOp1->fProcessTime;
        fPreviousW += fPTime;
        SGraph.RLink(pNd1,pNdTo,fPreviousW);
      }
    }

}
*/

void TTool::Link(TNodeID nID1, TNodeID nID2)
{  // use the first machine
    pWkCenter->Head();
    TMachine* pMachine = (TMachine*)pWkCenter->Current();
    Link(nID1, nID2, pMachine);
}

void TTool::Link(TSequence& sqX)
{
    TNodeID nPrev = 0;
    TMachine* pMachine = pWkCenter->GetID(sqX.mID);
    int i, iN = sqX.Num();
    if (!pMachine) return;
    if (pMachine->iBatchSize == 1)
    {
        for (i = 1; i <= iN; i++)
        {
            Link(nPrev, sqX[i], pMachine);
            nPrev = sqX[i];
        }
    }
    else
    {  // Batch size > 1
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
    }
}

double TTool::BGetProcessTime(int bID)
{
    TNode* pNode = SGraph.GetID(bID);
    return BGetProcessTime(pNode);
}

double TTool::BGetProcessTime(TNode* pNode)
{
    return pNode->op->fProcessTime;
}

double TTool::BGetReleaseTime(int bID)
{
    TNode* pNode = SGraph.GetID(bID);
    return BGetReleaseTime(pNode);
}

double TTool::BGetReleaseTime(TNode* pNode)
{
    return pNode->op->fReleaseTime;
    /*
      double fPMax = 0;
      TNode *pndTemp;
      for(int i=1;i<=pNode->stkPriorArc.Num();i++)
      {
        pndTemp = pNode->GetPriorNode(i);
        if(pndTemp->op->fReleaseTime>fPMax)
          fPMax = pndTemp->op->fReleaseTime;
      }
      return fPMax;
    */
}

void TTool::BSetActProcessTime(TNode* pNode, double fPTime, int mID)
{  //************* not used
    TNode* pndTemp;
    for (int i = 1; i <= pNode->stkPriorArc.Num(); i++)
    {
        pndTemp = pNode->GetPriorNode(i);
        pndTemp->op->fActProcessTime = fPTime;
        pndTemp->op->mID = mID;
    }
}

void TTool::BSetSetupTime(TNode* pDummy, double fTime)
{
    TNode* pndTemp;
    pDummy->op->fSetupTime = fTime;
    for (int i = 1; i <= pDummy->stkPriorArc.Num(); i++)
    {
        pndTemp = pDummy->GetPriorNode(i);
        if (pndTemp->IsOperation()) pndTemp->op->fSetupTime = fTime;
    }
}

TStatus TTool::BGetStatus(int bID)
{
    // ruturn machine status of the first operation in the batch
    TNode* pNode = SGraph.GetID(bID);
    return BGetStatus(pNode);
}

TStatus TTool::BGetStatus(TNode* pNode)
{
    return pNode->op->sMC_Status;
}

void TTool::BatchLink(int bID1, int bID2)
{
    TStatus sS1, sS2;
    double fPTime, fSetup;
    TNode *pNd1 = SGraph.GetID(bID1), *pNd2 = SGraph.GetID(bID2);
    TMachine* pMachine = pWkCenter->GetID(pNd2->op->mID);
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

/*
void TTool::LinkBatch(int bID1,int bID2,TMachine *pMachine)
{
  TStatus     sS1,sS2;
  double     fPTime,fSetup;
  TNode     *pNd1 = SGraph.GetID(bID1),
            *pNd2 = SGraph.GetID(bID2);
  if(!((pNd1)&&(pNd2)&&(pMachine)))
    return;
  if(pNd1->nID!=0)
  {
    sS1=BatchStatus(pNd1);
    sS2=BatchStatus(pNd2);
    fPTime = BatchProcessTime(pNd1)/pMachine->fSpeed;
    SetActProcessTime(pNd1,fPTime,pMachine->mID);
    fSetup = pMachine->suSetup.Get(sS1,sS2);
    SetSetupTime(pNd2,fSetup);

    //*** add links in SGraph & update Graph
    SGraph.Link(pNd1,pNd2,fPTime+fSetup);
    // update the other links if the machine speed != 100%
    if(pMachine->fSpeed!=1)
    {
      // error
    }
  } else
  { // the first operation
    sS1=pMachine->sInit;
    sS2=BatchStatus(pNd2);
    fSetup = pMachine->suSetup.Get(sS1,sS2);
    SetSetupTime(pNd2,fSetup);
    fPTime = pMachine->fAvailTime+fSetup;
    SGraph.Link(pNd1,pNd2,fPTime);
  }
}
*/

void TTool::Swap(int nID1, int nID2)
{
    // assume that the sequence is there
    // check if nID1 is next to nID2
    TSequence *psqOne = Seq.Gets_oID(nID1), *psqTwo = Seq.Gets_oID(nID2);
    int iLoc1 = psqOne->Locate(nID1), iLoc2 = psqTwo->Locate(nID2), nID1p = (*psqOne)[iLoc1 - 1],
        nID1n = (*psqOne)[iLoc1 + 1], nID2p = (*psqTwo)[iLoc2 - 1], nID2n = (*psqTwo)[iLoc2 + 1];
    TNode *pndOnep = SGraph.GetID(nID1p), *pndOne = SGraph.GetID(nID1), *pndOnen,
          *pndTwop = SGraph.GetID(nID2p), *pndTwo = SGraph.GetID(nID2), *pndTwon, *pndDFrom, *pndDTo;
    double fTemp;

    if (iLoc1 == psqOne->Num())
        pndOnen = 0;
    else
        pndOnen = SGraph.GetID(nID1n);
    if (iLoc2 == psqTwo->Num())
        pndTwon = 0;
    else
        pndTwon = SGraph.GetID(nID2n);
    // remove the links
    pndDFrom = DGraph.GetID(nID1p);
    pndDTo = DGraph.GetID(nID1);
    fTemp = pndDFrom->GetPostWeight(pndDTo);
    if (fTemp < fBigM)
        SGraph.RLink(pndOnep, pndOne, fTemp);
    else
        SGraph.DLink(pndOnep, pndOne);
    if (pndOnen)
    {
        pndDFrom = pndDTo;
        pndDTo = DGraph.GetID(nID1n);
        fTemp = pndDFrom->GetPostWeight(pndDTo);
        if (fTemp < fBigM)
            SGraph.RLink(pndOne, pndOnen, fTemp);
        else
            SGraph.DLink(pndOne, pndOnen);
    }

    pndDFrom = DGraph.GetID(nID2p);
    pndDTo = DGraph.GetID(nID2);
    fTemp = pndDFrom->GetPostWeight(pndDTo);
    if (fTemp < fBigM)
        SGraph.RLink(pndTwop, pndTwo, fTemp);
    else
        SGraph.DLink(pndTwop, pndTwo);
    if (pndTwon)
    {
        pndDFrom = pndDTo;
        pndDTo = DGraph.GetID(nID2n);
        fTemp = pndDFrom->GetPostWeight(pndDTo);
        if (fTemp < fBigM)
            SGraph.RLink(pndTwo, pndTwon, fTemp);
        else
            SGraph.DLink(pndTwo, pndTwon);
    }

    // add the new links
    TMachine *pmcOne = pWkCenter->GetID(psqOne->mID), *pmcTwo = pWkCenter->GetID(psqTwo->mID);
    if (pndOnep != pndTwo)
        Link(pndOnep, pndTwo, pmcOne);
    else
        Link(pndOne, pndTwo, pmcOne);
    if (pndOnen)
        if (pndTwo != pndOnen) Link(pndTwo, pndOnen, pmcOne);
    if (pndTwop != pndOne)
        Link(pndTwop, pndOne, pmcTwo);
    else
        Link(pndTwo, pndOne, pmcTwo);
    if (pndTwon)
        if (pndOne != pndTwon) Link(pndOne, pndTwon, pmcTwo);

    // modify sequence
    (*psqOne)[iLoc1] = nID2;
    (*psqTwo)[iLoc2] = nID1;
    ReCalc();
}

void TTool::Move(int nID1, int nID2)
{
    // assume that the sequence is there
    // check if nID1 is next to nID2
    TSequence *psqOne = Seq.Gets_oID(nID1), *psqTwo = Seq.Gets_oID(nID2);
    int iLoc1 = psqOne->Locate(nID1), iLoc2 = psqTwo->Locate(nID2), nID1p = (*psqOne)[iLoc1 - 1],
        nID1n = (*psqOne)[iLoc1 + 1], nID2p = (*psqTwo)[iLoc2 - 1], nID2n = (*psqTwo)[iLoc2 + 1];
    TNode *pndOnep = SGraph.GetID(nID1p), *pndOne = SGraph.GetID(nID1), *pndOnen,
          *pndTwop = SGraph.GetID(nID2p), *pndTwo = SGraph.GetID(nID2), *pndTwon, *pndDFrom, *pndDTo;
    double fTemp;

    if (iLoc1 == psqOne->Num())
        pndOnen = 0;
    else
        pndOnen = SGraph.GetID(nID1n);
    if (iLoc2 == psqTwo->Num())
        pndTwon = 0;
    else
        pndTwon = SGraph.GetID(nID2n);
    // remove the links
    pndDFrom = DGraph.GetID(nID1p);
    pndDTo = DGraph.GetID(nID1);
    fTemp = pndDFrom->GetPostWeight(pndDTo);
    if (fTemp < fBigM)
        SGraph.RLink(pndOnep, pndOne, fTemp);
    else
        SGraph.DLink(pndOnep, pndOne);
    if (pndOnen)
    {
        pndDFrom = pndDTo;
        pndDTo = DGraph.GetID(nID1n);
        fTemp = pndDFrom->GetPostWeight(pndDTo);
        if (fTemp < fBigM)
            SGraph.RLink(pndOne, pndOnen, fTemp);
        else
            SGraph.DLink(pndOne, pndOnen);
    }

    pndDFrom = DGraph.GetID(nID2p);
    pndDTo = DGraph.GetID(nID2);
    fTemp = pndDFrom->GetPostWeight(pndDTo);
    if (fTemp < fBigM)
        SGraph.RLink(pndTwop, pndTwo, fTemp);
    else
        SGraph.DLink(pndTwop, pndTwo);
    if (pndTwon)
    {
        pndDFrom = pndDTo;
        pndDTo = DGraph.GetID(nID2n);
        fTemp = pndDFrom->GetPostWeight(pndDTo);
        if (fTemp < fBigM)
            SGraph.RLink(pndTwo, pndTwon, fTemp);
        else
            SGraph.DLink(pndTwo, pndTwon);
    }

    // add the new links
    TMachine *pmcOne = pWkCenter->GetID(psqOne->mID), *pmcTwo = pWkCenter->GetID(psqTwo->mID);
    if (pndOnen)
        if (pndOnep != pndTwo)
            Link(pndOnep, pndOnen, pmcOne);
        else
            Link(pndOne, pndOnen, pmcOne);
    if (pndTwop != pndOne) Link(pndTwop, pndTwo, pmcTwo);
    Link(pndTwo, pndOne, pmcTwo);
    if (pndTwon)
        if (pndTwon != pndOne) Link(pndOne, pndTwon, pmcTwo);

    // modify sequence
    psqOne->Remove(iLoc1);
    if (pmcOne == pmcTwo) iLoc2 = psqTwo->Locate(nID2);
    psqTwo->Insert(iLoc2, nID1);
    ReCalc();
}

void TTool::MoveBack(int nID1, int nID2)
{
    // assume that the sequence is there
    // check if nID1 is next to nID2
    TSequence *psqOne = Seq.Gets_oID(nID1), *psqTwo = Seq.Gets_oID(nID2);
    int iLoc1 = psqOne->Locate(nID1), iLoc2 = psqTwo->Locate(nID2), nID1p = (*psqOne)[iLoc1 - 1],
        nID1n = (*psqOne)[iLoc1 + 1], nID2p = (*psqTwo)[iLoc2 - 1], nID2n = (*psqTwo)[iLoc2 + 1];
    TNode *pndOnep = SGraph.GetID(nID1p), *pndOne = SGraph.GetID(nID1), *pndOnen,
          *pndTwop = SGraph.GetID(nID2p), *pndTwo = SGraph.GetID(nID2), *pndTwon, *pndDFrom, *pndDTo;
    double fTemp;

    if (iLoc1 == psqOne->Num())
        pndOnen = 0;
    else
        pndOnen = SGraph.GetID(nID1n);
    if (iLoc2 == psqTwo->Num())
        pndTwon = 0;
    else
        pndTwon = SGraph.GetID(nID2n);
    // remove the links
    pndDFrom = DGraph.GetID(nID1p);
    pndDTo = DGraph.GetID(nID1);
    fTemp = pndDFrom->GetPostWeight(pndDTo);
    if (fTemp < fBigM)
        SGraph.RLink(pndOnep, pndOne, fTemp);
    else
        SGraph.DLink(pndOnep, pndOne);
    if (pndOnen)
    {
        pndDFrom = pndDTo;
        pndDTo = DGraph.GetID(nID1n);
        fTemp = pndDFrom->GetPostWeight(pndDTo);
        if (fTemp < fBigM)
            SGraph.RLink(pndOne, pndOnen, fTemp);
        else
            SGraph.DLink(pndOne, pndOnen);
    }

    pndDFrom = DGraph.GetID(nID2p);
    pndDTo = DGraph.GetID(nID2);
    fTemp = pndDFrom->GetPostWeight(pndDTo);
    if (fTemp < fBigM)
        SGraph.RLink(pndTwop, pndTwo, fTemp);
    else
        SGraph.DLink(pndTwop, pndTwo);
    if (pndTwon)
    {
        pndDFrom = pndDTo;
        pndDTo = DGraph.GetID(nID2n);
        fTemp = pndDFrom->GetPostWeight(pndDTo);
        if (fTemp < fBigM)
            SGraph.RLink(pndTwo, pndTwon, fTemp);
        else
            SGraph.DLink(pndTwo, pndTwon);
    }

    // add the new links
    TMachine *pmcOne = pWkCenter->GetID(psqOne->mID), *pmcTwo = pWkCenter->GetID(psqTwo->mID);
    if (pndTwop != pndOne) Link(pndTwop, pndOne, pmcTwo);
    Link(pndOne, pndTwo, pmcTwo);
    if (pndTwon)
        if (pndTwon != pndOne) Link(pndTwo, pndTwon, pmcTwo);
    if (pndOnen)
        if (pndOnep != pndTwo)
            if (pndOnen != pndTwo)
                Link(pndOnep, pndOnen, pmcOne);
            else
                Link(pndOnep, pndOne, pmcOne);
        else
            Link(pndTwo, pndOnen, pmcTwo);

    // modify sequence
    psqOne->Remove(iLoc1);
    if (pmcOne == pmcTwo) iLoc2 = psqTwo->Locate(nID2);
    psqTwo->Insert(iLoc2, nID2);
    (*psqTwo)[iLoc2] = nID1;
    ReCalc();
}

void TTool::Append(TSequence& sqX)
{
    if (&sqX == 0) return;
    if (sqX.Num() > 0)
    {
        Seq.Append(sqX);
        Link(sqX);
    }
    ReCalc();
}

void TTool::Append(TSequenceList& sqlX)
{
    if (&sqlX == 0) return;
    if (sqlX.Num() > 0)
    {
        TSequence seqI;
        sqlX.Head();
        for (int i = 1; i <= sqlX.Num(); i++)
        {
            seqI = *(TSequence*)(sqlX.Next());  // sqlX[i];
            Seq.Append(seqI);
            Link(seqI);
        }
    }
    // SGraph.ReCalc();
}

void TTool::Insert(TSequence& sqX)
{  // not use
    int i, j, iN;
    TSequence* pSeq;
    Seq.Head();
    for (i = 1; i <= Seq.Num(); i++)
    {
        pSeq = (TSequence*)Seq.Next();
        if (pSeq->mID == sqX.mID) break;
        pSeq = 0;
    }
    if (pSeq)  // insert the sequence
    {
        iN = Seq.Num();
        TMachine* pMachine = pWkCenter->GetID(sqX.mID);
        TSequence &cSeq = *pSeq, sqNew;
        TNode *pHead = SGraph.GetID(sqX[1]), *pNode1, *pNode2;

        // add links
        if (pMachine->iBatchSize == 1)
        {  // do not include link 0->i
            for (i = 1; i < iN; i++)
            {
                Link(sqX[i], sqX[i + 1], pMachine);
            }
            UpdateLink(sqX[iN], pMachine);
        }
        else
        {  // Batch size > 1
        }
        SGraph.ReCalc();

        // find the insertion point in Seq
        pNode1 = SGraph.GetID(cSeq[1]);
        if (pNode1->EST() > pHead->EST())
        {  // sqX ahead cSeq
            sqNew = sqX;
            sqNew << cSeq;
        }
        else
        {
            // find node before head
            for (i = 2; i <= iN; i++)
            {
                pNode2 = SGraph.GetID(cSeq[i]);
                if (pNode2->EST() > pHead->EST())
                {
                    pNode1 = SGraph.GetID(cSeq[--i]);
                    break;
                }
                pNode1 = pNode2;
            }
            // find space, insert at space, check obj()
            while ((pNode1->EST() + pNode1->GetPostWeight(pNode2) == pNode2->EST()) &&
                   (pNode1 != pNode2))  // no space
            {
                pNode1 = pNode2;
                if (i < cSeq.Num()) pNode2 = SGraph.GetID(cSeq[++i]);
            }
            if (pNode1 == pNode2)  // append seq to the end of current seq
            {
                sqNew = cSeq;
                sqNew << sqX;
            }
            else
            {
                for (j = 1; j <= i; j++) sqNew << cSeq[j];
                sqNew << sqX;
                for (j = i + 1; j <= iN; j++) sqNew << cSeq[j];
            }
        }
        // add link(0,Seq[1])
        cSeq = sqNew;
        SGraph = DGraph;
        for (i = 1; i <= iN; i++) Link(Seq[i]);
        SGraph.ReCalc();
    }
    else
    {  // no current sequence on the machine
        Append(sqX);
        return;
    }
}

void TTool::Insert(TSequenceList& sqlX)
{
    for (int i = 1; i <= sqlX.Num(); i++) Insert(sqlX[i]);
}

void TTool::PriorityRule(TIndexFunc pIndex)
{
    int i;
    TOperation *popSelect, *popTemp, *popTemp2;
    TMachine *pmcSelect, *pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp;
    TMcAvail MCAvail;
    TSequence* pSeq;
    int iLastMC = 0, iReCalcLST;
    // iOpDataCalc;
    TRuleReturn* pSelect;
    ClearSequence();
    TDGraph NGraph;
    NGraph.Copy(DGraph);
    NGraph.ReCalc();
    NGraph.Sequence(0);

    // set machine available time MCAvail;
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pmcTemp = pWkCenter->Get(i);
        MCAvail.Set(pmcTemp->mID, pmcTemp, pmcTemp->fAvailTime, 0);
    }
    MCAvail.fData1 = pShop->AvgProcessTime(pWkCenter);
    MCAvail.fData2 = pShop->AvgSetup(pWkCenter);

    // determine active operations
    while (NGraph.stkActive.Num())
    {
        jbTemp.Clear();
        NGraph.stkActive.Head();
        for (i = 1; i <= NGraph.stkActive.Num(); i++)
        {
            popTemp = ((TNode*)NGraph.stkActive.Next())->op;
            popTemp2 = jbTemp.Append(*popTemp);
            pndTemp = NGraph.GetID(popTemp2->oID);
            popTemp2->fReleaseTime = pndTemp->EST();
            popTemp2->fDueDate = pndTemp->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
        }
        pSelect = pIndex(pShop, MCAvail, jbTemp, NGraph, fK1, fK2);
        popSelect = pSelect->op;
        pmcSelect = pSelect->mc;
        iReCalcLST = (pSelect->iDataReCalc) ? 1 : 0;
        delete pSelect;

        //*** put it in the sequence list
        pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
        pNode2 = SGraph.GetID(popSelect->oID);
        Link(pNode1, pNode2, pmcSelect);
        pNode1N = NGraph.GetID(pNode1->nID);
        pNode2N = NGraph.GetID(pNode2->nID);
        NGraph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
        NGraph.Sequence(pNode1N);
        NGraph.Sequence(pNode2N);
        Seq.Append(pmcSelect->mID, popSelect->oID);
        if (iReCalcLST) NGraph.ReCalc();

        // UpdateOperation();
        //*** update MCAvail & find the next active list
        MCAvail.Set(pmcSelect->mID, pmcSelect, pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed,
            popSelect->oID);
    }

    Seq.Head();
    for (i = 1; i <= Seq.Num(); i++)
    {
        pSeq = (TSequence*)Seq.Next();
        pmcTemp = pWkCenter->GetID(pSeq->mID);
        if (pSeq->Num() > 0) UpdateLink((*pSeq)[pSeq->Num()], pmcTemp);
    }
    ReCalc();
}

void TTool::ParameterATC()
{
    double dMax = -fBigM, dMin = fBigM, dSum = 0, fCmax = 0, tau, neu, range;
    int i;
    TOperation* popTemp;
    for (i = 1; i <= pWkCenter->stkTask.Num(); i++)
    {
        popTemp = pWkCenter->GetTask(i);
        dMax = max(dMax, popTemp->fDueDate);
        dMin = min(dMin, popTemp->fDueDate);
        dSum += popTemp->fDueDate;
        fCmax += popTemp->fProcessTime;
    }
    range = (dMax - dMin) / fCmax;
    fK1 = (range <= 0.5) ? (4.5 + range) : (6 - 2 * range);
    tau = 1 - (dSum / pWkCenter->stkTask.Num() / fCmax);
    neu = pShop->AvgSetup(pWkCenter) / pShop->AvgProcessTime(pWkCenter);
    fK2 = tau / 2 * sqrt(neu);
}

void TTool::ParameterTER()
{
    double dMax = -fBigM, dMin = fBigM, dSum = 0, fCmax = 0, range;
    int i;
    TOperation* popTemp;
    for (i = 1; i <= pWkCenter->stkTask.Num(); i++)
    {
        popTemp = pWkCenter->GetTask(i);
        dMax = max(dMax, popTemp->fDueDate);
        dMin = min(dMin, popTemp->fDueDate);
        dSum += popTemp->fDueDate;
        fCmax += popTemp->fProcessTime;
    }
    range = (dMax - dMin) / fCmax;
    fK1 = 4 + 7 * range;
    fK2 = 0.3;
}

void TTool::ATC(double fK1I, double fK2I)
{
    if ((fK1I == 0) && (fK2I == 0))
        ParameterATC();
    else
    {
        fK1 = fK1I;
        fK2 = fK2I;
    }
    PriorityRule(IndexATC);
}

void TTool::TER(double fK1I, double fK2I)
{
    if ((fK1I == 0) && (fK2I == 0))
        ParameterTER();
    else
    {
        fK1 = fK1I;
        fK2 = fK2I;
    }
    PriorityRule(IndexTER);
}

int TTool::IsCriticalOperation(int nID)
{
    TNode *pNode = SGraph.GetID(nID), *pPost;
    int i;
    if ((pNode->IsOperation()) && (pNode->IsOnCriticalPath()))
    {
        for (i = 1; i <= pNode->stkPostArc.Num(); i++)
        {
            pPost = pNode->GetPostNode(i);
            if (pPost->IsSink())
            {
                if (pNode->EST() + pNode->GetPostWeight(i) == pPost->EST()) return 1;
            }
        }
    }
    return 0;
}

void TTool::Critical(TStack& opList)
{  // determine the operations those are critical
    /*
      int i,j;
      TNode *pNode,
            *pPost;
      opList.Clear();
      for(i=1;i<=SGraph.Num();i++)
      {
        //nID = (SGraph.Get(i))->nID;
        //if(IsCriticalOperation(nID))
        //  opList.Push(nID);

        pNode = SGraph.Get(i);
        if((pNode->IsOperation())&&(pNode->IsOnCriticalPath()))
        {
          for(j=1;j<=pNode->stkPostArc.Num();j++)
          {
            pPost = pNode->GetPostNode(j);
            if(pPost->IsSink())
            {
              if(fabs(pNode->EST() + pNode->GetPostWeight(j)==pPost->EST())<fSmall)
              {
                opList.Push(pNode->nID);
                break;
              }
            }
          }
        }

      }
      */
}

void TTool::CriticalAssoWith(TNode* pNode, TStack& opList)
{
    int i, j;
    opList.Clear();
    if (pNode->IsOnCriticalPath() == 0) return;
    // find sequence
    TSequence* pSeq = Seq.Gets_oID(pNode->nID);
    if (pSeq == 0) return;  // sequence not found
    int iLoc = pSeq->Locate(pNode->nID);
    TNode *pMoving, *pPost;
    double fTemp;

    for (i = iLoc - 1; i > 0; i--)
    {
        pMoving = SGraph.GetID((*pSeq)[i]);
        if (pMoving->IsOnCriticalPath())
        {
            for (j = 1; j <= pMoving->stkPostArc.Num(); j++)
            {
                pPost = pMoving->GetPostNode(j);
                if (pPost->IsSink())
                {
                    fTemp = pMoving->EST() + pMoving->GetPostWeight(j) - pPost->EST();
                    if (fabs(fTemp) < fSmall)
                    {
                        opList.Push(pMoving->nID);
                        break;
                    }
                }
            }
        }
        else
            break;
    }
    for (i = iLoc; i <= pSeq->Num(); i++)
    {
        pMoving = SGraph.GetID((*pSeq)[i]);
        if (pMoving->IsOnCriticalPath())
        {
            for (j = 1; j <= pMoving->stkPostArc.Num(); j++)
            {
                pPost = pMoving->GetPostNode(j);
                if (pPost->IsSink())
                {
                    fTemp = pMoving->EST() + pMoving->GetPostWeight(j) - pPost->EST();
                    if (fabs(fTemp) < fSmall)
                    {
                        opList.Push(pMoving->nID);
                        break;
                    }
                }
            }
        }
        else
            break;
    }
}

void TTool::Idle(TStack& opList)
{ /*
  int i;
  TNode *pNode,
        *pNode0=SGraph.GetID(0);
  opList.Clear();
  for(i=1;i<=SGraph.Num();i++)
  {
    pNode = SGraph.Get(i);
    if((pNode->IsOperation())&&(pNode->IsOnCriticalPath()))
    {
      if(fabs(pNode0->EST()+pNode0->GetPostWeight(pNode)==pNode->EST())<fSmall)
        opList.Push(pNode->nID);
    }
  }
  */
}

TNode* TTool::EndCluster(TNode* pNode)
{
    int i;
    if (!pNode->IsOnCriticalPath()) return pNode;
    // find sequence
    TSequence* pSeq = Seq.Gets_oID(pNode->nID);
    if (pSeq == 0) return pNode;  // sequence not found
    int iLoc = pSeq->Locate(pNode->nID);
    TNode *pMoving, *pAfter = SGraph.GetID((*pSeq)[iLoc]);

    for (i = iLoc; i < pSeq->Num(); i++)
    {
        pMoving = pAfter;
        pAfter = SGraph.GetID((*pSeq)[i + 1]);
        if (pMoving->EST() + pMoving->GetPostWeight(pAfter) < pAfter->EST()) return pMoving;
    }
    return pAfter;
}

TNode* TTool::BeginCluster(TNode* pNode)
{
    int i;
    if (!pNode->IsOnCriticalPath()) return pNode;
    // find sequence
    TSequence* pSeq = Seq.Gets_oID(pNode->nID);
    if (pSeq == 0) return pNode;  // sequence not found
    int iLoc = pSeq->Locate(pNode->nID);
    TNode *pMoving, *pNode0 = SGraph.GetID(0), *pBefore = SGraph.GetID((*pSeq)[iLoc]);

    for (i = iLoc; i > 1; i--)
    {
        pMoving = pBefore;
        pBefore = SGraph.GetID((*pSeq)[i - 1]);
        if (pBefore->EST() + pBefore->GetPostWeight(pMoving) < pMoving->EST()) return pMoving;
    }
    return pBefore;
}

void TTool::IdleAssoWith(TNode* pNode, TStack& opList)
{
    int i;
    opList.Clear();
    if (!pNode->IsOnCriticalPath()) return;
    // find sequence
    TSequence* pSeq = Seq.Gets_oID(pNode->nID);
    if (pSeq == 0) return;  // sequence not found
    int iLoc = pSeq->Locate(pNode->nID);
    TNode *pMoving, *pNode0 = SGraph.GetID(0);

    for (i = iLoc - 1; i > 0; i--)
    {
        pMoving = SGraph.GetID((*pSeq)[i]);
        if (pMoving->IsOnCriticalPath())
        {
            if (fabs(pNode0->EST() + pNode0->GetPostWeight(pMoving) - pMoving->EST()) < fSmall)
                opList.Push(pMoving->nID);
        }
        else
            break;
    }
    for (i = iLoc; i <= pSeq->Num(); i++)
    {
        pMoving = SGraph.GetID((*pSeq)[i]);
        if (pMoving->IsOnCriticalPath())
        {
            if (fabs(pNode0->EST() + pNode0->GetPostWeight(pMoving) - pMoving->EST()) < fSmall)
                opList.Push(pMoving->nID);
        }
        else
            break;
    }
}

int TTool::Valid()
{
    int iRtn = SGraph.ReCalc();
    if (iRtn == 2)
        return 0;
    else
        return 1;
}

int TTool::ReCalc(int fl)
{  // return 1 if feasible
    // fl = 1  force recalc
    // fl = 0  ReCalc
    if (fl)
    {
        TSequenceList sql = Seq;
        ClearSequence();
        Append(sql);
    }
    int iRecalc = SGraph.ReCalc();

    if (iRecalc == 2)
        return 0;  // infeasible sequence
    else
        return 1;
}

int TTool::Batch(TStack& oIDList, TMachine* pMC)
{
    int i, j, iN = oIDList.Num();
    double fR = 0, fP = 0, fTemp, fOldLink;
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
            fOldLink = paTemp->fWeight;
            pndTo = paTemp->pndTo;
            fOldLink -= pndTemp->op->fProcessTime;
            fOldLink += fTemp;
            if (pndDummy != pndTo)
            {  // reroute the links through dummy node
                // fTemp2 = paTemp->fWeight; //???
                // fTemp2 -= pndTemp->op->fProcessTime;
                // fTemp2 += fTemp;
                SGraph.Link(pndDummy, pndTo, fOldLink);
                // SGraph.DLink(pndTemp,pndTo);
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

//////////////////////////////////////// T1MC_Center ///////////////////////
T1MC_Center::T1MC_Center(TShop* pShopI,
    TWorkcenter* pWkcI,
    TMachine* pMCI,
    TDGraph* pGraphI,
    double (*ObjI)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight))
{
    SetData(pShopI, pWkcI, pMCI, pGraphI);
    SetObj(ObjI);
}

T1MC_Center::T1MC_Center(TTool& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pWkCenter->Get(1), &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Append(*Tl.Seq.Gets_mID(pMC->mID));
    pSeq = Seq.Get(1);
}

T1MC_Center& T1MC_Center::operator=(TTool& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pWkCenter->Get(1), &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Append(*Tl.Seq.Gets_mID(pMC->mID));
    pSeq = Seq.Get(1);
    return *this;
}

T1MC_Center::T1MC_Center(T1MC_Center& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pMC, &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Append(*Tl.Seq.Gets_mID(pMC->mID));
    pSeq = Seq.Get(1);
}

T1MC_Center& T1MC_Center::operator=(T1MC_Center& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pMC, &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Seq = Tl.Seq;
    pSeq = Seq.Gets_mID(pMC->mID);
    SGraph = Tl.SGraph;
    return *this;
}

void T1MC_Center::SetData(TShop* pShopI, TWorkcenter* pWkcI, TMachine* pMCI, TDGraph* pGrhI)
{
    pShop = pShopI;
    pWkCenter = pWkcI;
    DGraph = *pGrhI;
    SGraph = *pGrhI;
    pMC = pMCI;
    Seq.Clear();
    pSeq = 0;
}

void T1MC_Center::SetData(TShop* pShopI, TWorkcenter* pWkcI, TDGraph* pGrhI)
{
    SetData(pShopI, pWkcI, pWkcI->Get(1), pGrhI);
}

void T1MC_Center::SetSequence(TSequence& sq)
{
    Seq.Clear();
    Seq.Append(sq);
    pSeq = Seq.Get(1);
}

void T1MC_Center::PriorityRule(TIndexFunc pIndex)
{
    int i;
    TOperation *popSelect, *popTemp, *popTemp2;
    TMachine *pmcSelect, *pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp;
    TMcAvail MCAvail;
    int iLastMC = 0, iReCalcLST;
    // iOpDataCalc;
    TRuleReturn* pSelect;
    ClearSequence();
    TDGraph NGraph;
    NGraph.Copy(DGraph);
    NGraph.ReCalc();
    NGraph.Sequence(0);

    // set machine available time MCAvail;
    pmcTemp = pMC;
    MCAvail.Set(pmcTemp->mID, pmcTemp, pmcTemp->fAvailTime, 0);
    MCAvail.fData1 = pShop->AvgProcessTime(pWkCenter);
    MCAvail.fData2 = pShop->AvgSetup(pWkCenter);

    // determine active operations
    while (NGraph.stkActive.Num())
    {
        jbTemp.Clear();
        NGraph.stkActive.Head();
        for (i = 1; i <= NGraph.stkActive.Num(); i++)
        {
            popTemp = ((TNode*)NGraph.stkActive.Next())->op;
            popTemp2 = jbTemp.Append(*popTemp);
            pndTemp = NGraph.GetID(popTemp2->oID);
            popTemp2->fReleaseTime = pndTemp->EST();
            popTemp2->fDueDate = pndTemp->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
        }
        pSelect = pIndex(pShop, MCAvail, jbTemp, NGraph, fK1, fK2);
        popSelect = pSelect->op;
        pmcSelect = pSelect->mc;
        iReCalcLST = (pSelect->iDataReCalc) ? 1 : 0;
        delete pSelect;

        //*** put it in the sequence list
        pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
        pNode2 = SGraph.GetID(popSelect->oID);
        Link(pNode1, pNode2, pmcSelect);
        pNode1N = NGraph.GetID(pNode1->nID);
        pNode2N = NGraph.GetID(pNode2->nID);
        NGraph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
        // NGraph.Reduce(pNode1N);
        NGraph.Sequence(pNode1N);
        NGraph.Sequence(pNode2N);
        Seq.Append(pmcSelect->mID, popSelect->oID);
        if (iReCalcLST) NGraph.ReCalc();  // EST of nodes can't be reduced

        // UpdateOperation();
        //*** update MCAvail & find the next active list
        MCAvail.Set(pmcSelect->mID, pmcSelect, pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed,
            popSelect->oID);
    }

    UpdateLink(pNode2, pNode2->op->fProcessTime / pmcSelect->fSpeed, pNode2->op->fProcessTime);
    ReCalc();
}

TSequence& T1MC_Center::Random()
{
    InitRandom();
    ClearSequence();
    TSequence* pRtn = new TSequence;
    pRtn->mID = pMC->mID;
    TDGraph myGraph = DGraph;
    TStack stkTemp, ActiveList;
    int iN, randSel, nIDSel;
    myGraph.Sequence(0);
    myGraph.ActiveList(ActiveList);
    iN = ActiveList.Num();
    while (iN)
    {
        //*** select active operation
        randSel = ::Random(iN);
        nIDSel = ActiveList[randSel];
        pRtn->Append(nIDSel);
        myGraph.Sequence(nIDSel);
        myGraph.ActiveList(ActiveList);
        iN = ActiveList.Num();
    }
    Append(*pRtn);
    return *pRtn;
}
TSequence& T1MC_Center::TER()
{
    TSequence* pSeq;
    pSeq = new TSequence;
    TTool::TER();
    *pSeq = *(Seq.Get(1));
    return *pSeq;
}
/*
void T1MC_Center::Move(int iFromID,int iToID)
{
  TSequence *pSeq=Seq.Get(1);
  int iFrom = pSeq->Locate(iFromID),
      iTo   = pSeq->Locate(iToID);
  pSeq->Move(iFrom,iTo);
  TSequence sq = *pSeq;
  ClearSequence();
  Append(sq);
  ReCalc();
}


void T1MC_Center::MoveBack(int iFromID,int iToID)
{
  TSequence *pSeq=Seq.Get(1);
  int iFrom = pSeq->Locate(iFromID),
      iTo   = pSeq->Locate(iToID);
  pSeq->Move(iFrom,iTo);
  pSeq->Swap(iTo,iTo+1);
  TSequence sq = *pSeq;
  ClearSequence();
  Append(sq);
  ReCalc();
}
*/

int T1MC_Center::IsMove(TNode* pNode, TNode* pCrit)
{  // check moving benefit
    int i;
    double fTemp;
    TArc* paTo;
    TNode* pndTo;
    for (i = 1; i <= pCrit->stkPostArc.Num(); i++)
    {
        paTo = pCrit->GetPostArcPtr(i);
        pndTo = paTo->pndTo;
        if (pndTo->IsSink())
            if (pCrit->EST() + paTo->fWeight == pndTo->EST())
            {  // crit tail FOUND
                fTemp = pNode->GetPostWeight(pndTo);
                if (fTemp == fBigM) fTemp = 0;
                if (fTemp < paTo->fWeight) return 1;
            }
    }
    return 0;
}

int T1MC_Center::IsMoveBack(TNode* pNode, TNode* pCrit)
{
    // check pNode->EST()<pCrit->EST()?
    if (pNode->EST() >= pCrit->EST()) return 0;
    // check release time of pNode < pCrit?
    TNode* pNodeD = DGraph.GetID(pNode->nID);
    if (pNodeD->EST() < pCrit->EST())
        return 1;
    else
        return 0;
}

int T1MC_Center::CPI()
{  // can change objective
    pSeq = Seq.Gets_mID(pMC->mID);
    if (pSeq == 0) return 0;
    if (pSeq->Num() == 0) return 0;
    T1MC_Center tool;
    int iReturn = 0, iDetermine = 1;
    double fBestObj = Obj(), fTemp;
    TNode *pNode, *pCrit;
    // check backward
    int i, j, iN = pSeq->Num(), nID;
    TStack criticalList;

    // Critical(criticalList);
    // Trace("gr0.tl",SGraph);
    for (i = iN - 1; i >= 1; i--)
    {
        nID = (*pSeq)[i];
        pNode = SGraph.GetID(nID);
        if (pNode->IsOnCriticalPath())
        {
            if (iDetermine)
            {
                CriticalAssoWith(pNode, criticalList);
                iDetermine = 0;
            }
            for (j = 1; j <= criticalList.Num(); j++)
            {
                pCrit = SGraph.GetID(criticalList[j]);
                if (pNode->EST() < pCrit->EST())
                {  // check switching
                    if (IsMove(pNode, pCrit))
                    {
                        tool = *this;
                        // Trace("sq1.tl",tool.Seq);
                        // Trace("gr1.tl",tool.SGraph);
                        tool.Move(pNode->nID, pCrit->nID);
                        // Trace("sq2.tl",tool.Seq);
                        // Trace("gr2.tl",tool.SGraph);
                        // ReCalc();
                        if (tool.Valid())
                        {
                            fTemp = tool.Obj();
                            if (fTemp < fBestObj)
                            {  // move it!
                                *this = tool;
                                fBestObj = fTemp;
                                iDetermine = 1;
                                iReturn = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
            iDetermine = 1;
    }

    // check forward
    iDetermine = 1;
    for (i = 2; i <= iN; i++)
    {
        nID = (*pSeq)[i];
        pNode = SGraph.GetID(nID);
        /*
        if(iDetermine)
        {
          pCrit = BeginCluster(pNode);
          pndEnd = EndCluster(pNode);
          iDetermine = 0;
        }
        if(pNode==pndEnd)
          iDetermine = 1;

        if(pNode!=pCrit)
        {
          // check switching
          tool = *this;
          //Trace("sq1.tl",tool.Seq);
          //Trace("gr1.tl",tool.SGraph);
          tool.MoveBack(pNode->nID,pCrit->nID);
          //Trace("sq2.tl",tool.Seq);
          //Trace("gr2.tl",tool.SGraph);
          if(tool.Valid()) //*****
          {
            fTemp = tool.Obj();
            if(fTemp<fBestObj)
            { // move it!
              *this      = tool;
              fBestObj   = fTemp;
              iDetermine = 1;
              iReturn    = 1;
              break;
            }
          }
        }
        */

        if (pNode->IsOnCriticalPath())
        {
            if (iDetermine)
            {
                IdleAssoWith(pNode, criticalList);
                iDetermine = 0;
            }
            for (j = 1; j <= criticalList.Num(); j++)
            {
                pCrit = SGraph.GetID(criticalList[j]);
                if (pNode->EST() > pCrit->EST())
                {  // check switching
                    tool = *this;
                    // Trace("sq1.tl",tool.Seq);
                    // Trace("gr1.tl",tool.SGraph);
                    tool.MoveBack(pNode->nID, pCrit->nID);
                    // Trace("sq2.tl",tool.Seq);
                    // Trace("gr2.tl",tool.SGraph);
                    if (tool.Valid())  //*****
                    {
                        fTemp = tool.Obj();
                        if (fTemp < fBestObj)
                        {  // move it!
                            *this = tool;
                            fBestObj = fTemp;
                            iDetermine = 1;
                            iReturn = 1;
                            break;
                        }
                    }
                }
            }
        }  // else iDetermine = 1;
    }
    return iReturn;
}

int T1MC_Center::Local(int iLoop)
{
    pSeq = Seq.Get(1);
    int i, j, k, iN = pSeq->Num(), iM, nID;
    TNode* pNode;
    TStack criticalList;
    TSequence sqTemp, sqBest = *pSeq, sqOrg = *pSeq;
    TArrayL2 Set;
    int iSetX = 0, iSetY = 0, iGroup, iOp1, iOp2, iLoc1, iLoc2, iValid;

    ReCalc();
    double fBest = Obj(), fTemp;
    // find sets
    // it is seperated into groups (iSetX groups). the operations
    // in each group can be interchanged but not with
    // operations in other groups.
    for (i = 1; i <= iN; i++)
    {
        nID = (*pSeq)[i];
        pNode = SGraph.GetID(nID);
        if (pNode->IsOnCriticalPath())
        {
            sqTemp << nID;
        }
        else
        {
            if (sqTemp.Num() > 3)
            {  // transfer data to Set
                iSetX++;
                (Set[iSetX]).SetSize(sqTemp.Num() - 1);
                sqTemp.Pop();
                sqTemp.PopFirst();
                iM = sqTemp.Num();
                for (j = 1; j <= iM; j++) Set[iSetX][j] = sqTemp.PopFirst();
            }
            else
                sqTemp.Clear();
        }
    }
    if (sqTemp.Num() > 3)
    {  // transfer data to Set
       // skip the first and the last
        iSetX++;
        (Set[iSetX]).SetSize(sqTemp.Num() - 1);
        sqTemp.Pop();
        sqTemp.PopFirst();
        iM = sqTemp.Num();
        for (j = 1; j <= iM; j++) Set[iSetX][j] = sqTemp.PopFirst();
    }

    // check if interchange can be done
    if (iSetX == 0) return 1;  // optimal;

    for (k = 1; k <= iLoop; k++)
    {
        sqTemp = sqBest;
        // select 2 operations in the same group
        iGroup = ::Random(iSetX);
        iOp1 = ::Random(Set[iGroup].Num() - 1);
        iOp2 = iOp1;
        while (iOp2 == iOp1) iOp2 = ::Random(Set[iGroup].Num() - 1);

        // swapping
        iLoc1 = sqTemp.Locate(Set[iGroup][iOp1]);
        iLoc2 = sqTemp.Locate(Set[iGroup][iOp2]);
        sqTemp.Swap(iLoc1, iLoc2);
        ClearSequence();
        Append(sqTemp);
        iValid = ReCalc();  // ReCalc() & check valid?
        if (iValid)
        {
            fTemp = Obj();
            if (fTemp < fBest)
            {
                fBest = fTemp;
                sqBest = sqTemp;
            }
        }
    }
    ClearSequence();
    Append(sqBest);
    pSeq = Seq.Get(1);
    ReCalc();
    return 1;
}

////////////////////////////////////// TMMC_Center //////////////////////////////
TMMC_Center::TMMC_Center(TMMC_Center& Tl)
{
    pShop = Tl.pShop;
    pWkCenter = Tl.pWkCenter;
    DGraph = Tl.DGraph;
    SGraph = Tl.DGraph;
    ObjFnc = Tl.ObjFnc;
    Append(Tl.Seq);
}

TMMC_Center& TMMC_Center::operator=(TMMC_Center& Tl)
{
    pShop = Tl.pShop;
    pWkCenter = Tl.pWkCenter;
    DGraph = Tl.DGraph;
    SGraph = Tl.DGraph;
    ObjFnc = Tl.ObjFnc;
    Append(Tl.Seq);
    return *this;
}

TMMC_Center& TMMC_Center::operator=(TTool& Tl)
{
    pShop = Tl.pShop;
    pWkCenter = Tl.pWkCenter;
    DGraph = Tl.DGraph;
    SGraph = Tl.DGraph;
    ObjFnc = Tl.ObjFnc;
    Append(Tl.Seq);
    return *this;
}

void TMMC_Center::MyHeuristic()
{
    int i, j, k;
    TNode* pNode;
    TMachine* pMC;
    PriorityRule(IndexOpDueDate);
    T1MC_Center Tl;
    TDGraph Graph;
    TSequence *psqFocus, *psqT;
    // optimize each sequence on each machine
    for (i = 1; i <= NumMachine(); i++)
    {
        Graph = SGraph;
        pMC = pWkCenter->Get(i);
        for (j = 1; j <= Seq.Num(); j++)
        {
            psqT = Seq.Get(j);
            if (psqT->mID == pMC->mID)
                psqFocus = psqT;
            else
            {
                for (k = 1; k <= psqT->Num(); k++)
                {
                    pNode = Graph.GetID((*psqT)[k]);
                    Graph.Reduce(pNode);
                }
            }
        }
        Tl.SetData(pShop, pWkCenter, pMC, &Graph);
        Tl.SetObj(ObjFnc);
        Tl.SetSequence(*psqFocus);
        Tl.ReCalc();
        while (Tl.CPI() == 1)
            ;
        Tl.GetSequence(*psqFocus);
    }
}

int TTool::PriorityRule(TIndexFunc pIndex, TSequenceList& SeqList)
{  // have partial sequence "SeqList"
    int i;
    int iLeft[iMaxMachine];
    TOperation *popSelect, *popTemp, *popTemp2;
    TMachine *pmcSelect, *pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp;
    TMcAvail MCAvail;
    TSequence* pSeq;
    int iLastMC = 0, iReCalcLST;
    // iOpDataCalc;
    TRuleReturn* pSelect;
    ClearSequence();
    TDGraph NGraph;
    NGraph.Copy(DGraph);
    NGraph.ReCalc();
    NGraph.Sequence(0);

    // set machine available time MCAvail;
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pmcTemp = pWkCenter->Get(i);
        MCAvail.Set(pmcTemp->mID, pmcTemp, pmcTemp->fAvailTime, 0);
    }
    MCAvail.fData1 = pShop->AvgProcessTime(pWkCenter);
    MCAvail.fData2 = pShop->AvgSetup(pWkCenter);

    // append partial sequence ****
    TSequenceList sqL = SeqList;
    int iNS = sqL.Num(), iS = 1, iMark = 1, iContinue = 1;
    for (i = 1; i <= iNS; i++) iLeft[i] = 1;
    while (iContinue)
    {
        pndTemp = NGraph.GetID(sqL[iS][1]);
        if (NGraph.stkActive.Member(pndTemp))
        {
            iMark = iS;
            // sequence
            popSelect = pndTemp->op;
            pmcSelect = pWkCenter->GetID(sqL[iS].mID);
            pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
            pNode2 = SGraph.GetID(popSelect->oID);
            Link(pNode1, pNode2, pmcSelect);
            pNode1N = NGraph.GetID(pNode1->nID);
            pNode2N = NGraph.GetID(pNode2->nID);
            NGraph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
            NGraph.Sequence(pNode1N);
            NGraph.Sequence(pNode2N);
            Seq.Append(pmcSelect->mID, popSelect->oID);

            //*** update MCAvail
            MCAvail.Set(pmcSelect->mID, pmcSelect,
                pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed, popSelect->oID);

            sqL[iS].PopFirst();
            iLeft[iS] = sqL[iS].Num();
        }
        else
        {
            iS++;
            if (iS > iNS) iS = 1;
            if (iS == iMark) iContinue = 0;
        }
    }
    for (i = 1; i <= iNS; i++)
        if (iLeft[i] > 0) return 0;

    /*
      for(i=1;i<=SeqList.Num();i++)
      {
        pSeq = SeqList.Get(i);
        pmcSelect = pWkCenter->GetID(pSeq->mID);
        for(j=1;j<=pSeq->Num();j++)
        {
          // select active op
          popSelect = SGraph.GetID((*pSeq)[j])->op;
          pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
          pNode2 = SGraph.GetID(popSelect->oID);
          Link(pNode1,pNode2,pmcSelect);
          pNode1N = NGraph.GetID(pNode1->nID);
          pNode2N = NGraph.GetID(pNode2->nID);
          // check validity -- pNode1N must be in stkActive
          if(!NGraph.stkActive.Member(pNode2N))
            return 0;
          NGraph.RLink(pNode1N,pNode2N,pNode1->GetPostWeight(pNode2));
          NGraph.Sequence(pNode1N);
          NGraph.Sequence(pNode2N);
          Seq.Append(pmcSelect->mID,popSelect->oID);
      //    if(iReCalcLST)
      //      NGraph.ReCalc();

          //*** update MCAvail
          MCAvail.Set(pmcSelect->mID,pmcSelect,pNode2N->EST()+popSelect->fProcessTime/pmcSelect->fSpeed,popSelect->oID);
        }
      }
    */

    // determine active operations
    while (NGraph.stkActive.Num())
    {
        jbTemp.Clear();
        NGraph.stkActive.Head();
        for (i = 1; i <= NGraph.stkActive.Num(); i++)
        {
            popTemp = ((TNode*)NGraph.stkActive.Next())->op;
            popTemp2 = jbTemp.Append(*popTemp);
            pndTemp = NGraph.GetID(popTemp2->oID);
            popTemp2->fReleaseTime = pndTemp->EST();
            popTemp2->fDueDate = pndTemp->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
        }
        pSelect = pIndex(pShop, MCAvail, jbTemp, NGraph, fK1, fK2);
        popSelect = pSelect->op;
        pmcSelect = pSelect->mc;
        iReCalcLST = (pSelect->iDataReCalc) ? 1 : 0;
        delete pSelect;

        //*** put it in the sequence list
        pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
        pNode2 = SGraph.GetID(popSelect->oID);
        Link(pNode1, pNode2, pmcSelect);
        pNode1N = NGraph.GetID(pNode1->nID);
        pNode2N = NGraph.GetID(pNode2->nID);
        NGraph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
        NGraph.Sequence(pNode1N);
        NGraph.Sequence(pNode2N);
        Seq.Append(pmcSelect->mID, popSelect->oID);
        if (iReCalcLST) NGraph.ReCalc();

        // UpdateOperation();
        //*** update MCAvail & find the next active list
        MCAvail.Set(pmcSelect->mID, pmcSelect, pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed,
            popSelect->oID);
    }

    Seq.Head();
    for (i = 1; i <= Seq.Num(); i++)
    {
        pSeq = (TSequence*)Seq.Next();
        pmcTemp = pWkCenter->GetID(pSeq->mID);
        if (pSeq->Num() > 0) UpdateLink((*pSeq)[pSeq->Num()], pmcTemp);
    }
    ReCalc();
    return 1;
}

int TTool::IsFeasible()
{
    TNode* pndTemp;
    ReCalc();
    for (int i = 1; i <= SGraph.NumSink(); i++)
    {
        pndTemp = SGraph.GetSink(i);
        if (pndTemp->EST() == -fBigM) return 0;
    }
    return 1;
}

int TTool::PriorityRule(TIndexFunc pIndex, int, int iLoop)
{
    TSequenceList sqlTemp;
    return PriorityRule(pIndex, sqlTemp, iLoop);
}

int TTool::PriorityRule(TIndexFunc pIndex, TSequenceList& SeqList, int iLoop)
{  // have partial sequence "SeqList"
   // run local search for "iLoop" times after completed
    int i, j;
    int iLeft[iMaxMachine];
    TOperation *popSelect, *popTemp, *popTemp2;
    TMachine *pmcSelect, *pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp;
    TMcAvail MCAvail;
    TSequence* pSeq;
    int iLastMC = 0, iReCalcLST;
    // iOpDataCalc;
    TSequence sqNew,
        sqCheck;  // must interchange nodes in this list with node in sqNew

    TRuleReturn* pSelect;
    ClearSequence();
    TDGraph NGraph;
    NGraph.Copy(DGraph);
    NGraph.ReCalc();
    NGraph.Sequence(0);

    // set machine available time MCAvail;
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pmcTemp = pWkCenter->Get(i);
        MCAvail.Set(pmcTemp->mID, pmcTemp, pmcTemp->fAvailTime, 0);
    }
    MCAvail.fData1 = pShop->AvgProcessTime(pWkCenter);
    MCAvail.fData2 = pShop->AvgSetup(pWkCenter);

    // append partial sequence ****
    TSequenceList sqL = SeqList;
    int iNS = sqL.Num(),  // number of sequence
        iS = 1, iMark = 1, iContinue = 0;
    for (i = 1; i <= iNS; i++)
    {
        iLeft[iS] = sqL[iS].Num();
        if (iLeft[iS] > 0) iContinue = 1;
    }
    while (iContinue)
    {
        pndTemp = NGraph.GetID(sqL[iS][1]);
        if (NGraph.stkActive.Member(pndTemp))
        {
            iMark = iS;
            // sequence
            popSelect = pndTemp->op;
            pmcSelect = pWkCenter->GetID(sqL[iS].mID);
            pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
            pNode2 = SGraph.GetID(popSelect->oID);
            Link(pNode1, pNode2, pmcSelect);
            pNode1N = NGraph.GetID(pNode1->nID);
            pNode2N = NGraph.GetID(pNode2->nID);
            NGraph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
            NGraph.Sequence(pNode1N);
            NGraph.Sequence(pNode2N);
            Seq.Append(pmcSelect->mID, popSelect->oID);

            //*** update MCAvail
            MCAvail.Set(pmcSelect->mID, pmcSelect,
                pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed, popSelect->oID);

            sqL[iS].PopFirst();
            iLeft[iS] = sqL[iS].Num();
        }
        else
        {
            iS++;
            if (iS > iNS) iS = 1;
            if (iS == iMark) iContinue = 0;
        }
    }
    for (i = 1; i <= iNS; i++)
        if (iLeft[i] > 0) return 0;

    // determine active operations
    while (NGraph.stkActive.Num())
    {
        jbTemp.Clear();
        NGraph.stkActive.Head();
        for (i = 1; i <= NGraph.stkActive.Num(); i++)
        {
            popTemp = ((TNode*)NGraph.stkActive.Next())->op;
            popTemp2 = jbTemp.Append(*popTemp);
            pndTemp = NGraph.GetID(popTemp2->oID);
            popTemp2->fReleaseTime = pndTemp->EST();
            popTemp2->fDueDate = pndTemp->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
        }
        pSelect = pIndex(pShop, MCAvail, jbTemp, NGraph, fK1, fK2);
        popSelect = pSelect->op;
        pmcSelect = pSelect->mc;
        iReCalcLST = (pSelect->iDataReCalc) ? 1 : 0;
        delete pSelect;

        //*** put it in the sequence list
        pNode1 = SGraph.GetID(Seq.GetLast(pmcSelect->mID));
        pNode2 = SGraph.GetID(popSelect->oID);
        Link(pNode1, pNode2, pmcSelect);
        pNode1N = NGraph.GetID(pNode1->nID);
        pNode2N = NGraph.GetID(pNode2->nID);
        NGraph.RLink(pNode1N, pNode2N, pNode1->GetPostWeight(pNode2));
        NGraph.Sequence(pNode1N);
        NGraph.Sequence(pNode2N);
        Seq.Append(pmcSelect->mID, popSelect->oID);
        if (iReCalcLST) NGraph.ReCalc();

        // UpdateOperation();
        //*** update MCAvail & find the next active list
        MCAvail.Set(pmcSelect->mID, pmcSelect, pNode2N->EST() + popSelect->fProcessTime / pmcSelect->fSpeed,
            popSelect->oID);

        sqNew << popSelect->oID;
        //  if((SGraph.GetID(popSelect->oID))->IsOnCriticalPath())
        //    sqCheck<<popSelect->oID;
    }

    Seq.Head();
    for (i = 1; i <= Seq.Num(); i++)
    {
        pSeq = (TSequence*)Seq.Next();
        pmcTemp = pWkCenter->GetID(pSeq->mID);
        if (pSeq->Num() > 0) UpdateLink((*pSeq)[pSeq->Num()], pmcTemp);
    }
    ReCalc();

    // determine sqCheck
    sqCheck.Clear();
    for (i = 1; i <= sqNew.Num(); i++)
    {
        if ((SGraph.GetID(sqCheck[i])->IsOnCriticalPath())) sqCheck << sqNew[i];
    }

    // Local(iLoop)
    int nSelect1, nSelect2;
    double fTemp, fBest = Obj();
    if (sqCheck.Num() < 5) iLoop = sqCheck.Num() * sqNew.Num() * 2;
    if ((sqCheck.Num() >= 1) && (sqNew.Num() > 1))
    {
        for (i = 1; i <= iLoop; i++)
        {
            nSelect1 = sqCheck[Random(sqCheck.Num())];
            nSelect2 = nSelect1;
            while (nSelect2 == nSelect1) nSelect2 = sqNew[Random(sqNew.Num())];
            Swap(nSelect1, nSelect2);
            // check feasibility!!
            if (IsFeasible())
            {
                fTemp = Obj();
                if (fTemp < fBest)
                {
                    fBest = fTemp;
                    // determine sqCheck
                    sqCheck.Clear();
                    for (j = 1; j <= sqNew.Num(); j++)
                    {
                        pndTemp = SGraph.GetID(sqNew[j]);
                        if (pndTemp->IsOnCriticalPath()) sqCheck << pndTemp->nID;
                    }
                    if (sqCheck.Num() < 1) break;
                }
                else
                    Swap(nSelect1, nSelect2);
            }
            else
                Swap(nSelect1, nSelect2);
        }
    }
    ReCalc();
    return 1;
}

class TBeamElemt : public TTool
{
protected:
    int IsIn(TSequence& sqY);

public:
    void operator=(TBeamElemt& bm)
    {
        TTool::operator=(bm);
        MCAvail = bm.MCAvail;
        fObj = bm.fObj;
        SeqL = bm.SeqL;
        gActive = bm.gActive;
    };
    void operator=(TTool& tl)
    {
        TTool::operator=(tl);
    };
    int IsSimilar(TSequenceList& sqlY);
    TMcAvail MCAvail;
    double fObj;
    TDGraph gActive;
    TSequenceList SeqL;
};

int TBeamElemt::IsIn(TSequence& sqY)
{
    TSequence* psqX;
    for (int i = 1; i <= SeqL.Num(); i++)
    {
        psqX = SeqL.Get(i);
        if ((*psqX) == sqY) return 1;
    }
    return 0;
}

int TBeamElemt::IsSimilar(TSequenceList& sqlY)
{  // return 1 only if the current sequence, SeqL, is the same as sqlY.
    TSequence* psqX;
    for (int i = 1; i <= SeqL.Num(); i++)
    {
        psqX = sqlY.Get(i);
        if (!IsIn(*psqX)) return 0;
    }
    return 1;
}

void TMMC_Center::BeamSearch(int iBands)
{
    TBeamElemt Best[6], OBest[6], BTemp, BTemp2;
    int i, j, k, l, iN = SGraph.Num(), iContinue = 1, iMCSpeed, iInsert, nID;
    TNode* pndTemp;
    TMachine *pMC, *pmcTemp, *pmcSelect;
    TSequenceList SeqL;
    double fMinAvail, fTemp, fTemp2, fMinComplete;

    ClearSequence();
    // initialize
    Best[1] = *this;
    Best[1].fObj = 1;
    Best[1].gActive = DGraph;
    Best[1].gActive.Sequence(0);
    // find MCAvail & check m/c speed
    iMCSpeed = 1;
    pWkCenter->Head();
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pMC = (TMachine*)pWkCenter->Next();
        Best[1].MCAvail.Set(pMC->mID, pMC, pMC->fAvailTime, 0);
        if (pMC->fSpeed != 1) iMCSpeed = 0;
    }
    for (i = 2; i <= iBands; i++) Best[i].fObj = fBigM;

    //***
    while (iContinue)
    {
        for (j = 1; j <= iBands; j++)
        {
            OBest[j] = Best[j];
            Best[j].fObj = fBigM;
        }
        for (i = 1; i <= iBands; i++)
        {
            if (OBest[i].fObj == fBigM) break;
            // OBest[i].ClearSequence();
            // OBest[i].Append(OBest[i].SeqL);
            iN = OBest[i].gActive.stkActive.Num();
            BTemp2 = OBest[i];
            BTemp2.ClearSequence();
            BTemp2.Append(BTemp.SeqL);  // BTemp2 is a TBeamElemt <- TTool

            // find machine (first avail)
            fMinAvail = fBigM;
            pndTemp = BTemp2.gActive.GetActive(1);
            for (k = 1; k <= pndTemp->op->stkMachine.Num(); k++)
            {
                pmcTemp = pndTemp->op->GetMachine(k);
                if (BTemp2.MCAvail.Get(pmcTemp->mID) < fMinAvail)
                {
                    fMinAvail = BTemp2.MCAvail.Get(pmcTemp->mID);
                    pmcSelect = pmcTemp;
                }
            }

            fMinComplete = fBigM;
            for (j = 1; j <= iN; j++)
            {  // find fMinComplete
                pndTemp = BTemp2.gActive.GetActive(j);
                pndTemp = BTemp2.SGraph.GetID(pndTemp->nID);
                if (pndTemp->IsOperation())
                {
                    fTemp = pndTemp->EST();
                    fTemp2 = max(fMinAvail, fTemp);
                    fTemp2 += pndTemp->op->fProcessTime / pmcTemp->fSpeed;
                    if (fTemp2 < fMinComplete) fMinComplete = fTemp2;
                }
            }

            for (j = 1; j <= iN; j++)
            {
                BTemp = OBest[i];
                pndTemp = OBest[i].gActive.GetActive(j);
                nID = pndTemp->nID;
                pndTemp = BTemp2.SGraph.GetID(nID);
                if ((pndTemp->IsOperation()) && (pndTemp->EST() < fMinComplete))
                {
                    // SeqL  = OBest[i].SeqL;
                    BTemp.SeqL.Append(pmcSelect->mID, pndTemp->nID);
                    BTemp.gActive.Sequence(pndTemp->nID);
                    // BTemp.SetObj(WTeE);
                    if (BTemp.PriorityRule(IndexBetterEDD, BTemp.SeqL))
                    {
                        fTemp = BTemp.Obj();
                        pndTemp = BTemp.SGraph.GetID(nID);
                        // BTemp.SeqL = SeqL;
                        BTemp.MCAvail.Set(pmcSelect->mID, pmcSelect,
                            pndTemp->EST() + pndTemp->op->fProcessTime / pmcSelect->fSpeed, pndTemp->nID);

                        // insert BTemp to Best[]
                        iInsert = 1;
                        if (fTemp < Best[iBands].fObj)
                        {
                            for (k = iBands - 1; k > 0; k--)
                            {
                                if (fTemp > Best[k].fObj) break;
                            }
                            if (iMCSpeed == 1)
                            {  // when machines have same speed, check if fTemp is similar to Best[<k]?
                                for (l = k - 1; l >= 1; l--)
                                {
                                    if (Best[l].fObj < fTemp) break;
                                    if (Best[l].IsSimilar(BTemp.SeqL)) iInsert = 0;
                                }
                            }
                            if (iInsert)
                            {
                                for (l = iBands; l >= k + 2; l--)
                                {
                                    if (Best[l - 1].fObj < fBigM) Best[l] = Best[l - 1];
                                }
                                Best[k + 1] = BTemp;
                                Best[k + 1].fObj = fTemp;
                                // cout<<"Best seq = "<<Best[k+1].SeqL;
                                // Trace("Best.seq",Best[k+1].SeqL);
                            }
                        }
                    }
                }
            }
        }
        // check stop
        if (Best[1].gActive.stkActive.Num() == 0) iContinue = 0;
    }
    Append(Best[1].Seq);
}

void TMMC_Center::BeamSearch(int iBands, int iLoop)
{
    TBeamElemt Best[6], OBest[6], BTemp, BTemp2;
    int i, j, k, l, iN = SGraph.Num(), iContinue = 1, iMCSpeed, iInsert, nID;
    TNode* pndTemp;
    TMachine *pMC, *pmcTemp, *pmcSelect;
    TSequenceList SeqL, sqlTemp;
    double fMinAvail, fTemp, fTemp2, fMinComplete;

    ClearSequence();
    // initialize
    Best[1] = *this;
    Best[1].fObj = 1;
    Best[1].gActive = DGraph;
    Best[1].gActive.Sequence(0);
    // find MCAvail & check m/c speed
    iMCSpeed = 1;
    pWkCenter->Head();
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pMC = (TMachine*)pWkCenter->Next();
        Best[1].MCAvail.Set(pMC->mID, pMC, pMC->fAvailTime, 0);
        if (pMC->fSpeed != 1) iMCSpeed = 0;
    }
    for (i = 2; i <= iBands; i++) Best[i].fObj = fBigM;

    //***
    while (iContinue)
    {
        for (j = 1; j <= iBands; j++)
        {
            OBest[j] = Best[j];
            Best[j].fObj = fBigM;
        }
        for (i = 1; i <= iBands; i++)
        {
            if (OBest[i].fObj == fBigM) break;
            // OBest[i].ClearSequence();
            // OBest[i].Append(OBest[i].SeqL);
            iN = OBest[i].gActive.stkActive.Num();
            BTemp2 = OBest[i];
            BTemp2.ClearSequence();
            BTemp2.Append(BTemp.SeqL);  // BTemp2 is a TBeamElemt <- TTool

            // find machine (first avail)
            fMinAvail = fBigM;
            pndTemp = BTemp2.gActive.GetActive(1);
            for (k = 1; k <= pndTemp->op->stkMachine.Num(); k++)
            {
                pmcTemp = pndTemp->op->GetMachine(k);
                if (BTemp2.MCAvail.Get(pmcTemp->mID) < fMinAvail)
                {
                    fMinAvail = BTemp2.MCAvail.Get(pmcTemp->mID);
                    pmcSelect = pmcTemp;
                }
            }

            fMinComplete = fBigM;
            for (j = 1; j <= iN; j++)
            {  // find fMinComplete
                pndTemp = BTemp2.gActive.GetActive(j);
                pndTemp = BTemp2.SGraph.GetID(pndTemp->nID);
                if (pndTemp->IsOperation())
                {
                    fTemp = pndTemp->EST();
                    fTemp2 = max(fMinAvail, fTemp);
                    fTemp2 += pndTemp->op->fProcessTime / pmcTemp->fSpeed;
                    if (fTemp2 < fMinComplete) fMinComplete = fTemp2;
                }
            }

            for (j = 1; j <= iN; j++)
            {
                BTemp = OBest[i];
                pndTemp = OBest[i].gActive.GetActive(j);
                nID = pndTemp->nID;
                pndTemp = BTemp2.SGraph.GetID(nID);
                if ((pndTemp->IsOperation()) && (pndTemp->EST() < fMinComplete))
                {
                    // SeqL  = OBest[i].SeqL;
                    BTemp.SeqL.Append(pmcSelect->mID, pndTemp->nID);
                    BTemp.gActive.Sequence(pndTemp->nID);
                    // BTemp.SetObj(WTeE);
                    sqlTemp = BTemp.SeqL;
                    if (BTemp.PriorityRule(IndexBetterEDD, BTemp.SeqL, iLoop))
                    {
                        fTemp = BTemp.Obj();
                        pndTemp = BTemp.SGraph.GetID(nID);
                        // BTemp.SeqL = SeqL;
                        BTemp.MCAvail.Set(pmcSelect->mID, pmcSelect,
                            pndTemp->EST() + pndTemp->op->fProcessTime / pmcSelect->fSpeed, pndTemp->nID);

                        // insert BTemp to Best[]
                        iInsert = 1;
                        if (fTemp < Best[iBands].fObj)
                        {
                            for (k = iBands - 1; k > 0; k--)
                            {
                                if (fTemp > Best[k].fObj) break;
                            }
                            if (iMCSpeed == 1)
                            {  // when machines have same speed, check if fTemp is similar to Best[<k]?
                                for (l = k - 1; l >= 1; l--)
                                {
                                    if (Best[l].fObj < fTemp) break;
                                    if (Best[l].IsSimilar(BTemp.SeqL)) iInsert = 0;
                                }
                            }
                            if (iInsert)
                            {
                                for (l = iBands; l >= k + 2; l--)
                                {
                                    if (Best[l - 1].fObj < fBigM) Best[l] = Best[l - 1];
                                }
                                Best[k + 1] = BTemp;
                                Best[k + 1].fObj = fTemp;
                                // cout<<"Best seq = "<<Best[k+1].SeqL;
                                // Trace("Best.seq",Best[k+1].SeqL);
                            }
                        }
                    }
                }
            }
        }
        // check stop
        if (Best[1].gActive.stkActive.Num() == 0) iContinue = 0;
    }
    Append(Best[1].Seq);
}

class TBB_Elemt : public TSequenceList
{
public:
    double fUB;
    TBB_Elemt()
    {
        fUB = 0;
    };
    TBB_Elemt(TSequenceList& sql) : TSequenceList(sql)
    {
        fUB = 0;
    };
    TBB_Elemt& operator=(TSequenceList& sql)
    {
        fUB = 0;
        TSequenceList::operator=(sql);
        return *this;
    };
};

void TMMC_Center::BranchNBound2()
{
    TStackP List;
    TBB_Elemt Best, *pBest, BTemp, BTemp2, *pTemp;
    int i, j, iN, nLast;
    double fUB = fBigM, fLB, fLB2, fTemp, fMin, fMinComplete;
    TMachine *pmcTemp, *pmcSelect;
    TNode *pndTemp, *pndSelect;
    TSequenceList sqL, sqUB;
    TDGraph grTemp, grTemp2;
    TSequence *psqTemp, sqTemp;

    ofstream out("bb.out");

    // init
    ClearSequence();
    Best = Seq;
    Best.fUB = fBigM;
    fLB = Obj();
    fUB = fBigM;
    pTemp = new TBB_Elemt(Best);
    List.InsertSort(pTemp, fLB);
    int iCount = 0;

    while (List.Num() > 0)
    {
        fLB = List.Index(1);
        if (fUB <= fLB)
        {
            cout << "UB<=LB from the list" << endl;
            break;  // solution found!!
        }
        pBest = (TBB_Elemt*)List.PopFirst();

        ClearSequence();
        Append(*pBest);  // already recalc
        grTemp = DGraph;
        grTemp.Sequence(0);
        fMin = fBigM;
        // pmcSelect = first available machine
        for (i = 1; i <= pWkCenter->Num(); i++)
        {
            pmcTemp = pWkCenter->Get(i);
            nLast = pBest->GetLast(pmcTemp->mID);
            if (nLast > 0)
            {
                pndTemp = SGraph.GetID(nLast);
                fTemp = pndTemp->EST() + pndTemp->op->fProcessTime / pmcTemp->fSpeed;
            }
            else
            {
                pndTemp = SGraph.GetID(0);
                fTemp = pmcTemp->fAvailTime;
            }
            if (fTemp < fMin)
            {
                pmcSelect = pmcTemp;
                fMin = fTemp;
                pndSelect = pndTemp;
            }
        }

        for (i = 1; i <= pBest->Num(); i++)
        {  // find active set -- grTemp.stkActive
            psqTemp = pBest->Get(i);
            for (j = 1; j <= psqTemp->Num(); j++) grTemp.Sequence((*psqTemp)[j]);
        }
        grTemp.FindActive();
        iN = grTemp.stkActive.Num();

        cout << ++iCount << ") List:" << List.Num() << "  : LB=" << fLB << "  : UB=" << fUB << endl;

        out << (*(TSequenceList*)pBest);
        out << "UB : " << pBest->fUB;
        out << "    LB : " << fLB << endl;
        out << iCount << ") Branching nodes :";
        for (j = 1; j <= iN; j++) out << (grTemp.GetActive(j))->nID << ", ";
        out << endl;
        //    out<<(*(TSequenceList*)pBest);
        //    out<<iCount<<") List:"<<List.Num()<<"  : LB="<<fLB<<"  : UB="<<fUB<<endl;
        //    out<<(*(TSequenceList*)pBest);
        //    out<<"UB : "<<sqUB;

        // find min completion
        grTemp2 = SGraph;
        fMinComplete = fBigM;
        for (j = 1; j <= iN; j++)
        {
            pndTemp = grTemp2.GetID(grTemp.GetActive(j)->nID);
            if (pndTemp->IsOperation())
            {
                fTemp = pndTemp->EST();
                if (fMin > fTemp) fTemp = fMin;
                fTemp += pndTemp->op->fProcessTime;
                if (fTemp < fMinComplete) fMinComplete = fTemp;
            }
        }

        for (j = 1; j <= iN; j++)
        {  // branching
            BTemp = *pBest;
            pndTemp = grTemp2.GetID(grTemp.GetActive(j)->nID);
            if ((pndTemp->IsOperation()) && (pndTemp->EST() < fMinComplete))
            {  // if it is an operation, else skip
                BTemp.Append(pmcSelect->mID, pndTemp->nID);
                ClearSequence();
                Append(BTemp);
                fLB2 = Obj();
                BTemp2 = BTemp;
                if (PriorityRule(IndexOpDueDate, BTemp2))
                {
                    BTemp.fUB = Obj();
                    out << "branch : " << (TSequenceList)BTemp;
                    out << "=====> LB:" << fLB2 << " / UB:" << BTemp.fUB << endl;
                    if (BTemp.fUB < fUB)
                    {
                        fUB = BTemp.fUB;
                        sqUB = Seq;
                        if (fUB <= fLB)
                        {
                            cout << "Find UB that <= LB" << endl;
                            delete pBest;
                            break;
                        }
                    }
                    if (fLB2 < fUB)
                    {
                        pTemp = new TBB_Elemt(BTemp);
                        List.InsertSort(pTemp, fLB2);
                    }
                }
            }
        }
        if (j > iN)
            delete pBest;
        else
            break;
    }
    ClearSequence();
    Append(sqUB);

    // remove List
    for (i = 1; i <= List.Num(); i++)
    {
        pBest = (TBB_Elemt*)List.Pop();
        delete pBest;
    }
}

void TMMC_Center::BranchNBound()
{
    TStackP List;
    TBeamElemt Best, *pBest, BTemp, *pTemp;
    int i, j, k, iN, nID;
    double fMinAvail, fUB = fBigM, fLB, fLB2, fTemp;
    TMachine *pmcTemp, *pmcSelect, *pMC;
    TNode* pndTemp;
    TSequenceList sqL, sqUB;

    // init
    ClearSequence();
    Best = *this;
    Best.gActive = DGraph;
    Best.gActive.Sequence(0);
    Best.fObj = fBigM;  // UB
    pWkCenter->Head();
    for (i = 1; i <= pWkCenter->Num(); i++)
    {
        pMC = (TMachine*)pWkCenter->Next();
        Best.MCAvail.Set(pMC->mID, pMC, pMC->fAvailTime, 0);
    }
    fLB = Best.Obj();
    pTemp = new TBeamElemt(Best);
    List.InsertSort(pTemp, fLB);
    int iCount = 0;

    while (1)
    {
        fLB = List.Index(1);
        if (fUB <= fLB) break;  // solution found!!
        pBest = (TBeamElemt*)List.PopFirst();
        cout << ++iCount << ") List:" << List.Num() << "  : LB=" << fLB << "  : UB=" << fUB << endl;
        iN = pBest->gActive.stkActive.Num();
        cout << pBest->SeqL;
        for (j = 1; j <= iN; j++)
        {
            BTemp = *pBest;
            pndTemp = pBest->gActive.GetActive(j);
            nID = pndTemp->nID;
            if (pndTemp->IsOperation())
            {
                fMinAvail = fBigM;
                for (k = 1; k <= pndTemp->op->stkMachine.Num(); k++)
                {
                    pmcTemp = pndTemp->op->GetMachine(k);
                    if (BTemp.MCAvail.Get(pmcTemp->mID) < fMinAvail)
                    {
                        fMinAvail = BTemp.MCAvail.Get(pmcTemp->mID);
                        pmcSelect = pmcTemp;
                    }
                }
                BTemp.SeqL.Append(pmcSelect->mID, pndTemp->nID);
                BTemp.gActive.Sequence(pndTemp->nID);
                sqL = BTemp.SeqL;
                BTemp.ClearSequence();
                BTemp.Append(sqL);
                fLB2 = BTemp.Obj();
                if (BTemp.PriorityRule(IndexTER, BTemp.SeqL))
                {
                    fTemp = BTemp.Obj();
                    BTemp.fObj = fTemp;
                    if (fTemp < fUB)
                    {
                        fUB = fTemp;
                        sqUB = BTemp.Seq;
                        if (fUB <= fLB)
                        {
                            delete pBest;
                            break;
                        }
                    }
                    pndTemp = BTemp.SGraph.GetID(nID);
                    BTemp.MCAvail.Set(pmcSelect->mID, pmcSelect,
                        pndTemp->EST() + pndTemp->op->fProcessTime / pmcSelect->fSpeed, pndTemp->nID);
                    pTemp = new TBeamElemt(BTemp);
                    if (fLB2 < fUB) List.InsertSort(pTemp, fLB2);
                }
            }
        }
        delete pBest;
    }
    Append(sqUB);

    // remove List
    for (i = 1; i <= List.Num(); i++)
    {
        pBest = (TBeamElemt*)List.Pop();
        delete pBest;
    }
}

//***************************** Batch Machine *********************************
TBMC_Center::TBMC_Center(TBMC_Center& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pMC, &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Append(*Tl.Seq.Gets_mID(pMC->mID));
    pSeq = Seq.Get(1);
}

TBMC_Center& TBMC_Center::operator=(TBMC_Center& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pMC, &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Seq = Tl.Seq;
    pSeq = Seq.Gets_mID(pMC->mID);
    SGraph = Tl.SGraph;
    return *this;
}

TBMC_Center& TBMC_Center::operator=(TTool& Tl)
{
    SetData(Tl.pShop, Tl.pWkCenter, Tl.pWkCenter->Get(1), &Tl.DGraph);
    SetObj(Tl.ObjFnc);
    Append(*Tl.Seq.Gets_mID(pMC->mID));
    pSeq = Seq.Get(1);
    return *this;
}

void TBMC_Center::PriorityRule(TIndexFunc pIndex)
{
    int i, j;
    TOperation *popTemp, *popTemp2, *popTemp3;
    TMachine* pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp, jbTemp2;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp, *pNodeN;
    TMcAvail MCAvail;
    int iLastMC = 0, iReCalcLST, iNode1 = 0, iNode2;
    // iOpDataCalc;
    double fProcessT, fProcessTO = pMC->fAvailTime, fSetup;
    TStack stkBTemp;
    TRuleReturn* pSelect;
    TStatus sBatchStatus, sS1, sS2;

    ClearSequence();
    TDGraph NGraph;
    NGraph.Copy(DGraph);
    NGraph.ReCalc();
    NGraph.Sequence(0);

    // set machine available time MCAvail;
    pmcTemp = pWkCenter->Get(1);
    MCAvail.Set(pMC->mID, pmcTemp, pMC->fAvailTime, 0);
    MCAvail.fData1 = pShop->AvgProcessTime(pWkCenter);
    MCAvail.fData2 = pShop->AvgSetup(pWkCenter);

    // determine active operations
    while (NGraph.stkActive.Num())
    {
        jbTemp.Clear();
        jbTemp2.Clear();
        NGraph.stkActive.Head();
        for (i = 1; i <= NGraph.stkActive.Num(); i++)
        {
            popTemp = ((TNode*)NGraph.stkActive.Next())->op;
            popTemp2 = jbTemp.Append(*popTemp);
            popTemp3 = jbTemp2.Append(*popTemp);
            pndTemp = NGraph.GetID(popTemp2->oID);
            popTemp2->fReleaseTime = pndTemp->EST();
            popTemp2->fDueDate = pndTemp->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
            popTemp3->fReleaseTime = popTemp2->fReleaseTime;
            popTemp3->fDueDate = popTemp2->fDueDate;
            popTemp3->fWeight = popTemp2->fWeight;
            popTemp3->fEWeight = popTemp2->fEWeight;
            popTemp3->pJob = popTemp2->pJob;
        }
        fProcessT = 0;

        // select the first oper in the new batch
        pSelect = pIndex(pShop, MCAvail, jbTemp2, NGraph, fK1, fK2);
        stkBTemp.Push(pSelect->op->oID);
        iReCalcLST = (pSelect->iDataReCalc) ? 1 : 0;
        fProcessT = pSelect->op->fProcessTime;
        sBatchStatus = pSelect->op->sMC_Status;
        delete pSelect;

        jbTemp2.Clear();
        for (j = 1; j <= jbTemp.Num(); j++)
        {  // screen operations that can be processed in the same batch
            popTemp = jbTemp.Get(j);
            if (popTemp->sMC_Status == sBatchStatus)
                if (!stkBTemp.Member(popTemp->oID))
                {
                    popTemp2 = jbTemp2.Append(*popTemp);
                    popTemp2->pJob = popTemp->pJob;
                }
        }
        while ((stkBTemp.Num() < pMC->iBatchSize) && (jbTemp2.Num() > 0))
        {
            // find other oper in the same batch -- have same MC_Status
            pSelect = pIndex(pShop, MCAvail, jbTemp2, NGraph, fK1, fK2);
            stkBTemp.Push(pSelect->op->oID);
            fProcessT = max(fProcessT, pSelect->op->fProcessTime);
            delete pSelect;

            jbTemp2.Clear();
            for (j = 1; j <= jbTemp.Num(); j++)
            {  // screen operations that can be processed in the same batch
                popTemp = jbTemp.Get(j);
                if (popTemp->sMC_Status == sBatchStatus)
                    if (!stkBTemp.Member(popTemp->oID))
                    {
                        popTemp2 = jbTemp2.Append(*popTemp);
                        popTemp2->pJob = popTemp->pJob;
                    }
            }
        }

        /*
            for(i=1;((i<=pMC->iBatchSize)&&(jbTemp.Num()>0));i++)
            {
              jbTemp2 = jbTemp;
              for(j=1;j<=jbTemp.Num();j++)
                (jbTemp2.Get(j))->pJob = (jbTemp.Get(j))->pJob;
              pSelect = pIndex(pShop,MCAvail,jbTemp2,NGraph,fK1,fK2);
              jbTemp.Head();
              for(j=1;j<=jbTemp.Num();j++)
              {
                if(((TOperation*)jbTemp.Current())->oID==pSelect->op->oID)
                  break;
                jbTemp.Next();
              }
              jbTemp.Delete();
              stkBTemp.Push(pSelect->op->oID);
              fProcessT = max(fProcessT,pSelect->op->fProcessTime);
              iReCalcLST = (pSelect->iDataReCalc)?1:0;
              delete pSelect;
            }*/

        iNode2 = Batch(stkBTemp, pMC);
        pNode1 = SGraph.GetID(iNode1);
        pNode2 = SGraph.GetID(iNode2);
        if (iNode1 == 0)
            sS1 = pMC->sInit;
        else
            sS1 = pNode1->op->sMC_Status;
        sS2 = pNode2->op->sMC_Status;
        fSetup = pMC->suSetup.Get(sS1, sS2);
        SGraph.Link(pNode1, pNode2, fProcessTO + fSetup);
        // SGraph.Sequence(pNode1);
        fProcessTO = fProcessT;
        pNodeN = NGraph.AddDummy();
        // link post arcs from pNode2 (dummy)
        for (j = 1; j <= pNode2->stkPostArc.Num(); j++)
        {
            pNode2N = NGraph.GetID(pNode2->GetPostNode(j)->nID);
            NGraph.Link(pNodeN, pNode2N, pNode2->GetPostWeight(j));
        }

        for (i = 1; i <= pNode2->stkPriorArc.Num(); i++)
        {
            pNode1N = NGraph.GetID((pNode2->GetPriorNode(i))->nID);
            if (pNode1N->nID != iNode1)
            {
                NGraph.Link(pNode1N, pNodeN, 0);
                NGraph.Sequence(pNode1N);
                Seq.Append(pMC->mID, pNode1N->nID);
            }
        }
        NGraph.Sequence(pNodeN);
        Seq.Append(pMC->mID, 0);
        iNode1 = iNode2;

        if (iReCalcLST) NGraph.ReCalc();

        // UpdateOperation();
        //*** update MCAvail & find the next active list
        MCAvail.Set(pMC->mID, pMC, pNodeN->EST() + fProcessTO, 0);  // last op is set to 0
    }

    ReCalc();
}

void TBMC_Center::PriorityRuleS(TIndexFunc pIndex)
{  // multifamily & setup
    int i, j;
    TOperation* popTemp;
    TOperation* popTemp2;
    TOperation* popTemp3;
    TMachine* pmcTemp;
    TWorkcenter wkTemp;
    TJob jbTemp, jbTemp2;
    TNode *pNode1, *pNode2, *pNode1N, *pNode2N, *pndTemp, *pNodeN;
    TMcAvail MCAvail;
    int iLastMC = 0, iReCalcLST, iNode1 = 0, iNode2;
    // iOpDataCalc;
    double fProcessT, fProcessTO = pMC->fAvailTime, fSetup;
    TStack stkBTemp;
    TRuleReturn* pSelect;
    TStatus sBatchStatus, sS1, sS2, sBatch;

    ClearSequence();
    TDGraph NGraph;
    NGraph.Copy(DGraph);
    NGraph.ReCalc();
    NGraph.Sequence(0);

    // set machine available time MCAvail;
    pmcTemp = pWkCenter->Get(1);
    MCAvail.Set(pMC->mID, pmcTemp, pMC->fAvailTime, 0);
    MCAvail.fData1 = pShop->AvgProcessTime(pWkCenter);
    MCAvail.fData2 = pShop->AvgSetup(pWkCenter);

    // determine active operations
    while (NGraph.stkActive.Num())
    {
        jbTemp.Clear();
        jbTemp2.Clear();
        NGraph.stkActive.Head();
        for (i = 1; i <= NGraph.stkActive.Num(); i++)
        {
            popTemp = ((TNode*)NGraph.stkActive.Next())->op;
            popTemp2 = jbTemp.Append(*popTemp);
            popTemp3 = jbTemp2.Append(*popTemp);
            pndTemp = NGraph.GetID(popTemp2->oID);
            popTemp2->fReleaseTime = pndTemp->EST();
            popTemp2->fDueDate = pndTemp->DueDate();
            popTemp2->fWeight = popTemp->pJob->fWeight;
            popTemp2->fEWeight = popTemp->pJob->fEWeight;
            popTemp2->pJob = popTemp->pJob;
            popTemp3->fReleaseTime = popTemp2->fReleaseTime;
            popTemp3->fDueDate = popTemp2->fDueDate;
            popTemp3->fWeight = popTemp2->fWeight;
            popTemp3->fEWeight = popTemp2->fEWeight;
            popTemp3->pJob = popTemp2->pJob;
        }
        fProcessT = 0;

        // select the first oper in the new batch
        pSelect = pIndex(pShop, MCAvail, jbTemp2, NGraph, fK1, fK2);
        sBatch = pSelect->op->sMC_Status;
        stkBTemp.Push(pSelect->op->oID);
        iReCalcLST = (pSelect->iDataReCalc) ? 1 : 0;
        fProcessT = pSelect->op->fProcessTime;
        sBatchStatus = pSelect->op->sMC_Status;
        delete pSelect;

        jbTemp2.Clear();
        for (j = 1; j <= jbTemp.Num(); j++)
        {  // extract the sequeced operation out
            popTemp = jbTemp.Get(j);
            if (!stkBTemp.Member(popTemp->oID))
            {
                popTemp2 = jbTemp2.Append(*popTemp);
                popTemp2->pJob = popTemp->pJob;
            }
        }

        while ((stkBTemp.Num() < pMC->iBatchSize) && (jbTemp2.Num() > 0))
        {
            // find other oper in the same batch -- have same MC_Status
            pSelect = pIndex(pShop, MCAvail, jbTemp2, NGraph, fK1, fK2);
            if (pSelect->op->sMC_Status == sBatch)
            {
                stkBTemp.Push(pSelect->op->oID);
                fProcessT = max(fProcessT, pSelect->op->fProcessTime);
                delete pSelect;
                jbTemp2.Clear();
                for (j = 1; j <= jbTemp.Num(); j++)
                {  // screen operations that can be processed in the same batch
                    popTemp = jbTemp.Get(j);
                    if (!stkBTemp.Member(popTemp->oID))
                    {
                        popTemp2 = jbTemp2.Append(*popTemp);
                        popTemp2->pJob = popTemp->pJob;
                    }
                }
            }
            else
            {
                delete pSelect;
                break;
            }
        }

        iNode2 = Batch(stkBTemp, pMC);
        pNode1 = SGraph.GetID(iNode1);
        pNode2 = SGraph.GetID(iNode2);
        if (iNode1 == 0)
            sS1 = pMC->sInit;
        else
            sS1 = pNode1->op->sMC_Status;
        sS2 = pNode2->op->sMC_Status;
        fSetup = pMC->suSetup.Get(sS1, sS2);
        SGraph.Link(pNode1, pNode2, fProcessTO + fSetup);
        // SGraph.Sequence(pNode1);
        fProcessTO = fProcessT;
        pNodeN = NGraph.AddDummy();
        // link post arcs from pNode2 (dummy)
        for (j = 1; j <= pNode2->stkPostArc.Num(); j++)
        {
            pNode2N = NGraph.GetID(pNode2->GetPostNode(j)->nID);
            NGraph.Link(pNodeN, pNode2N, pNode2->GetPostWeight(j));
        }

        for (i = 1; i <= pNode2->stkPriorArc.Num(); i++)
        {
            pNode1N = NGraph.GetID((pNode2->GetPriorNode(i))->nID);
            if (pNode1N->nID != iNode1)
            {
                NGraph.Link(pNode1N, pNodeN, 0);
                NGraph.Sequence(pNode1N);
                Seq.Append(pMC->mID, pNode1N->nID);
            }
        }
        NGraph.Sequence(pNodeN);
        Seq.Append(pMC->mID, 0);
        iNode1 = iNode2;

        if (iReCalcLST) NGraph.ReCalc();

        // UpdateOperation();
        //*** update MCAvail & find the next active list
        MCAvail.Set(pMC->mID, pMC, pNodeN->EST() + fProcessTO, 0);  // last op is set to 0
    }

    ReCalc();
}

void TBMC_Center::Batching()
{
    TSequence sq = *Seq.Get(1);
    Batching(sq);
    sq = *Seq.Get(1);
    ClearSequence();
    Append(sq);
}

void TBMC_Center::Batching(TSequence& sq)
{
    // rebatch current sequence
    // remove 0 from the seq
    int i, j, k, l, iN = sq.Num(), iID, iTemp;
    double fTemp, fTemp1, fTemp2, fObj, fBestObjStepJ;
    TJob* pJob;
    TNode *pndTemp, *pnTo;
    TArc* paTo;
    TSequence sqOld = *Seq.Get(1);
    double fObjOld = Obj();

    sq.Remove0();
    // SetArray
    int iNumOp = sq.Num(), iNumJob = DGraph.NumSink();
    TArrayF p(iNumOp + 1),               // process time
        r(iNumOp + 1),                   // op release time
        w(iNumJob + 1),                  // tardy weight
        h(iNumJob + 1),                  // early weight
        d(iNumJob + 1);                  // job due date
    TArrayF2 s(iNumOp + 1, iNumOp + 1),  // setup time
        dp(iNumOp + 1, iNumOp + 1),      // dependent time constraints = -BigM if not persist
        q(iNumOp + 1, iNumJob + 1);  // op remaining processing time q[1][2] = remainig time of op 1 to job 2
    TArrayL ID(iNumOp + 1),          // index of opID
        sStatus(iNumOp + 1);
    DGraph.ReCalc();
    // set default value
    for (i = 1; i <= iNumOp; i++)
    {
        ID[sq[i]] = i;
        for (j = 1; j <= iNumOp; j++)
        {
            dp[i][j] = -fBigM;
            q[i][j] = -fBigM;
        }
    }
    // set job info
    for (i = 1; i <= iNumJob; i++)
    {
        pndTemp = DGraph.GetSink(i);
        pJob = pShop->GetJob(pndTemp->nID);
        w[i] = pJob->fWeight;
        h[i] = pJob->fEWeight;
        d[i] = pJob->fDueDate;
    }
    // set operation info
    for (i = 1; i <= iNumOp; i++)
    {
        iID = sq[i];
        pndTemp = DGraph.GetID(iID);
        p[i] = pndTemp->op->fProcessTime;
        r[i] = pndTemp->EST();
        sStatus[i] = pndTemp->op->sMC_Status;
        for (j = 1; j <= pndTemp->stkPostArc.Num(); j++)
        {
            paTo = pndTemp->GetPostArcPtr(j);
            pnTo = paTo->pndTo;
            if (pnTo->IsOperation())
            {  // add to dp
                iTemp = ID[pnTo->nID];
                dp[i][iTemp] = paTo->fWeight;
            }
            else
            {                                        // if(pnTo->IsSink())
                iTemp = nMaxNode + pnTo->nID;        // 327677
                q[i][iTemp] = paTo->fWeight - p[i];  ////////
            }
        }
    }

    // dynamic prog
    int iNumBatch = pMC->iBatchSize;
    TArrayF f(iNumOp + 1),  // f(i)
        arr_fTemp;
    TArrayL f_BatchSize(iNumOp + 1);
    TArrayF2 c(iNumJob + 1, iNumOp + 1),  // c(job,j)
        ccc(iNumBatch + 1, iNumJob + 1),  // buffer for ccc(batch size,job,j)
        t(iNumOp + 1, iNumOp + 1);        // starting time of operation i in step j, t[i][j]
    TArrayF fff(iNumBatch + 1),           // buffer for f(batch size,job,j)
        obj(iNumBatch + 1),               // objective of different batch size
        ttt(iNumOp + 1);                  // buffer for t[i][j]

    // ofstream ofile("batch.out");
    // init
    f[0] = 0;
    f_BatchSize[0] = 0;
    t[0][0] = 0;
    for (i = 1; i <= iNumJob; i++)
    {
        pndTemp = DGraph.GetSink(i);
        c[i][0] = pndTemp->EST();
        // c[i][0] = 0;
    }

    // determine f & c
    for (j = 1; j <= iNumOp; j++)
    {
        // ofile<<"STEP j = "<<j<<endl;
        fBestObjStepJ = fBigM;
        for (i = 1; i <= iNumBatch; i++)
        {
            if (j - i >= 0)
            {
                // determine ttt(k)
                for (k = 1; k <= j - i; k++)
                {
                    ttt[k] = t[k][j - i];
                }

                fTemp = f[j - i];
                for (k = j - i + 1; k <= j; k++)
                {
                    if (r[k] > fTemp) fTemp = r[k];
                    for (l = 1; l < j - i; l++)
                    {
                        if (ttt[l] + dp[l][k] > fTemp) fTemp = ttt[l] + dp[l][k];
                    }
                }
                for (k = j - i + 1; k <= j; k++) ttt[k] = fTemp;

                for (k = j + 1; k <= iNumOp; k++)
                {
                    ttt[k] = r[k];
                    for (l = 1; l < k; l++)
                    {
                        if (ttt[l] + dp[l][k] > ttt[k]) ttt[k] = ttt[l] + dp[l][k];
                    }
                }

                fTemp1 = f[j - i];
                fTemp2 = 0;
                for (k = j; k >= j - i + 1; k--)
                {
                    if (ttt[k] > fTemp1) fTemp1 = ttt[k];
                    if (p[k] > fTemp2) fTemp2 = p[k];
                }
                fff[i] = fTemp1 + fTemp2;
                // ofile<<"fff["<<i<<"] = "<<fff[i]<<endl; //***

                //
                fTemp = 0;
                for (k = j - i + 1; k < j; k++)
                {
                    // iTemp = j-k;
                    fTemp += (1 / (dp[k][j]));
                    if (sStatus[j] != sStatus[k])
                    {  // break batch if dif status
                        fTemp = fBigM;
                        break;
                    }
                }
                if (fTemp > 0.0001)
                {
                    c[k][j] = fBigM;
                    fObj = fBigM;
                    // ccc[i][k] = fBigM;
                }
                else
                {
                    for (k = 1; k <= iNumJob; k++)
                    {
                        ccc[i][k] = c[k][j - i];
                        fTemp1 = -fBigM;
                        for (l = j - i + 1; l <= j; l++)
                        {
                            if (q[l][k] > fTemp1) fTemp1 = q[l][k];
                        }
                        fTemp2 = fff[i] + fTemp1;
                        if ((fTemp2 > ccc[i][k]) && (fTemp1 >= 0)) ccc[i][k] = fTemp2;
                        for (l = j + 1; l <= iNumOp; l++)
                        {
                            if (ttt[l] + p[l] + q[l][k] > ccc[i][k]) ccc[i][k] = ttt[l] + p[l] + q[l][k];
                        }
                        // ofile<<"ccc["<<i<<"]["<<k<<"] = "<<ccc[i][k]<<endl; //***
                    }
                    d[0] = 0;
                    arr_fTemp = ccc[i];
                    arr_fTemp[0] = 0;
                    w[0] = 0;
                    h[0] = 0;
                    fObj = ObjFnc(iNumJob + 1, d, arr_fTemp, w, h);
                }

                // ofile<<"objective = "<<fObj<<endl; //***
                if (fObj < fBestObjStepJ)
                {
                    fBestObjStepJ = fObj;
                    f_BatchSize[j] = i;
                }
            }  // end if (j-i>=0)
        }      // end i

        iTemp = f_BatchSize[j];
        f[j] = fff[iTemp];
        // ofile<<"f["<<j<<"] = "<<f[j]<<endl; //***
        for (k = 1; k <= iNumJob; k++)
        {
            c[k][j] = ccc[iTemp][k];
            // ofile<<"c["<<k<<"]["<<j<<"] = "<<c[k][j]<<endl; //***
        }
        fTemp = 0;
        for (k = j - f_BatchSize[j] + 1; k <= j; k++)
            if (p[k] > fTemp) fTemp = p[k];
        for (k = 1; k <= j; k++)
        {
            if (k <= (j - f_BatchSize[j]))
                t[k][j] = t[k][j - 1];
            else
                t[k][j] = f[j] - fTemp;
            // ofile<<"t["<<k<<"]["<<j<<"] = "<<t[k][j]<<endl;   //***
        }
        // ofile<<" == i = "<<f_BatchSize[j]<<" ==\n"<<endl; //***

        // update r
        /*
        for(i=j+1;i<=iNumOp;i++)
        {
          if((f[j]+dp[j][i])>r[i])
            r[i] = f[j]+dp[j][i];
        }
        */

    }  // end j

    // add 0 to the seq
    j = iNumOp;
    while (j > 0)
    {
        iTemp = f_BatchSize[j];
        sq[j];
        sq.Insert(0);
        j -= iTemp;
    }

    ClearSequence();
    Append(sq);
    ReCalc();
    if (Obj() > fObjOld)
    {
        ClearSequence();
        Append(sqOld);
        ReCalc();
    }
}

void TBMC_Center::TER()
{
    TSequence seqTemp;
    ClearSequence();
    T1MC_Center smc(*((TTool*)this));
    smc.PriorityRule(IndexTER);
    smc.GetSequence(seqTemp);
    Batching(seqTemp);
    Append(seqTemp);
    ReCalc();
}

void TBMC_Center::CPI()
{
    // run after a feasible sequence is loaded
    TDGraph ghTemp = SGraph;
    TSequence sqDummy;
    TArrayL2 BatchList(SGraph.NumDummy() + 2, pMC->iBatchSize + 1);
    TArrayL BatchSize(SGraph.NumDummy() + 2), BatchIndex(SGraph.NumDummy() + 4);
    int i, j, k, iTemp;
    double fTemp;
    TNode *pndTemp, *pndOp_S, *pndOp_G, *pndTo_S, *pndBatch_S, *pndBatch_G, *pndBatchTo_G;
    //*pndTo_G,*pndBatchTo_S,
    TArc* paTo_S;

    pSeq = Seq.Get(1);

    // remove links between dummies
    for (i = 1; i <= ghTemp.NumDummy(); i++)
    {
        pndBatch_G = ghTemp.GetDummy(i);
        pndBatchTo_G = NULL;
        pndBatchTo_G = pndBatch_G->GetPostDummy();
        if (pndBatchTo_G) ghTemp.DLink(pndBatch_G, pndBatchTo_G);
    }

    // reduce graph (ghTemp)
    for (i = 1; i <= pSeq->Num(); i++)
    {
        iTemp = (*pSeq)[i];
        if (iTemp > 0)
        {  // remove only the operations
            pndOp_S = SGraph.GetID(iTemp);
            pndOp_G = ghTemp.GetID(iTemp);
            pndBatch_S = pndOp_S->GetPostDummy();
            pndBatch_G = ghTemp.GetID(pndBatch_S->nID);

            for (j = 1; j <= pndOp_S->stkPostArc.Num(); j++)
            {
                paTo_S = pndOp_S->GetPostArcPtr(j);
                pndTo_S = paTo_S->pndTo;
                if (pndTo_S->IsOperation())
                {  // if pndS is an operation, add the link between batches
                    pndTemp = ghTemp.GetID(pndTo_S->nID);
                    pndBatchTo_G = pndTemp->GetPostDummy();
                    // pndBatchTo_G is point to the batch holding the op that pndSTo is pointed to
                    ghTemp.Link(pndBatch_G, pndBatchTo_G, paTo_S->fWeight);
                }
            }
            // remove pndG from ghTemp
            ghTemp.Reduce(pndOp_G);
        }
    }

    // find batch sequence
    int iBatchNo = 0;
    for (i = 1; i <= pSeq->Num(); i++)
    {
        iBatchNo++;
        j = 1;
        iTemp = (*pSeq)[i];
        if (iTemp > 0)
        {
            pndOp_S = SGraph.GetID(iTemp);
            pndBatch_S = pndOp_S->GetPostDummy();
            sqDummy << pndBatch_S->nID;
            BatchSize[iBatchNo] = 1;
            BatchList[iBatchNo][j] = iTemp;
            // BatchID[iBatchNo]      = pndBatch_S->nID;
            BatchIndex[-pndBatch_S->nID] = iBatchNo;
            // increae i
            while ((iTemp > 0) && (i <= pSeq->Num()))
            {
                i++;
                iTemp = (*pSeq)[i];
                if (iTemp > 0)
                {
                    BatchSize[iBatchNo] += 1;
                    BatchList[iBatchNo][++j] = iTemp;
                }
            }
        }
    }
    sqDummy.mID = pMC->mID;

    // single m/c cpi
    T1MC_Center smc(pShop, pWkCenter, pMC, &ghTemp, ObjFnc);
    smc.Append(sqDummy);
    smc.CPI();

    // convert to linear sequence
    TSequence sqLinear;
    TStackP stkTemp;
    DGraph.ReCalc();
    for (i = 1; i <= smc.pSeq->Num(); i++)
    {
        iTemp = (*smc.pSeq)[i];
        iTemp *= -1;
        // find batch -- j
        j = BatchIndex[iTemp];
        /*
        for(j=1;j<=iBatchNo;j++)
        {
          if(BatchID[j]==iTemp)
            break;
        }
        */
        // put the op in sqLinear according to release time (DGraph)
        stkTemp.Clear();
        for (k = 1; k <= BatchSize[j]; k++)
        {
            pndTemp = DGraph.GetID(BatchList[j][k]);
            fTemp = pndTemp->EST();
            stkTemp.InsertSort(pndTemp, fTemp);
        }
        while (stkTemp.Num() > 0)
        {
            pndTemp = (TNode*)stkTemp.PopFirst();
            sqLinear << pndTemp->nID;
        }
        sqLinear << 0;
    }
    sqLinear.mID = pMC->mID;

    ClearSequence();
    Append(sqLinear);
    // batching
    Batching();
}

void TBMC_Center::CPI_Setup()
{
    // run after a feasible sequence is loaded
    TDGraph ghTemp = SGraph;
    TSequence sqDummy;
    TArrayL2 BatchList(SGraph.NumDummy() + 2, pMC->iBatchSize + 1);
    TArrayL BatchSize(SGraph.NumDummy() + 2), BatchIndex(SGraph.NumDummy() + 4);
    int i, j, k, iTemp;
    TNode *pndTemp, *pndOp_S, *pndOp_G, *pndTo_S, *pndBatch_S, *pndBatch_G,
        *pndBatchTo_G;  //*pndBatchTo_S, *pndTo_G,
    TArc* paTo_S;

    pSeq = Seq.Get(1);

    // remove links between dummies
    for (i = 1; i <= ghTemp.NumDummy(); i++)
    {
        pndBatch_G = ghTemp.GetDummy(i);
        pndBatchTo_G = NULL;
        pndBatchTo_G = pndBatch_G->GetPostDummy();
        if (pndBatchTo_G) ghTemp.DLink(pndBatch_G, pndBatchTo_G);
    }

    // reduce graph (ghTemp)
    for (i = 1; i <= pSeq->Num(); i++)
    {
        iTemp = (*pSeq)[i];
        if (iTemp > 0)
        {  // remove only the operations
            pndOp_S = SGraph.GetID(iTemp);
            pndOp_G = ghTemp.GetID(iTemp);
            pndBatch_S = pndOp_S->GetPostDummy();
            pndBatch_G = ghTemp.GetID(pndBatch_S->nID);

            for (j = 1; j <= pndOp_S->stkPostArc.Num(); j++)
            {
                paTo_S = pndOp_S->GetPostArcPtr(j);
                pndTo_S = paTo_S->pndTo;
                if (pndTo_S->IsOperation())
                {  // if pndS is an operation, add the link between batches
                    pndTemp = ghTemp.GetID(pndTo_S->nID);
                    pndBatchTo_G = pndTemp->GetPostDummy();
                    // pndBatchTo_G is point to the batch holding the op that pndSTo is pointed to
                    ghTemp.Link(pndBatch_G, pndBatchTo_G, paTo_S->fWeight);
                }
            }
            // remove pndG from ghTemp
            ghTemp.Reduce(pndOp_G);
        }
    }

    // find batch sequence
    int iBatchNo = 0;
    for (i = 1; i <= pSeq->Num(); i++)
    {
        iBatchNo++;
        j = 1;
        iTemp = (*pSeq)[i];
        if (iTemp > 0)
        {
            pndOp_S = SGraph.GetID(iTemp);
            pndBatch_S = pndOp_S->GetPostDummy();
            sqDummy << pndBatch_S->nID;
            BatchSize[iBatchNo] = 1;
            BatchList[iBatchNo][j] = iTemp;
            // BatchID[iBatchNo]      = pndBatch_S->nID;
            BatchIndex[-pndBatch_S->nID] = iBatchNo;
            // increae i
            while ((iTemp > 0) && (i <= pSeq->Num()))
            {
                i++;
                iTemp = (*pSeq)[i];
                if (iTemp > 0)
                {
                    BatchSize[iBatchNo] += 1;
                    BatchList[iBatchNo][++j] = iTemp;
                }
            }
        }
    }
    sqDummy.mID = pMC->mID;

    // single m/c cpi
    T1MC_Center smc(pShop, pWkCenter, pMC, &ghTemp, ObjFnc);
    smc.Append(sqDummy);
    smc.CPI();
    smc.Local(70);

    // convert to linear sequence
    TSequence sqLinear;
    DGraph.ReCalc();
    for (i = 1; i <= smc.pSeq->Num(); i++)
    {
        iTemp = (*smc.pSeq)[i];
        iTemp *= -1;
        // find batch -- j
        j = BatchIndex[iTemp];
        // put the op in sqLinear according to release time (DGraph)
        for (k = 1; k <= BatchSize[j]; k++)
        {
            pndTemp = DGraph.GetID(BatchList[j][k]);
            sqLinear << pndTemp->nID;
        }
        sqLinear << 0;
    }
    sqLinear.mID = pMC->mID;

    ClearSequence();
    Append(sqLinear);
}
