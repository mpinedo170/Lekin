#include "StdAfx.h"

#include "CPM.h"

TDGraph grhTemp1_buf;
TNode nTemp1_buf;

void Trace(LPCTSTR szName, TDGraph& Graph)
{
    ofstream myfile(szName);
    myfile << Graph;
    myfile.close();
}

void Log(LPCTSTR szName, TDGraph& Graph)
{
    ofstream myfile(szName, ios::out | ios::app);
    myfile << Graph << endl;
    myfile.close();
}

/////////////////////// TNode //////////////////////////

TNode::TNode(TNodeID _nID, double _fIndex, double _fEST, double _fLST, double _fDueDate)
{
    nID = _nID;
    fIndex = _fIndex;
    fEST = _fEST;
    fLST = _fLST;
    fDueDate = _fDueDate;
    op = 0;
}

TNode::TNode(TNode& ndX)
{
    int iN, i;
    nID = ndX.nID;
    fIndex = ndX.fIndex;
    fEST = ndX.fEST;
    fLST = ndX.fLST;
    fDueDate = ndX.fDueDate;
    op = ndX.op;
    iN = ndX.stkPriorArc.Num();
    ndX.stkPriorArc.Head();
    for (i = 1; i <= iN; i++) AddPrior(*(TArc*)ndX.stkPriorArc.Next());
    iN = ndX.stkPostArc.Num();
    ndX.stkPostArc.Head();
    for (i = 1; i <= iN; i++) AddPost(*(TArc*)ndX.stkPostArc.Next());
}

TNode& TNode::operator=(TNode& ndX)
{
    Clear();
    int iN, i;
    nID = ndX.nID;
    fIndex = ndX.fIndex;
    fEST = ndX.fEST;
    fLST = ndX.fLST;
    fDueDate = ndX.fDueDate;
    op = ndX.op;
    iN = ndX.stkPriorArc.Num();
    ndX.stkPriorArc.Head();
    for (i = 1; i <= iN; i++) AddPrior(*(TArc*)ndX.stkPriorArc.Next());
    iN = ndX.stkPostArc.Num();
    ndX.stkPostArc.Head();
    for (i = 1; i <= iN; i++) AddPost(*(TArc*)ndX.stkPostArc.Next());
    return (*this);
}

void TNode::Clear()
{
    int i, iN;
    TArc* parcTemp;
    op = 0;
    iN = stkPriorArc.Num();
    for (i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPriorArc.Pop();
        delete parcTemp;
    }
    iN = stkPostArc.Num();
    for (i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPostArc.Pop();
        delete parcTemp;
    }
}

void TNode::Destroy()
{
    Clear();
    stkPriorArc.Destroy();
    stkPostArc.Destroy();
    delete this;
}

void TNode::AddPrior(TArc& arcX)
{
    int iTemp = 1, iN = stkPriorArc.Num();
    TArc* parcTemp;
    stkPriorArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPriorArc.Next();
        if (parcTemp->pndTo == arcX.pndTo)
        {
            iTemp = 0;
            parcTemp->fWeight = arcX.fWeight;
        }
    }
    if (iTemp)
    {
        TArc* parcNew = new TArc(arcX);
        stkPriorArc.Push(parcNew);
    }
}

void TNode::AddPrior(TNode* pndFrom, double fWeightI)
{
    int iTemp = 1, iN = stkPriorArc.Num();
    TArc* parcTemp;
    stkPriorArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPriorArc.Next();
        if (parcTemp->pndTo == pndFrom)
        {
            iTemp = 0;
            parcTemp->fWeight = fWeightI;
        }
    }
    if (iTemp)
    {
        TArc* parcNew = new TArc(pndFrom, fWeightI);
        stkPriorArc.Push(parcNew);
    }
}

int TNode::RemovePrior(TNode* pndRemove)
{
    int iN;
    TArc* parcTemp;
    iN = stkPriorArc.Num();
    stkPriorArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPriorArc.Current();
        if ((parcTemp->pndTo) == pndRemove)
        {
            stkPriorArc.Delete();
            delete parcTemp;
            return 1;
        }
        stkPriorArc.Next();
    }
    return 0;
}

void TNode::AddPost(TArc& arcX)
{
    int iTemp = 1, iN = stkPostArc.Num();
    TArc* parcTemp;
    stkPostArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPostArc.Next();
        if (parcTemp->pndTo == arcX.pndTo)
        {
            iTemp = 0;
            parcTemp->fWeight = arcX.fWeight;
        }
    }
    if (iTemp)
    {
        TArc* parcNew = new TArc(arcX);
        stkPostArc.Push(parcNew);
    }
}

void TNode::AddPost(TNode* pndTo, double fWeightI)
{
    int i;
    int iN = stkPostArc.Num();
    TArc* pArc;
    stkPostArc.Head();
    for (i = 1; i <= iN; i++)
    {
        pArc = (TArc*)stkPostArc.Next();
        if (pArc->pndTo == pndTo)
        {
            pArc->fWeight = fWeightI;
            break;
        }
    }
    if (i > iN)
    {
        TArc* parcNew = new TArc(pndTo, fWeightI);
        stkPostArc.Push(parcNew);
    }
}

int TNode::RemovePost(TNode* pndRemove)
{
    int iN;
    TArc* parcTemp;
    iN = stkPostArc.Num();
    stkPostArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPostArc.Current();
        if ((parcTemp->pndTo) == pndRemove)
        {
            stkPostArc.Delete();
            delete parcTemp;
            return 1;
        }
        stkPostArc.Next();
    }
    return 0;
}

double TNode::GetPriorWeight(TNode* pndFrom)
{
    int iN = stkPriorArc.Num();
    TArc* parcTemp;
    stkPriorArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPriorArc.Next();
        if ((parcTemp->pndTo) == pndFrom) return parcTemp->fWeight;
    }
    return fBigM;
}

double TNode::GetPostWeight(TNode* pndToI)
{
    int iN = stkPostArc.Num();
    TArc* parcTemp;
    stkPostArc.Head();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)stkPostArc.Next();
        if ((parcTemp->pndTo) == pndToI) return parcTemp->fWeight;
    }
    return fBigM;
}

TNode* TNode::GetPostDummy()
{
    TNode* pndTemp;
    for (int i = 1; i <= stkPostArc.Num(); i++)
    {
        pndTemp = GetPostNode(i);
        if (pndTemp->IsDummy()) return pndTemp;
    }
    return NULL;
}

/////////////////////// TDGraph /////////////////////////

TDGraph::TDGraph(int _iType)
{
    iType = _iType;
    nLastSink = -(nMaxNode - 1);
    nLastDummy = -2;
    AddNode(0, 0, 0, fBigMM, fBigMM);
    iFlag = 0;
}

TDGraph::TDGraph(TDGraph& dgX)
{
    TNode *pndTemp, *pndT2, *pndFrom, *pndFromOld, *pndTo;
    TArc* parcTemp;
    TStackP stkTemp = dgX.stkActive;
    nLastSink = dgX.nLastSink;
    nLastDummy = dgX.nLastDummy;
    iType = dgX.iType;
    iFlag = dgX.iFlag;
    int i, j, iN, iM;
    iN = dgX.Num();

    ////////////////// set size of aryNode /////////////
    int iMaxNodeID = 0;
    dgX.Head();
    for (i = 1; i <= iN; i++)
    {
        pndT2 = (TNode*)dgX.Next();
        if (pndT2->nID > iMaxNodeID) iMaxNodeID = pndT2->nID;
    }
    aryNode.SetSize(iMaxNodeID);

    dgX.Head();
    /////////////////// add nodes //////////////////////
    for (i = 1; i <= iN; i++)
    {
        pndT2 = (TNode*)dgX.Next();
        pndTemp = new TNode(pndT2->nID, pndT2->fIndex, pndT2->fEST, pndT2->fLST, pndT2->fDueDate);
        pndTemp->op = pndT2->op;
        Push(pndTemp, pndT2->fIndex);
        if (pndT2->nID >= 0) aryNode.Put(pndT2->nID, pndTemp);
        if (pndT2->IsSink()) stkSink.Push(pndTemp, pndT2->fIndex);
        if (pndT2->IsDummy()) stkDummy.Push(pndTemp, pndT2->fIndex);
        if (stkTemp.Locate(pndT2) > 0)
        {
            stkActive.Push(pndTemp);
            stkTemp.Delete();
        }
    }
    /////////////////// add links //////////////////////
    for (i = 1; i <= iN; i++)
    {
        pndFrom = (TNode*)operator[](i);
        pndFromOld = (TNode*)dgX[i];
        iM = (pndFromOld->stkPostArc).Num();
        pndFromOld->stkPostArc.Head();
        for (j = 1; j <= iM; j++)
        {
            parcTemp = (TArc*)(pndFromOld->stkPostArc.Next());
            pndTo = (TNode*)operator[](dgX.Locate(parcTemp->pndTo));
            Link(pndFrom, pndTo, parcTemp->fWeight);
        }
    }
}

TDGraph& TDGraph::operator=(TDGraph& dgX)
{
    Clear();
    TNode* pNode = (TNode*)Pop();
    pNode->Clear();
    delete pNode;

    TNode *pndTemp, *pndT2, *pndFrom, *pndFromOld, *pndTo;
    TArc* parcTemp;
    TStackP stkTemp = dgX.stkActive;
    nLastSink = dgX.nLastSink;
    nLastDummy = dgX.nLastDummy;
    iType = dgX.iType;
    iFlag = dgX.iFlag;
    int i, j, iN, iM;
    iN = dgX.Num();
    ////////////////// set size of aryNode /////////////
    int iMaxNodeID = 0;
    dgX.Head();
    for (i = 1; i <= iN; i++)
    {
        pndT2 = (TNode*)dgX.Next();
        if (pndT2->nID > iMaxNodeID) iMaxNodeID = pndT2->nID;
    }
    aryNode.SetSize(iMaxNodeID);
    /////////////////// add nodes //////////////////////
    dgX.Head();
    for (i = 1; i <= iN; i++)
    {
        pndT2 = (TNode*)dgX.Next();
        pndTemp = new TNode(pndT2->nID, pndT2->fIndex, pndT2->fEST, pndT2->fLST, pndT2->fDueDate);
        pndTemp->op = pndT2->op;
        Push(pndTemp, pndT2->fIndex);
        if (pndT2->nID >= 0) aryNode.Put(pndT2->nID, pndTemp);
        if (pndT2->IsSink()) stkSink.Push(pndTemp, pndT2->fIndex);
        if (pndT2->IsDummy()) stkDummy.Push(pndTemp, pndT2->fIndex);
        if (stkTemp.Member(pndT2))
        {
            stkActive.Push(pndTemp);
            stkTemp.Delete();
        }
    }
    /////////////////// add links //////////////////////
    for (i = 1; i <= iN; i++)
    {
        pndFrom = (TNode*)operator[](i);
        pndFromOld = (TNode*)dgX[i];
        iM = (pndFromOld->stkPostArc).Num();
        pndFromOld->stkPostArc.Head();
        for (j = 1; j <= iM; j++)  ///// post arcs //////
        {
            parcTemp = (TArc*)(pndFromOld->stkPostArc.Next());
            pndTo = (TNode*)operator[](dgX.Locate(parcTemp->pndTo));
            Link(pndFrom, pndTo, parcTemp->fWeight);
        }
    }
    return (*this);
}

TDGraph::~TDGraph()
{
    Clear();
    TNode* pndTemp = (TNode*)Pop();
    // pndTemp->Clear();
    delete pndTemp;
}

TNode* TDGraph::AddNode(TNodeID nIDI, double fIndexI, double fESTI, double fLSTI, double fDueDateI)
{
    TNode* pndTemp = new TNode(nIDI, fIndexI, fESTI, fLSTI, fDueDateI);
    Push(pndTemp, fIndexI);
    if (nIDI >= 0)
        aryNode.Put(nIDI, pndTemp);
    else if (pndTemp->IsDummy())
    {
        nLastDummy = min(nLastDummy, nIDI - 1);
        stkDummy.Push(pndTemp);
    }
    else
    {
        nLastSink = max(nLastSink, nIDI + 1);
        stkSink.Push(pndTemp, fBigMM);  // may have error here!
    }
    return pndTemp;
}

TNode* TDGraph::AddSink(TNodeID nIDI, double fWeightI, double fDueDateI)
{  // add sink node after node nIDI
    int iN, i;
    TNode *pndTemp, *pndReturn = 0;
    pndTemp = GetID(nIDI);
    ///// single sink -- find sink node address /////
    if (iType == 1)
    {
        ///// if no sink node -> add one /////
        if (stkSink.Num() == 0)
        {
            pndReturn = AddNode(nLastSink, fBigMM, 0, fBigM, fDueDateI);
            // stkSink.Push(pndReturn,fBigMM);
        }
        pndReturn = (TNode*)stkSink[1];
    }
    ///// case: manual sink node assignment /////
    if (nIDI > 0)
    {
        ///// cannot find the node /////
        if (!pndTemp)
            return 0;
        else
        {
            ///// add new sink node /////
            if (iType > 1)
            {
                pndReturn = AddNode(nLastSink, fBigMM, 0, fDueDateI, fDueDateI);
                // stkSink.Push(pndReturn,fBigMM);
                Link(pndTemp, pndReturn, fWeightI);
            }
            else
                ///// add arc to sink node /////
                Link(pndTemp, pndReturn, fWeightI);
        }
    }
    else
    ///// case: scan every nodes that don't have post arc /////
    {
        iN = Num();
        for (i = 0; i <= iN; i++)
        {
            pndTemp = (TNode*)operator[](i);
            if (pndTemp->nID != 0)
            {
                if ((pndTemp->stkPostArc.Num() == 0) && (pndTemp->nID >= 0))
                {
                    if (iType == 2)
                    {
                        pndReturn = AddNode(nLastSink, fBigMM, 0, fDueDateI, fDueDateI);
                        // stkSink.Push(pndReturn,fBigMM);
                        Link(pndTemp, pndReturn, fWeightI);
                    }
                    else  // single sink node
                        Link(pndTemp, pndReturn, fWeightI);
                }
            }
        }
    }
    return pndReturn;
}

TNode* TDGraph::AddDummy()
{
    TNode* pRtn = AddNode(nLastDummy);
    // stkDummy.Push(pRtn);
    return pRtn;
}

int TDGraph::Remove(TNodeID nIDI)
{
    TNode* pndTemp;
    pndTemp = GetID(nIDI);
    return (Remove(pndTemp));
}

int TDGraph::Remove(TNode* pndX)
{
    int iN;
    TNode* pndTemp;
    TArc* parcTemp;
    iN = pndX->stkPriorArc.Num();
    ///// remove the connecting arcs /////
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)pndX->stkPriorArc[1];
        if (parcTemp) DLink(parcTemp->pndTo, pndX);
    }
    iN = pndX->stkPostArc.Num();
    for (int i = 1; i <= iN; i++)
    {
        parcTemp = (TArc*)pndX->stkPostArc[1];
        if (parcTemp) DLink(pndX, parcTemp->pndTo);
    }
    ///// remove pndX from the stack /////
    if (Member(pndX))
    {
        pndTemp = (TNode*)Current();
        Delete();
        aryNode.Put(pndTemp->nID, 0);
        // pndTemp->Clear();
        delete pndTemp;
        return 1;
    }
    else
        return 0;
}

int TDGraph::RemoveDummy(TNode* pNode)
{
    int iTemp = stkDummy.Locate(pNode);
    if (iTemp > 0)
        return RemoveDummy(iTemp);
    else
        return 0;
}

int TDGraph::RemoveDummy(int iLocation)
{
    TNode* pNode;
    int i, iTemp;
    if ((iLocation > 0) && (iLocation <= NumDummy()))
    {
        pNode = (TNode*)stkDummy.Pop(iLocation);
        iTemp = pNode->nID;
        Remove(pNode);
        // find nLastDummy
        if (iTemp >= nLastDummy + 1)
        {
            nLastDummy = -2;
            stkDummy.Head();
            if (iLocation <= nLastDummy + 1)
                for (i = 1; i <= stkDummy.Num(); i++)
                {
                    pNode = (TNode*)stkDummy.Next();
                    nLastDummy = min(nLastDummy, pNode->nID - 1);
                }
        }
        return 1;
    }
    else
        return 0;
}

void TDGraph::Clear()
{
    TNode* pndTemp;
    /*
    Head();
    for(int i=1;i<=Num();i++)
    {
      pndTemp = (TNode*)Next();
      pndTemp->Clear();
      delete pndTemp;
    }
    TStackP::Clear();
    */
    int iN;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pndTemp = (TNode*)Pop();
        // pndTemp->Clear();
        delete pndTemp;
    }

    stkActive.Clear();
    stkSink.Clear();
    stkDummy.Clear();
    aryNode.Clear();
    AddNode(0, 0, 0, fBigMM, fBigMM);
    nLastSink = -(nMaxNode - 1);
    nLastDummy = -2;
    iFlag = 0;
}

void TDGraph::Destroy()
{
    TNode* pndTemp;
    int iN;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pndTemp = (TNode*)Pop();
        pndTemp->Clear();
        delete pndTemp;
    }
    stkActive.Destroy();
    stkSink.Destroy();
    stkDummy.Destroy();
    aryNode.Clear();
    TStackP::Destroy();
}

TNode* TDGraph::GetID(TNodeID nIDI)
{
    TNode* pndTemp;
    int i;
    if (nIDI >= 0)
        return (TNode*)(aryNode[nIDI]);
    else
    {
        if (nIDI <= nLastSink)
        {
            stkSink.Head();
            for (i = 1; i <= stkSink.Num(); i++)
            {
                pndTemp = (TNode*)stkSink.Next();
                if (nIDI == pndTemp->nID) return pndTemp;
            }
        }
        if (nIDI >= nLastDummy)
        {
            stkDummy.Head();
            for (i = 1; i <= stkDummy.Num(); i++)
            {
                pndTemp = (TNode*)stkDummy.Next();
                if (nIDI == pndTemp->nID) return pndTemp;
            }
        }
    }
    return 0;
}

TNode* TDGraph::GetSink(int iN)
{
    if ((iN <= NumSink()) && (iN > 0))
        return ((TNode*)stkSink[iN]);
    else
        return 0;
}

int TDGraph::Link(TNodeID nID1, TNodeID nID2, double fWeightI)
{
    TNode *pndTo, *pndFrom;
    pndFrom = GetID(nID1);
    pndTo = GetID(nID2);
    return Link(pndFrom, pndTo, fWeightI);
}

int TDGraph::Link(TNode* pndFrom, TNode* pndTo, double fWeightI)
{  // replace the link with max(fWeightI,current weight)
    int i, iN, iLocation;
    TArc* pArc;
    if ((pndFrom) && (pndTo))
    {
        //*****
        int iChange = 0;
        iN = pndFrom->stkPostArc.Num();
        pndFrom->stkPostArc.Head();
        for (i = 1; i <= iN; i++)
        {
            pArc = (TArc*)pndFrom->stkPostArc.Next();
            if (pArc->pndTo == pndTo)
            {
                // replace weight
                if (fWeightI > pArc->fWeight)
                {
                    pArc->fWeight = fWeightI;
                    iChange = 1;
                }
                break;
            }
        }
        if (iChange == 1)
        {
            iN = pndTo->stkPriorArc.Num();
            pndTo->stkPriorArc.Head();
            for (i = 1; i <= iN; i++)
            {
                pArc = (TArc*)pndTo->stkPriorArc.Next();
                if (pArc->pndTo == pndFrom)
                {
                    // replace weight
                    pArc->fWeight = fWeightI;
                    break;
                }
            }
        }
        if (i > iN)
        {  // not found
            pndFrom->AddPost(pndTo, fWeightI);
            pndTo->AddPrior(pndFrom, fWeightI);
        }
        //*****
        ///// add pndFrom to stkActive if pndFrom don't have prior arcs /////
        if ((pndFrom->stkPriorArc).Num() == 0) AddActive(pndFrom);
        ///// remove pndTo from stkActive /////
        iLocation = stkActive.Locate(pndTo);
        if ((iLocation > 0) && (pndFrom->nID != 0)) stkActive.Pop(iLocation);
        ///// update EST /////
        if (pndFrom->EST() + fWeightI > pndTo->EST()) pndTo->fEST = pndFrom->EST() + fWeightI;

        ///// reset iFlag /////
        iFlag = 1;
        return 1;
    }
    return 0;  // cannot establish a link
}

int TDGraph::RLink(TNodeID nID1, TNodeID nID2, double fWeightI)
{  // replace the link with fWeightI if exists
    TNode *pndTo, *pndFrom;
    pndFrom = GetID(nID1);
    pndTo = GetID(nID2);
    return RLink(pndFrom, pndTo, fWeightI);
}

int TDGraph::RLink(TNode* pndFrom, TNode* pndTo, double fWeightI)
{
    int iLocation;
    if ((pndFrom) && (pndTo))
    {
        pndFrom->AddPost(pndTo, fWeightI);
        pndTo->AddPrior(pndFrom, fWeightI);
        ///// add pndFrom to stkActive if pndFrom don't have prior arcs /////
        if ((pndFrom->stkPriorArc).Num() == 0) AddActive(pndFrom);
        ///// remove pndTo from stkActive /////
        iLocation = stkActive.Locate(pndTo);
        if (iLocation > 0) stkActive.Pop(iLocation);

        ///// update EST /////
        if (pndFrom->EST() + fWeightI > pndTo->EST()) pndTo->fEST = pndFrom->EST() + fWeightI;

        ///// reset iFlag /////
        iFlag = 1;
        return 1;
    }
    return 0;  // cannot establish a link
}

int TDGraph::DLink(TNodeID nID1, TNodeID nID2)
{
    TNode *pndTo, *pndFrom;
    pndFrom = GetID(nID1);
    pndTo = GetID(nID2);
    return (DLink(pndFrom, pndTo));
}

int TDGraph::DLink(TNode* pndFrom, TNode* pndTo)
{
    TNode* pndTemp;
    int iReturn, iTemp, iLocation;
    if ((pndFrom) && (pndTo))
    {
        iReturn = pndFrom->RemovePost(pndTo);
        iTemp = pndTo->RemovePrior(pndFrom);
        if (iTemp > 0) iReturn = 1;
        ///// add pndTo to stkActive if no prior arc but some post arcs /////
        if ((pndTo->stkPostArc).Num() > 0)
        {
            if ((pndTo->stkPriorArc).Num() == 0)
                AddActive(pndTo);
            else if (pndTo->stkPriorArc.Num() == 1)
            {
                pndTemp = pndTo->GetPriorNode(1);
                if (pndTemp->nID == 0) AddActive(pndTo);
            }
        }

        ///// remove pndFrom from stkActive if no post arc /////
        iLocation = stkActive.Locate(pndFrom);
        if ((iLocation > 0) && ((pndFrom->stkPostArc).Num() == 0)) stkActive.Pop(iLocation);
        ///// reset the iFlag /////
        iFlag = 1;
        return iReturn;
    }
    return 0;  // cannot establish a link
}

void TDGraph::FindActive()
{
    // search for active nodes
    int i;
    TNode *pNode, *pndTemp;
    stkActive.Clear();
    Head();
    for (i = 1; i <= Num(); i++)
    {
        pNode = (TNode*)Next();
        if (pNode->IsSink() != 1)
        {
            if ((pNode->stkPostArc).Num() > 0)
            {
                if ((pNode->stkPriorArc).Num() == 0)
                    AddActive(pNode);
                else if (pNode->stkPriorArc.Num() == 1)
                {
                    pndTemp = pNode->GetPriorNode(1);
                    if (pndTemp->nID == 0) AddActive(pNode);
                }
            }
        }
    }
}

void TDGraph::ActiveList(TStack& stk, double fTime)
{
    stk.Clear();
    TNode* pndTemp;
    int iN = stkActive.Num();
    stkActive.Head();
    for (int i = 1; i <= iN; i++)
    {
        pndTemp = (TNode*)stkActive.Next();
        if (pndTemp->fEST <= fTime) stk.Push(pndTemp->nID);
    }
}

TNodeID TDGraph::Active(double fTime)
{
    int i, iN = stkActive.Num();
    double fMin = fBigM, fBIndex = -fBigM;
    TStackP stkTemp;
    TNode *pndTemp, *pndMin, *pndBIndex;
    if (iN == 0) return -1;  // no active node
    stkActive.Head();
    for (i = 1; i <= iN; i++)
    {
        pndTemp = (TNode*)stkActive.Current();
        if (stkActive.Index() > fBIndex)
        {
            pndBIndex = pndTemp;
            fBIndex = stkActive.Index();
        }
        if (pndTemp->fEST < fMin)
        {
            fMin = pndTemp->fEST;
            pndMin = pndTemp;
        }
        stkActive.Next();
    }
    if (pndBIndex->fEST <= fTime)
        return pndBIndex->nID;
    else
        return pndMin->nID;
}

int TDGraph::CopyEST(TDGraph& dgX)
{
    int iN = Num(), iM = dgX.Num(), iOk = 1;
    TNode *pndFrom, *pndTo;
    if (iN == iM)
    {
        Head();
        dgX.Head();
        for (int i = 1; i <= iN; i++)
        {
            pndTo = (TNode*)Next();
            pndFrom = (TNode*)dgX.Next();
            if (pndTo->nID == pndFrom->nID)
                pndTo->fEST = pndFrom->fEST;
            else
                iOk = 0;
        }
        if (iOk) return 1;
    }
    return 0;
}

int TDGraph::CopyLST(TDGraph& dgX)
{
    int iN = Num(), iM = dgX.Num(), iOk = 1;
    TNode *pndFrom, *pndTo;
    if (iN == iM)
    {
        Head();
        dgX.Head();
        for (int i = 1; i <= iN; i++)
        {
            pndTo = (TNode*)Next();
            pndFrom = (TNode*)dgX.Next();
            if (pndTo->nID == pndFrom->nID)
            {
                pndTo->fLST = pndFrom->fLST;
                pndTo->fDueDate = pndFrom->fDueDate;
            }
            else
                iOk = 0;
        }
        if (iOk) return 1;
    }
    return 0;
}

int TDGraph::CopyESTLST(TDGraph& dgX)
{
    int iN = Num(), iM = dgX.Num(), iOk = 1;
    TNode *pndFrom, *pndTo;
    if (iN == iM)
    {
        Head();
        dgX.Head();
        for (int i = 1; i <= iN; i++)
        {
            pndTo = (TNode*)Next();
            pndFrom = (TNode*)dgX.Next();
            if (pndTo->nID == pndFrom->nID)
            {
                pndTo->fEST = pndFrom->fEST;
                pndTo->fLST = pndFrom->fLST;
                pndTo->fDueDate = pndFrom->fDueDate;
            }
            else
                iOk = 0;
        }
        if (iOk) return 1;
    }
    return 0;
}

int TDGraph::Sequence(TNodeID nIDI)
{
    return Sequence(GetID(nIDI));
}

int TDGraph::Sequence(TNode* pndCurrent)
{
    int i, iN, j, iM;
    TNode* pndPointTo;
    iN = stkActive.Num();
    ///// check if nIDI is in the active list /////
    for (i = 1; i <= iN; i++)
        if (stkActive[i] == pndCurrent)
        {
            iM = (pndCurrent->stkPostArc).Num();
            for (j = 1; j <= iM; j++)
            {
                pndPointTo = (pndCurrent->GetPostArcPtr(1))->pndTo;
                UpDate(pndCurrent, pndPointTo);
                DLink(pndCurrent, pndPointTo);
            }
            return 1;
        }
    return 0;
}

int TDGraph::Sequence(TNodeID nIDI, double fProcessTime)
{
    int i, j, iN, iM;
    TNode *pndCurrent = GetID(nIDI), *pndPointTo;
    TArc* pPostArc;
    iN = stkActive.Num();
    ///// check if nIDI is in the active list /////
    for (i = 1; i <= iN; i++)
        if (stkActive[i] == pndCurrent)
        {
            iM = (pndCurrent->stkPostArc).Num();
            for (j = 1; j <= iM; j++)
            {
                pPostArc = pndCurrent->GetPostArcPtr(1);
                pPostArc->fWeight = (double)fProcessTime;
                pndPointTo = pPostArc->pndTo;
                UpDate(pndCurrent, pndPointTo);
                DLink(pndCurrent, pndPointTo);
            }
            return 1;
        }
    return 0;
}

void TDGraph::Copy(TDGraph& dgX)
{
    Clear();
    TNode* pNode = (TNode*)Pop();
    pNode->Clear();
    delete pNode;
    TNode *pndTemp, *pndT2, *pndFrom, *pndFromOld, *pndTo;
    TArc* parcTemp;
    TStackP stkTemp = dgX.stkActive;
    nLastSink = dgX.nLastSink;
    nLastDummy = dgX.nLastDummy;
    iType = dgX.iType;
    iFlag = dgX.iFlag;
    int i, j, iN, iM, iTemp;
    iN = dgX.Num();
    ////////////////// set size of aryNode /////////////
    int iMaxNodeID = 0;
    dgX.Head();
    for (i = 1; i <= iN; i++)
    {
        pndT2 = (TNode*)dgX.Next();
        if (pndT2->nID > iMaxNodeID) iMaxNodeID = pndT2->nID;
    }
    aryNode.SetSize(iMaxNodeID);
    /////////////////// add nodes //////////////////////
    dgX.Head();
    for (i = 1; i <= iN; i++)
    {
        pndT2 = (TNode*)dgX.Next();
        pndTemp = new TNode(pndT2->nID, pndT2->fIndex, 0, fBigM, fBigM);
        if (pndT2->stkPriorArc.Num() == 0)
        {  // if there is no prior arc, copy data
            pndTemp->fEST = pndT2->fEST;
            pndTemp->fLST = pndT2->fLST;
            pndTemp->fDueDate = pndT2->fDueDate;
        }
        pndTemp->op = pndT2->op;
        if (pndT2->IsSink()) pndTemp->fDueDate = pndT2->fDueDate;
        Push(pndTemp, pndT2->fIndex);
        if (pndT2->nID >= 0) aryNode.Put(pndT2->nID, pndTemp);
        if (pndT2->IsSink()) stkSink.Push(pndTemp, pndT2->fIndex);
        if (pndT2->IsDummy()) stkDummy.Push(pndTemp, pndT2->fIndex);
        if (stkTemp.Member(pndT2))
        {
            stkActive.Push(pndTemp);
            stkTemp.Delete();
        }
    }
    /////////////////// add links //////////////////////
    for (i = 1; i <= iN; i++)
    {
        pndFrom = (TNode*)operator[](i);
        pndFromOld = (TNode*)dgX[i];
        iM = (pndFromOld->stkPostArc).Num();
        pndFromOld->stkPostArc.Head();
        for (j = 1; j <= iM; j++)
        {
            parcTemp = (TArc*)pndFromOld->stkPostArc.Next();
            iTemp = dgX.Locate(parcTemp->pndTo);
            pndTo = (TNode*)operator[](iTemp);
            Link(pndFrom, pndTo, parcTemp->fWeight);
        }
    }
    iFlag = 1;
}

int TDGraph::ReCalcEST()
{
    int iN, i;
    TNode *pndCurrent, *pndPointTo;
    grhTemp1_buf.Copy(*this);

    // assign -BigM to all the node with prior node except active nodes
    grhTemp1_buf.Head();
    for (i = 1; i <= grhTemp1_buf.Num(); i++)
    {
        pndCurrent = (TNode*)grhTemp1_buf.Next();
        if (pndCurrent->stkPriorArc.Num() > 0)
            if (!grhTemp1_buf.stkActive.Locate(pndCurrent)) pndCurrent->fEST = -fBigM;
    }
    // Trace("EST1.grh",grhTemp1_buf);
    // Trace("Graph.grh",*this);

    /////// determine EST ///////
    // sequence node 0 first
    pndCurrent = grhTemp1_buf.GetID(0);
    if (pndCurrent)
    {
        if (grhTemp1_buf.stkActive.Locate(pndCurrent) > 0)
        {
            iN = (pndCurrent->stkPostArc).Num();
            for (i = 1; i <= iN; i++)
            {
                pndCurrent->stkPostArc.Head();
                pndPointTo = ((TArc*)pndCurrent->stkPostArc.Current())->pndTo;
                grhTemp1_buf.UpDate(pndCurrent, pndPointTo);
                grhTemp1_buf.DLink(pndCurrent, pndPointTo);
            }
        }
    }
    while (grhTemp1_buf.stkActive.Num() > 0)
    {
        grhTemp1_buf.stkActive.Head();
        pndCurrent = (TNode*)grhTemp1_buf.stkActive.Current();
        iN = (pndCurrent->stkPostArc).Num();
        for (i = 1; i <= iN; i++)
        {
            pndCurrent->stkPostArc.Head();
            pndPointTo = ((TArc*)pndCurrent->stkPostArc.Current())->pndTo;
            grhTemp1_buf.UpDate(pndCurrent, pndPointTo);
            grhTemp1_buf.DLink(pndCurrent, pndPointTo);
        }
    }
    // Trace("EST2.grh",grhTemp1_buf);
    // Trace("Graph.grh",*this);

    //////// copy EST from grhTemp1_buf /////////
    Head();
    grhTemp1_buf.Head();
    for (i = 1; i <= Num(); i++)
    {
        pndCurrent = (TNode*)Next();
        pndCurrent->fEST = ((TNode*)grhTemp1_buf.Next())->fEST;
    }
    // Trace("EST3.grh",*this);

    //////// check feasibility by look at the prior arcs of the sink //////
    iN = grhTemp1_buf.stkSink.Num();
    grhTemp1_buf.stkSink.Head();
    for (i = 1; i <= iN; i++)
    {
        pndCurrent = (TNode*)grhTemp1_buf.stkSink.Next();
        if ((pndCurrent->stkPriorArc).Num()) return 0;  // prior arc found  -> infeasible
    }
    return 1;
}

int TDGraph::ReCalcLST()
{
    /////// initilize for LST : adding the sink node to the stkActiveR //////
    grhTemp1_buf = *this;
    int i, iN;
    TStackP stkActiveR;
    TNode *pndCurrent, *pndPointTo;

    // assign fBigM to all the node with post node
    grhTemp1_buf.Head();
    for (i = 1; i <= grhTemp1_buf.Num(); i++)
    {
        pndCurrent = (TNode*)grhTemp1_buf.Next();
        if (pndCurrent->stkPostArc.Num() > 0)
        {
            pndCurrent->fLST = fBigM;
            pndCurrent->fDueDate = fBigM;
        }
    }

    iN = (grhTemp1_buf.stkSink).Num();
    grhTemp1_buf.stkSink.Head();
    for (i = 1; i <= iN; i++)
    {
        pndCurrent = (TNode*)grhTemp1_buf.stkSink.Next();
        pndCurrent->fLST = pndCurrent->fEST;
        if (pndCurrent->stkPriorArc.Num()) stkActiveR.Push(pndCurrent, pndCurrent->fIndex);
    }
    /////// determine LST & DueDate ///////
    while (stkActiveR.Num() > 0)
    {
        stkActiveR.Head();
        pndCurrent = (TNode*)stkActiveR.Current();
        iN = (pndCurrent->stkPriorArc).Num();
        for (i = 1; i <= iN; i++)
        {
            pndCurrent->stkPriorArc.Head();
            pndPointTo = ((TArc*)pndCurrent->stkPriorArc.Current())->pndTo;
            grhTemp1_buf.UpDateR(pndPointTo, pndCurrent);
            grhTemp1_buf.DLink(pndPointTo, pndCurrent);
            if (((pndPointTo->stkPostArc).Num() == 0) && (pndPointTo->stkPriorArc).Num())
                stkActiveR.Push(pndPointTo, pndPointTo->fIndex);
            if ((pndCurrent->stkPriorArc).Num() == 0) stkActiveR.PopFirst();
        }
    }
    //////// copy LST & DueDate from grhTemp1_buf /////////
    iN = Num();
    Head();
    grhTemp1_buf.Head();
    for (i = 1; i <= iN; i++)
    {
        pndCurrent = (TNode*)Next();
        pndCurrent->fLST = ((TNode*)grhTemp1_buf.Current())->fLST;
        pndCurrent->fDueDate = ((TNode*)grhTemp1_buf.Next())->fDueDate;
    }
    //////// check feasibility by look at the post arcs of the source //////
    pndCurrent = grhTemp1_buf.GetID(0);
    if ((pndCurrent->stkPostArc).Num()) return 0;  // post arc found  -> infeasible
    return 1;
}

int TDGraph::ReCalc()
{
    int iReturn1 = 1, iReturn2 = 1;
    if (iFlag)  // need recalc
    {
        iReturn1 = ReCalcEST();
        //    if(iReturn)
        iReturn2 = ReCalcLST();
        if (iReturn1 && iReturn2)
        {
            iFlag = 0;
            return 1;  // return normal
        }
        else
            return 2;  // not a feasible sequence
    }
    else
        return 0;  // recalc is not necessary
}

void TDGraph::ReCalc2()
{
    int iN, i;
    TNode *pndCurrent, *pndPointTo;
    grhTemp1_buf = *this;

    // don't assign -BigM to all the node with prior node except active nodes
    /*
    grhTemp1_buf.Head();
    for(i=1;i<=grhTemp1_buf.Num();i++)
    {
      pndCurrent = (TNode*)grhTemp1_buf.Next();
      if(pndCurrent->stkPriorArc.Num()>0)
        if(!grhTemp1_buf.stkActive.Locate(pndCurrent))
          pndCurrent->fEST = -fBigM;
    }
    */

    /////// determine EST ///////
    // sequence node 0 first
    pndCurrent = grhTemp1_buf.GetID(0);
    if (pndCurrent)
    {
        if (grhTemp1_buf.stkActive.Locate(pndCurrent) > 0)
        {
            iN = (pndCurrent->stkPostArc).Num();
            for (i = 1; i <= iN; i++)
            {
                pndCurrent->stkPostArc.Head();
                pndPointTo = ((TArc*)pndCurrent->stkPostArc.Current())->pndTo;
                grhTemp1_buf.UpDate(pndCurrent, pndPointTo);
                grhTemp1_buf.DLink(pndCurrent, pndPointTo);
            }
        }
    }
    while (grhTemp1_buf.stkActive.Num() > 0)
    {
        grhTemp1_buf.stkActive.Head();
        pndCurrent = (TNode*)grhTemp1_buf.stkActive.Current();
        iN = (pndCurrent->stkPostArc).Num();
        for (i = 1; i <= iN; i++)
        {
            pndCurrent->stkPostArc.Head();
            pndPointTo = ((TArc*)pndCurrent->stkPostArc.Current())->pndTo;
            grhTemp1_buf.UpDate(pndCurrent, pndPointTo);
            grhTemp1_buf.DLink(pndCurrent, pndPointTo);
        }
    }

    //////// copy EST from grhTemp1_buf /////////
    Head();
    grhTemp1_buf.Head();
    for (i = 1; i <= Num(); i++)
    {
        pndCurrent = (TNode*)Next();
        pndCurrent->fEST = ((TNode*)grhTemp1_buf.Next())->fEST;
    }

    //    if(iReturn)
    grhTemp1_buf = *this;
    TStackP stkActiveR;

    // assign fBigM to all the node with post node
    grhTemp1_buf.Head();
    for (i = 1; i <= grhTemp1_buf.Num(); i++)
    {
        pndCurrent = (TNode*)grhTemp1_buf.Next();
        if (pndCurrent->stkPostArc.Num() > 0)
        {
            pndCurrent->fLST = fBigM;
            pndCurrent->fDueDate = fBigM;
        }
    }

    iN = (grhTemp1_buf.stkSink).Num();
    grhTemp1_buf.stkSink.Head();
    for (i = 1; i <= iN; i++)
    {
        pndCurrent = (TNode*)grhTemp1_buf.stkSink.Next();
        pndCurrent->fLST = pndCurrent->fEST;
        if (pndCurrent->stkPriorArc.Num()) stkActiveR.Push(pndCurrent, pndCurrent->fIndex);
    }
    /////// determine LST & DueDate ///////
    while (stkActiveR.Num() > 0)
    {
        stkActiveR.Head();
        pndCurrent = (TNode*)stkActiveR.Current();
        iN = (pndCurrent->stkPriorArc).Num();
        for (i = 1; i <= iN; i++)
        {
            pndCurrent->stkPriorArc.Head();
            pndPointTo = ((TArc*)pndCurrent->stkPriorArc.Current())->pndTo;
            grhTemp1_buf.UpDateR(pndPointTo, pndCurrent);
            grhTemp1_buf.DLink(pndPointTo, pndCurrent);
            if (((pndPointTo->stkPostArc).Num() == 0) && (pndPointTo->stkPriorArc).Num())
                stkActiveR.Push(pndPointTo, pndPointTo->fIndex);
            if ((pndCurrent->stkPriorArc).Num() == 0) stkActiveR.PopFirst();
        }
    }
    //////// copy LST & DueDate from grhTemp1_buf /////////
    iN = Num();
    Head();
    grhTemp1_buf.Head();
    for (i = 1; i <= iN; i++)
    {
        pndCurrent = (TNode*)Next();
        pndCurrent->fLST = ((TNode*)grhTemp1_buf.Current())->fLST;
        pndCurrent->fDueDate = ((TNode*)grhTemp1_buf.Next())->fDueDate;
    }
}

double TDGraph::MakeSpan()
{
    int iN = stkSink.Num();
    double fReturn = 0, fTemp;
    TNode* pNode;
    if (iFlag) ReCalc();
    stkSink.Head();
    for (int i = 1; i <= iN; i++)
    {
        pNode = (TNode*)stkSink.Next();
        fTemp = pNode->EST();
        fReturn = (fReturn > fTemp) ? fReturn : fTemp;
    }
    return fReturn;
}

void TDGraph::UpDate(TNode* pndFrom, TNode* pndTo)
{
    double fLength;
    fLength = pndFrom->GetPostWeight(pndTo);
    pndTo->fEST = ((pndTo->fEST) > ((pndFrom->fEST) + fLength)) ? (pndTo->fEST) : ((pndFrom->fEST) + fLength);
}

void TDGraph::UpDateR(TNode* pndFrom, TNode* pndTo)
{
    double fLength;
    fLength = pndFrom->GetPostWeight(pndTo);
    pndFrom->fLST = min((pndFrom->fLST), ((pndTo->fLST) - fLength));
    pndFrom->fDueDate = min((pndFrom->fDueDate), (pndTo->fDueDate) - fLength);
}

void TDGraph::AddActive(TNode* pndX)
{
    // modified 8/16/96
    // int iN = stkActive.Num();
    if (!pndX->IsSink())
        if (!stkActive.Member(pndX)) stkActive.Push(pndX, pndX->fIndex);
}

double TDGraph::DueDate(TNodeID nID)
{
    if (iFlag) ReCalc();
    return GetID(nID)->fDueDate;
}

int TDGraph::Reduce(TNode* pNode)
{
    TArc* pArc;
    TNode *pndPrior, *pndPost;
    double fLength1, fLength2;
    if (pNode == 0) return 0;
    int i, iN, j, iM;
    iN = pNode->stkPriorArc.Num();
    iM = pNode->stkPostArc.Num();
    for (i = 1; i <= iN; i++)
    {
        pArc = pNode->GetPriorArcPtr(i);
        pndPrior = pArc->pndTo;
        fLength1 = pArc->fWeight;
        for (j = 1; j <= iM; j++)
        {
            pArc = pNode->GetPostArcPtr(j);
            pndPost = pArc->pndTo;
            fLength2 = pArc->fWeight;
            Link(pndPrior, pndPost, fLength1 + fLength2);
            // pNode->RemovePost(pndPost);
            // pndPost->RemovePrior(pNode);
        }
        // pndPrior->RemovePost(pNode);
        // pNode->RemovePrior(pndPrior);
        //    DLink(pndPrior,pNode);
    }
    Remove(pNode);
    return 1;
}

//*** overload iostream ***//

ostream& operator<<(ostream& os, TNode& ndX)
{
    int iN, i;
    os << ndX.nID << ") EST = " << ndX.EST() << "   LST = " << ndX.LST() << "   DueDate = ";
    os << ndX.DueDate();
    //**********
    // os<<"\taddress : "<<&ndX;
    //**********
    os << endl << "\tPrior nodes : ";
    iN = (ndX.stkPriorArc).Num();
    if (iN > 0) os << ndX.GetPriorID(1) << ":" << ndX.GetPriorWeight(1) << " ";
    for (i = 2; i <= iN; i++) os << " " << ndX.GetPriorID(i) << ":" << ndX.GetPriorWeight(i) << " ";
    os << endl << "\tPost nodes : ";
    iN = (ndX.stkPostArc).Num();
    if (iN > 0) os << ndX.GetPostID(1) << ":" << ndX.GetPostWeight(1) << " ";
    for (i = 2; i <= iN; i++) os << " " << ndX.GetPostID(i) << ":" << ndX.GetPostWeight(i) << " ";
    os << endl << endl;
    return os;
}

ostream& operator<<(ostream& os, TDGraph& dgX)
{
    int iN, i;
    os << "Project Graph" << endl << "=============" << endl << "Active nodes : ";
    iN = (dgX.stkActive).Num();
    if (iN > 0) os << (((TNode*)dgX.stkActive[1])->ID());
    for (i = 2; i <= iN; i++)
    {
        os << ", " << (((TNode*)dgX.stkActive[i])->ID());
    }
    os << endl;
    /*
    cArray2_f Arr;
    TNode pNode;
    dgX.Head();
    int iLastOp=0;
    for(i=1;i<=dgX.Num();i++)
    {
      pNode = (TNode*)dgX.Next();
      if(pNode->IsOperation())
        iLastOp++;
    }
    */
    iN = dgX.Num();
    for (i = 1; i <= iN; i++) os << (*(TNode*)dgX[i]);
    return os;
}
