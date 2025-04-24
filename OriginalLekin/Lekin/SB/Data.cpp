#include "StdAfx.h"

#include "Data.h"

void Trace(LPCSTR szName, TJob& jb)
{
    ofstream myfile(szName);
    myfile << jb;
    myfile.close();
}

void Trace(LPCSTR szName, TJobList& jl)
{
    ofstream myfile(szName);
    myfile << jl;
    myfile.close();
}

void Trace(LPCSTR szName, TWorkcenter& wkc)
{
    ofstream myfile(szName);
    myfile << wkc;
    myfile.close();
}

void Trace(LPCSTR szName, TWorkcenterList& wl)
{
    ofstream myfile(szName);
    myfile << wl;
    myfile.close();
}

//*********************** TOperation *************************//

TOperation::TOperation()
{
    fStartTime = fFinishTime = fSetupTime = fActProcessTime = 0;
    mID = 0;
    pJob = 0;
}

TOperation::TOperation(TOperationID _oID,
    double _fProcessTime,
    double _fReleaseTime,
    double _fDueDate,
    double _fWeight,
    double _fEWeight,
    LPCSTR _szDescription)
{
    oID = _oID;
    fProcessTime = _fProcessTime;
    fReleaseTime = _fReleaseTime;
    fDueDate = _fDueDate;
    fWeight = _fWeight;
    fEWeight = _fEWeight;
    _tcscpy(szDescription, _szDescription);
    sMC_Status = sDefault;
    fStartTime = fFinishTime = fSetupTime = fActProcessTime = 0;
    mID = 0;
    pJob = 0;
}

TOperation::TOperation(TOperation& opX)
{
    oID = opX.oID;
    sMC_Status = opX.sMC_Status;
    _tcscpy(szDescription, opX.szDescription);
    fProcessTime = opX.fProcessTime;
    fReleaseTime = opX.fReleaseTime;
    fDueDate = opX.fDueDate;
    fWeight = opX.fWeight;
    fEWeight = opX.fEWeight;
    fStartTime = opX.fStartTime;
    fFinishTime = opX.fFinishTime;
    fSetupTime = opX.fSetupTime;
    fActProcessTime = opX.fActProcessTime;
    mID = opX.mID;
    stkPriorOp = opX.stkPriorOp;
    stkPostOp = opX.stkPostOp;
    stkMachine = opX.stkMachine;
    pJob = opX.pJob;
}

TOperation& TOperation::operator=(TOperation& opX)
{
    oID = opX.oID;
    fProcessTime = opX.fProcessTime;
    fReleaseTime = opX.fReleaseTime;
    fStartTime = opX.fStartTime;
    fDueDate = opX.fDueDate;
    fWeight = opX.fWeight;
    fEWeight = opX.fEWeight;
    fSetupTime = opX.fSetupTime;
    sMC_Status = opX.sMC_Status;
    fFinishTime = opX.fFinishTime;
    mID = opX.mID;
    fActProcessTime = opX.fActProcessTime;
    _tcscpy(szDescription, opX.szDescription);
    stkPriorOp = opX.stkPriorOp;
    stkPostOp = opX.stkPostOp;
    stkMachine = opX.stkMachine;
    pJob = opX.pJob;
    return (*this);
}

int TOperation::IsInMCStack(int _mID)
{
    for (int i = 1; i <= stkMachine.Num(); i++)
        if ((GetMachine(i)->mID) == _mID) return 1;
    return 0;
}

void TOperation::Clear()
{
    stkPriorOp.Clear();
    stkPostOp.Clear();
    stkMachine.Clear();
    oID = 0;
}

//*********************** TJob ***********************//

TJob::TJob(
    TJobID _jID, double _fRelease, double _fDueDate, double _fWeight, double _fEWeight, LPCSTR _szDescription)
{
    jID = _jID;
    fReleaseTime = _fRelease;
    fDueDate = _fDueDate;
    fWeight = _fWeight;
    fEWeight = _fEWeight;
    _tcscpy(szDescription, _szDescription);
}

TJob::TJob(TJob& jbX)
{
    int iN, iM, i, j;
    TOperation* popTemp;
    TOperationID oTo, oFrom;
    jID = jbX.jID;
    fReleaseTime = jbX.fReleaseTime;
    fDueDate = jbX.fDueDate;
    fWeight = jbX.fWeight;
    fEWeight = jbX.fEWeight;
    _tcscpy(szDescription, jbX.szDescription);
    iN = jbX.Num();
    jbX.Head();
    //***** append operations
    for (i = 1; i <= iN; i++)
    {
        popTemp = (TOperation*)jbX.Next();  // Get(i);
        popTemp = Append(*popTemp);
        popTemp->stkPriorOp.Clear();
        popTemp->stkPostOp.Clear();
    }
    //***** add the precedent relations
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        oTo = popTemp->oID;
        iM = (popTemp->stkPriorOp).Num();
        popTemp->stkPriorOp.Head();
        for (j = 1; j <= iM; j++)
        {
            // oFrom = (popTemp->GetPrior(j))->oID;
            oFrom = ((TOperation*)popTemp->stkPriorOp.Next())->oID;
            Link(oFrom, oTo);
        }
    }
}

void TJob::Clear()
{
    int iN;
    TOperation* popTemp;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        popTemp = (TOperation*)operations.Pop();
        // popTemp->Clear();
        delete popTemp;
    }
    jID = 0;
}

TJob& TJob::operator=(TJob& jbX)
{
    int iN, iM, i, j;
    TOperation* popTemp;
    TOperationID oTo, oFrom;
    Clear();
    jID = jbX.jID;
    fReleaseTime = jbX.fReleaseTime;
    fDueDate = jbX.fDueDate;
    fWeight = jbX.fWeight;
    fEWeight = jbX.fEWeight;
    _tcscpy(szDescription, jbX.szDescription);
    iN = jbX.Num();
    jbX.Head();
    //***** append operations
    for (i = 1; i <= iN; i++)
    {
        popTemp = (TOperation*)jbX.Next();  // Get(i);
        popTemp = Append(*popTemp);
        popTemp->stkPriorOp.Clear();
        popTemp->stkPostOp.Clear();
    }
    //***** add the precedent relations
    for (i = 1; i <= iN; i++)
    {
        popTemp = jbX.Get(i);
        oTo = popTemp->oID;
        iM = popTemp->stkPriorOp.Num();
        popTemp->stkPriorOp.Head();
        for (j = 1; j <= iM; j++)
        {
            // oFrom = (popTemp->GetPrior(j))->oID;
            oFrom = ((TOperation*)popTemp->stkPriorOp.Next())->oID;
            Link(oFrom, oTo);
        }
    }
    return (*this);
}

TOperation* TJob::Append(TOperation& opX)
{
    TOperation* popNew;
    if (GetID(opX.oID) > 0)
        return 0;  // duplicate ID found
    else
    {
        popNew = new TOperation(opX);
        popNew->pJob = this;
        /*** remove precedence relations ***/
        popNew->stkPriorOp.Clear();
        popNew->stkPostOp.Clear();
        operations.Push(popNew);
        operations.SetIndex(Num());
        return popNew;
    }
}

TOperation* TJob::GetID(TOperationID oID)
{
    TOperation* pOp;
    Head();
    for (int i = 1; i <= Num(); i++)
    {
        pOp = (TOperation*)Next();
        if (pOp->oID == oID) return pOp;
    }
    return 0;
}

int TJob::Link(TOperationID oID1, TOperationID oID2)
{
    TOperation* popFrom;
    TOperation* popTo;
    int i, iN;
    popFrom = GetID(oID1);
    popTo = GetID(oID2);
    if ((popFrom != 0) && (popTo != 0))
    {
        // check if the link exist?
        iN = popTo->stkPostOp.Num();
        popTo->stkPostOp.Head();
        for (i = 1; i <= iN; i++)
        {
            if (popTo == popFrom->stkPriorOp.Next()) return 0;  // found existing link
        }
        popFrom->AddPost(popTo);
        popTo->AddPrior(popFrom);
        return 1;
    }
    else
        return 0;
}

int TJob::DLink(TOperationID oID1, TOperationID oID2)
{
    TOperation* popFrom;
    TOperation* popTo;
    int iTemp, iReturn = 0;
    popFrom = GetID(oID1);
    popTo = GetID(oID2);
    if ((popFrom != 0) && (popTo != 0))
    {
        iReturn = popFrom->RemovePost(popTo);
        iTemp = popTo->RemovePrior(popFrom);
        if (iReturn == 0) iReturn = iTemp;
        iTemp = popFrom->RemovePost(popTo);
        if (iReturn == 0) iReturn = iTemp;
        iTemp = popTo->RemovePrior(popFrom);
        if (iReturn == 0) iReturn = iTemp;
        return iReturn;
    }
    else
        return 0;
}

int TJob::Remove(TOperation* popX)
{
    int iN, iM, i, j, k;
    TOperation *popTemp = 0, *popFrom, *popTo;
    iN = Num();
    Head();
    for (i = 1; i <= iN; i++)
    {
        popTemp = Current();
        if (popTemp == popX)
        {
            // add links
            iM = popX->stkPriorOp.Num();
            for (j = 1; j <= iM; j++)
            {
                popFrom = popX->GetPrior(j);
                for (k = 1; k <= popX->stkPostOp.Num(); k++)
                {
                    popTo = popX->GetPost(k);
                    popFrom->AddPost(popTo);
                    popTo->AddPrior(popFrom);
                }
                popTemp->RemovePost(popX);
            }
            // remove prior links
            iM = popX->stkPriorOp.Num();
            for (j = 1; j <= iM; j++)
            {
                popTemp = popX->GetPrior(j);
                popTemp->RemovePost(popX);
            }
            // remove post links
            iM = popX->stkPostOp.Num();
            for (j = 1; j <= iM; j++)
            {
                popTemp = popX->GetPost(j);
                popTemp->RemovePrior(popX);
            }
            operations.Delete();
            // popX->Clear();
            delete popX;
            popX = NULL;
            return 1;
        }
        Next();
    }
    return 0;
}

int TJob::Remove(int iLocation)
{
    TOperation* popTemp;
    TOperation* popTemp2;
    int i, iN;
    if (iLocation > Num()) return 0;
    popTemp = (TOperation*)operations.Pop(iLocation);
    // remove prior links
    iN = popTemp->stkPriorOp.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = popTemp->GetPrior(i);
        popTemp2->RemovePost(popTemp);
    }
    // remove post links
    iN = popTemp->stkPostOp.Num();
    for (i = 1; i <= iN; i++)
    {
        popTemp2 = popTemp->GetPost(i);
        popTemp2->RemovePrior(popTemp);
    }
    // popTemp->Clear();
    delete popTemp;
    popTemp = NULL;
    return 1;
}

//************************* TJobList **************************//

TJobList::TJobList(TJobList& jblX)
{
    int i, iN = jblX.Num();
    for (i = 1; i <= iN; i++)
    {
        Append(jblX[i]);
    }
}

TJobList& TJobList::operator=(TJobList& jblX)
{
    Clear();
    int i, iN = jblX.Num();
    for (i = 1; i <= iN; i++)
    {
        Append(jblX[i]);
    }
    return *this;
}

TJob* TJobList::Append(TJob& jbX)
{
    TJob* pjobNew = new TJob(jbX);
    Push(pjobNew);
    return pjobNew;
}

TJob* TJobList::GetID(TJobID jIDI)
{
    TJob* pjbTemp;
    int iN = Num();
    Head();
    for (int i = 1; i <= iN; i++)
    {
        pjbTemp = (TJob*)Next();
        if (pjbTemp->jID == jIDI) return (pjbTemp);
    }
    return 0;
}

TOperation* TJobList::Geto_oID(TOperationID oIDI)
{
    int i, j, iM, iN = Num();
    TJob* pjbTemp;
    TOperation* popTemp;
    Head();
    for (i = 1; i <= iN; i++)
    {
        pjbTemp = (TJob*)Next();
        iM = pjbTemp->Num();
        pjbTemp->Head();
        for (j = 1; j <= iM; j++)
        {
            popTemp = (TOperation*)pjbTemp->Current();
            if (popTemp->oID == oIDI) return popTemp;
            pjbTemp->Next();
        }
    }
    return 0;
}

TJob* TJobList::Getj_oID(TOperationID oIDI)
{
    int i, iN = Num();
    TJob* pReturn;
    Head();
    for (i = 1; i <= iN; i++)
    {
        pReturn = (TJob*)Next();
        if (pReturn->GetID(oIDI)) return pReturn;
    }
    return 0;
}

int TJobList::Remove(int iLocation)
{
    TJob* pjbTemp;
    if (iLocation <= Num())
    {
        pjbTemp = (TJob*)Pop(iLocation);
        // pjbTemp->Clear();
        delete pjbTemp;
        return 1;
    }
    else
        return 0;
}

int TJobList::Remove(TJob* pjb)
{
    if (Locate(pjb) > 0)
    {
        Delete();
        // pjb->Clear();
        delete pjb;
        return 1;
    }
    else
        return 0;
}

int TJobList::RemoveID(TJobID jIDI)
{
    int iN, i;
    TJob* pjbTemp;
    iN = Num();
    Head();
    for (i = 1; i <= iN; i++)
    {
        pjbTemp = (TJob*)Current();
        if (pjbTemp->jID == jIDI)
        {
            Delete();
            // pjbTemp->Clear();
            delete pjbTemp;
            return 1;
        }
        Next();
    }
    return 0;
}

void TJobList::RandomProcessTime(int iFrom, int iTo)
{
    int i, j;
    TOperation* pOp;
    TJob* pJob;
    for (i = 1; i <= Num(); i++)
    {
        pJob = Get(i);
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOp = pJob->Get(j);
            pOp->fProcessTime = (double)Random(iFrom, iTo);
        }
    }
}

void TJobList::RandomReleaseTime(int iFrom, int iTo)
{
    int i, j;
    TJob* pJob;
    TOperation* popTemp;
    for (i = 1; i <= Num(); i++)
    {
        pJob = Get(i);
        pJob->fReleaseTime = (double)Random(iFrom, iTo);
        for (j = 1; j <= pJob->Num(); j++)
        {
            popTemp = pJob->Get(j);
            popTemp->fReleaseTime = 0;
        }
    }
}

void TJobList::RandomDueDate(double fPercentFrom, double fPercentTo)
{  // RandomDuedate(1.2,1.3) = 20% - 30% over combined process time
    int i, j;
    double fTemp, fBest;
    TJob* pJob;
    TOperation *pOp, *pOpNext;
    for (i = 1; i <= Num(); i++)
    {
        pJob = Get(i);
        // find max combined process time
        fBest = 0;
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOp = pJob->Get(j);
            fTemp = 0;
            if (pOp->stkPriorOp.Num() == 0)
            {
                fTemp = pOp->fProcessTime;
                pOpNext = pOp->GetPost(1);
                while (pOpNext)
                {
                    fTemp += pOpNext->fProcessTime;
                    pOpNext = pOpNext->GetPost(1);
                }
                if (fTemp > fBest) fBest = fTemp;
            }
        }
        fTemp = Random(fPercentFrom, fPercentTo, 3);
        pJob->fDueDate = pJob->fReleaseTime + (int)(fBest * fTemp + 0.5);
    }
}

void TJobList::RandomPriority(int iFrom, int iTo)
{
    int i;
    TJob* pJob;
    for (i = 1; i <= Num(); i++)
    {
        pJob = Get(i);
        pJob->fWeight = (double)Random(iFrom, iTo);
        pJob->fEWeight = 1 / pJob->fWeight / 100;
    }
}

void TJobList::RandomStatus(TCHAR cFrom, TCHAR cTo)
{
    int iFrom = cFrom, iTo = cTo;
    TCHAR cRtn;
    int i, j;
    TOperation* pOp;
    TJob* pJob;
    for (i = 1; i <= Num(); i++)
    {
        pJob = Get(i);
        for (j = 1; j <= pJob->Num(); j++)
        {
            pOp = pJob->Get(j);
            cRtn = TCHAR(Random(iFrom, iTo));
            pOp->sMC_Status = cRtn;
        }
    }
}

void TJobList::Clear()
{
    int iN;
    iN = Num();
    TJob* pjbTemp;
    for (int i = 1; i <= iN; i++)
    {
        pjbTemp = (TJob*)Pop();
        // pjbTemp->Clear();
        delete pjbTemp;
    }
}

//************************** TSetupString ****************************//

TSetupString::TSetupString(TSetupString& susX)
{
    TSetupElemt* sueTemp;
    int iN;
    iN = susX.Num();
    // susX.Head();
    for (int i = 1; i <= iN; i++)
    {
        // sueTemp = (TSetupElemt*)susX.Next();
        sueTemp = &(susX[i]);
        Append(sueTemp->sCode, sueTemp->fTime);
    }
    sCode = susX.sCode;
    fDefault = susX.fDefault;
}

TSetupString& TSetupString::operator=(TSetupString& susX)
{
    TSetupElemt* sueTemp;
    int iN;
    Clear();
    iN = susX.Num();
    // susX.Head();
    for (int i = 1; i <= iN; i++)
    {
        // sueTemp = (TSetupElemt*)susX.Next();
        sueTemp = &(susX[i]);
        Append(sueTemp->sCode, sueTemp->fTime);
    }
    sCode = susX.sCode;
    fDefault = susX.fDefault;
    return (*this);
}

void TSetupString::Clear()
{
    TSetupElemt* psueTemp;
    int iN;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        psueTemp = (TSetupElemt*)Pop();
        delete psueTemp;
    }
}

void TSetupString::Append(TStatus sCodeI, double fTimeI)
{
    if (LocateCode(sCodeI) == 0)
    {
        TSetupElemt* psueNew = new TSetupElemt(sCodeI, fTimeI);
        Push(psueNew);
    }
}

TSetupElemt* TSetupString::LocateCode(TStatus sCodeI)
{
    int iN;
    TSetupElemt* suE;
    iN = Num();
    Head();
    for (int i = 1; i <= iN; i++)
    {
        suE = (TSetupElemt*)Next();
        if (suE->sCode == sCodeI) return (suE);
        // Next();
    }
    return 0;
}

int TSetupString::Remove(TStatus sCodeI)
{
    TSetupElemt* psueTemp = 0;
    int iN;
    iN = Num();
    Head();
    for (int i = 1; i <= iN; i++)
    {
        psueTemp = (TSetupElemt*)Current();
        if (psueTemp->sCode == sCodeI)
        {
            Delete();
            delete psueTemp;
            return 1;
        }
    }
    return 0;
}

double TSetupString::Get(TStatus sCodeI)
{
    TSetupElemt* suE = LocateCode(sCodeI);
    if (suE)
        return (suE->fTime);
    else
        return fDefault;
}

//************************* TSetup **************************//

TSetup::TSetup(TSetup& suX)
{
    int iN;
    TSetupString susTemp(sDefault, 0);
    fDefault = suX.fDefault;
    iN = suX.Num();
    for (int i = 1; i <= iN; i++)
    {
        susTemp = suX[i];
        Append(susTemp);
    }
}

TSetup& TSetup::operator=(TSetup& suX)
{
    int iN;
    TSetupString susTemp(sDefault, 0);
    Clear();
    fDefault = suX.fDefault;
    iN = suX.Num();
    for (int i = 1; i <= iN; i++)
    {
        susTemp = suX[i];
        Append(susTemp);
    }
    return (*this);
}

void TSetup::Clear()
{
    TSetupString* psusTemp;
    int iN;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        psusTemp = (TSetupString*)Pop();
        // psusTemp->Clear();
        delete psusTemp;
    }
}

void TSetup::Append(TSetupString& susX)
{
    TSetupString* psusNew;
    if (LocateCode(susX.sCode) == 0)
    {
        susX.fDefault = fDefault;
        psusNew = new TSetupString(susX);
        Push(psusNew);
    }
}

void TSetup::Set(TStatus sFrom, TStatus sTo, double fSetupI)
{
    TSetupString* psusTemp;
    TSetupElemt* psueTemp;
    TStatus sTemp = sFrom;
    /*if(sFrom>sTo)
    {
      sFrom = sTo;
      sTo   = sTemp;
    }*/
    psusTemp = LocateCode(sFrom);
    if (psusTemp != 0)
    {
        psueTemp = psusTemp->LocateCode(sTo);
        if (psueTemp != 0)
            psueTemp->fTime = fSetupI;  // replace
        else
            psusTemp->Append(sTo, fSetupI);
    }
    else
    {  ///// add new SetupString /////
        psusTemp = new TSetupString(sFrom, fDefault);
        psusTemp->Append(sTo, fSetupI);
        Append(*psusTemp);
        psusTemp->Clear();
        delete psusTemp;
    }
}

TSetupString* TSetup::LocateCode(TStatus sCode)
{
    int iN = Num();
    TSetupString* susTemp;
    Head();
    for (int i = 1; i <= iN; i++)
    {
        susTemp = (TSetupString*)Current();
        if (susTemp->sCode == sCode) return (susTemp);
        Next();
    }
    return 0;
}

int TSetup::Remove(TStatus sCode)
{
    TSetupString* psusTemp = 0;
    int iN;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        if (operator[](i).sCode == sCode)
        {
            psusTemp = (TSetupString*)Pop(i);
            psusTemp->Clear();
            delete psusTemp;
            return 1;
        }
    }
    return 0;
}

double TSetup::Get(TStatus sFrom, TStatus sTo)
{
    TSetupString* psusTemp;
    TStatus sTemp = sFrom;
    /*if(sFrom>sTo)
    {
      sFrom = sTo;
      sTo   = sTemp;
    }*/
    psusTemp = LocateCode(sFrom);
    if (psusTemp) return (psusTemp->Get(sTo));
    return fDefault;
}

//*********************** TMachine *****************************//
TMachine::TMachine(TMachineID _mID,
    LPCSTR _szDescription,
    TStatus _sInit,
    double _fSpeed,
    int _iBatchSize,
    double _fAvailTime)
{
    mID = _mID;
    sInit = _sInit;
    fSpeed = _fSpeed;
    iBatchSize = _iBatchSize;
    fAvailTime = _fAvailTime;
    _tcscpy(szDescription, _szDescription);
    pWkCenter = 0;
}

TMachine::TMachine(TMachine& mchX)
{
    mID = mchX.mID;
    sInit = mchX.sInit;
    fAvailTime = mchX.fAvailTime;
    fSpeed = mchX.fSpeed;
    iBatchSize = mchX.iBatchSize;
    _tcscpy(szDescription, mchX.szDescription);
    suSetup = mchX.suSetup;
    stkTask = mchX.stkTask;
    fAvailTime = mchX.fAvailTime;
    pWkCenter = mchX.pWkCenter;
}

TMachine& TMachine::operator=(TMachine& mchX)
{
    mID = mchX.mID;
    sInit = mchX.sInit;
    fAvailTime = mchX.fAvailTime;
    fSpeed = mchX.fSpeed;
    iBatchSize = mchX.iBatchSize;
    _tcscpy(szDescription, mchX.szDescription);
    suSetup = mchX.suSetup;
    stkTask = mchX.stkTask;
    fAvailTime = mchX.fAvailTime;
    pWkCenter = mchX.pWkCenter;
    return (*this);
}

int TMachine::IsTask(int iOp)
{
    TOperation* popTemp;
    stkTask.Head();
    for (int i = 1; i <= stkTask.Num(); i++)
    {
        popTemp = (TOperation*)stkTask.Next();
        if (popTemp->oID == iOp) return 1;
    }
    return 0;
}

int TMachine::RemoveTask(TOperation* popX)
{
    if (stkTask.Locate(popX))
        return (stkTask.Delete());
    else
        return 0;
}

void TMachine::RandomSetup(TCHAR cFrom, TCHAR cTo, int iFrom, int iTo)
{
    suSetup.Clear();
    for (TCHAR ci = cFrom; ci <= cTo; ci++)
        for (TCHAR cj = ci + 1; cj <= cTo; cj++)
        {
            suSetup.Set(ci, cj, Random(iFrom, iTo));
            suSetup.Set(cj, ci, Random(iFrom, iTo));
        }
}

//*********************** TWorkcenter *************************//

TWorkcenter::TWorkcenter(TWorkcenter& wkcX)
{
    int iN;
    wID = wkcX.wID;
    _tcscpy(szDescription, wkcX.szDescription);
    stkTask = wkcX.stkTask;
    iN = wkcX.Num();
    wkcX.Head();
    for (int i = 1; i <= iN; i++) Append(*(TMachine*)(wkcX.Next()));
}

TWorkcenter& TWorkcenter::operator=(TWorkcenter& wkcX)
{
    int iN;
    Clear();
    wID = wkcX.wID;
    _tcscpy(szDescription, wkcX.szDescription);
    stkTask = wkcX.stkTask;
    iN = wkcX.Num();
    wkcX.Head();
    for (int i = 1; i <= iN; i++) Append(*(TMachine*)(wkcX.Next()));
    return (*this);
}

TMachine* TWorkcenter::Append(TMachine& mchX)
{
    TMachine* pmchNew;
    if (GetID(mchX.mID) > 0)
        return 0;  // duplicate ID found
    else
    {
        pmchNew = new TMachine(mchX);
        pmchNew->pWkCenter = this;
        Push(pmchNew);
        return pmchNew;
    }
}

void TWorkcenter::AddTask(TOperation* popX)
{
    if (!stkTask.Member(popX)) stkTask.Push(popX);
}

TOperation* TWorkcenter::GetTask(int iLocation)
{
    if (iLocation <= stkTask.Num())
        return (TOperation*)stkTask[iLocation];
    else
        return 0;
}

int TWorkcenter::RemoveTask(TOperation* popX)
{
    if (stkTask.Locate(popX))
        return (stkTask.Delete());
    else
        return 0;
}

int TWorkcenter::IsTask(TOperationID oIDI)
{
    int iN = stkTask.Num();
    TOperation* popTemp;
    stkTask.Head();
    for (int i = 1; i <= iN; i++)
    {
        popTemp = (TOperation*)stkTask.Next();
        if (popTemp->oID == oIDI) return 1;
    }
    return 0;
}

void TWorkcenter::Clear()
{
    int iN;
    TMachine* pmchTemp;
    iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pmchTemp = (TMachine*)Pop();
        // pmchTemp->Clear();
        delete pmchTemp;
    }
    stkTask.Clear();
    wID = 0;
}

TMachine* TWorkcenter::GetID(TMachineID mIDI)
{
    int iN = Num();
    TMachine* pmcTemp;
    Head();
    for (int i = 1; i <= iN; i++)
    {
        pmcTemp = (TMachine*)Current();
        if (pmcTemp->mID == mIDI) return (pmcTemp);
        Next();
    }
    return 0;
}

int TWorkcenter::Remove(TMachine* pmchX)
{
    int iN, i;
    TMachine* pmchTemp = 0;
    iN = Num();
    for (i = 1; i <= iN; i++)
    {
        if (Get(i) == pmchX)
        {
            pmchTemp = (TMachine*)Pop(i);
            // pmchTemp->Clear();
            delete pmchTemp;
            return 1;
        }
    }
    return 0;
}

int TWorkcenter::Remove(int iLocation)
{
    TMachine* pmchTemp;
    if ((iLocation <= Num()) && (iLocation > 0))
    {
        pmchTemp = (TMachine*)Pop(iLocation);
        // pmchTemp->Clear();
        delete pmchTemp;
        return 1;
    }
    else
        return 0;
}

//************************ TWorkcenterList ************************//

TWorkcenterList::TWorkcenterList(TWorkcenterList& wklX)
{
    int i, iN = wklX.Num();
    wklX.Head();
    for (i = 1; i <= iN; i++)
    {
        Append(*(TWorkcenter*)(wklX.Next()));
    }
}

TWorkcenterList& TWorkcenterList::operator=(TWorkcenterList& wklX)
{
    Clear();
    int i, iN = wklX.Num();
    wklX.Head();
    for (i = 1; i <= iN; i++)
    {
        Append(*(TWorkcenter*)(wklX.Next()));
    }
    return *this;
}

TWorkcenter* TWorkcenterList::Append(TWorkcenter& wkcX)
{
    TWorkcenter* pwkcNew = new TWorkcenter(wkcX);
    Push(pwkcNew);
    return pwkcNew;
}

TWorkcenter* TWorkcenterList::GetID(TWorkcenterID wIDI)
{
    int iN = Num();
    TWorkcenter* pwkTemp;
    Head();
    for (int i = 1; i <= iN; i++)
    {
        pwkTemp = (TWorkcenter*)Current();
        if (pwkTemp->wID == wIDI) return (pwkTemp);
        Next();
    }
    return 0;
}

TWorkcenter* TWorkcenterList::Getw_mID(TMachineID mIDI)
{
    int i, iN = Num();
    TWorkcenter* pwkTemp;
    Head();
    for (i = 1; i <= iN; i++)
    {
        pwkTemp = (TWorkcenter*)Current();  // Get(i);
        if (pwkTemp->GetID(mIDI)) return (pwkTemp);
        Next();
    }
    return 0;
}

TMachine* TWorkcenterList::Getm_mID(TMachineID mIDI)
{
    int i, iN = Num();
    TWorkcenter* pwkTemp;
    TMachine* pmcTemp;
    // Head();
    for (i = 1; i <= iN; i++)
    {
        pwkTemp = Get(i);
        // pwkTemp = (TWorkcenter*)Current();
        pmcTemp = pwkTemp->GetID(mIDI);
        if (pmcTemp) return (pmcTemp);
    }
    return 0;
}

TWorkcenter& TWorkcenterList::Getm_oID(TOperationID oIDI)
{  // return list of mc
    TWorkcenter* pWk;
    TWorkcenter* pWkCenter = new TWorkcenter;
    TMachine* pMC;
    int i, iN = Num(), j;
    Head();
    for (i = 1; i <= iN; i++)
    {
        pWk = (TWorkcenter*)Next();  // wkc
        if (pWk->IsTask(oIDI))
        {  // wkc that process oIDI
            pWk->Head();
            for (j = 1; j <= pWk->Num(); j++)
            {
                pMC = (TMachine*)pWk->Next();  // mc
                if (pMC->IsTask(oIDI))
                {
                    pWkCenter->Append(*pMC);
                    break;
                }
            }
        }
    }
    return *pWkCenter;
}

int TWorkcenterList::Remove(int iLocation)
{
    TWorkcenter* pwkcTemp;
    if (iLocation <= Num())
    {
        pwkcTemp = (TWorkcenter*)Pop(iLocation);
        // pwkcTemp->Clear();
        delete pwkcTemp;
        return 1;
    }
    else
        return 0;
}

int TWorkcenterList::RemoveID(TWorkcenterID wIDI)
{
    int iN, i;
    TWorkcenter* pwkcTemp;
    iN = Num();
    Head();
    for (i = 1; i <= iN; i++)
    {
        pwkcTemp = (TWorkcenter*)Current();
        if (pwkcTemp->wID == wIDI)
        {
            Delete();
            // pwkcTemp->Clear();
            delete pwkcTemp;
            return 1;
        }
        Next();
    }
    return 0;
}

void TWorkcenterList::Clear()
{
    int iN;
    iN = Num();
    TWorkcenter* pwkcTemp;
    for (int i = 1; i <= iN; i++)
    {
        pwkcTemp = (TWorkcenter*)Pop();
        // pwkcTemp->Clear();
        delete pwkcTemp;
    }
}

void TWorkcenterList::ClearTask()
{
    int i, j, iN = Num();
    TWorkcenter* pWkCenter;
    TMachine* pMC;
    for (i = 1; i <= iN; i++)
    {
        pWkCenter = Get(i);
        pWkCenter->stkTask.Clear();
        for (j = 1; j <= pWkCenter->Num(); j++)
        {
            pMC = pWkCenter->Get(j);
            pMC->stkTask.Clear();
        }
    }
}

//*** overload iostream ***//

ostream& operator<<(ostream& os, TOperation& opX)
{
    int iN, i;
    os.precision(10);
    os << "Oper. " << opX.oID << " : " << opX.szDescription << endl;
    os << "  r/p/d/act p = (" << opX.fReleaseTime << "/" << opX.fProcessTime << "/" << opX.fDueDate << "/"
       << opX.fActProcessTime << ")" << endl;
    os << " Weight/setup = " << opX.fWeight << "/" << opX.fEWeight << "/" << opX.fSetupTime << endl;
    os << "  begin/end   = " << opX.fStartTime << " : " << opX.fFinishTime << endl;
    os << "   Prior Oper.= ";
    iN = opX.stkPriorOp.Num();
    if (iN > 0)
        os << (opX.GetPrior(1))->oID;
    else
        os << "None";
    for (i = 2; i <= iN; i++) os << "," << (opX.GetPrior(i))->oID;
    os << endl;
    os << "   Post Oper. = ";
    iN = opX.stkPostOp.Num();
    if (iN > 0)
        os << (opX.GetPost(1))->oID;
    else
        os << "None";
    for (i = 2; i <= iN; i++) os << "," << (opX.GetPost(i))->oID;
    os << endl;
    return os;
}

ostream& operator<<(ostream& os, TJob& jbX)
{
    os.precision(10);
    int iM, iL, j, k;
    char szInt5[6];

    TOperation* popTemp;
    jIDtoa(szInt5, jbX.jID, 5);
    os << "Job: " << szInt5 << " (" << jbX.szDescription << ")" << endl;
    os << "  release : " << jbX.fReleaseTime << endl;
    os << "  duedate : " << jbX.fDueDate << endl;
    os << "  weight  : " << jbX.fWeight << "/" << jbX.fEWeight << endl;
    os << "Operations" << endl;
    iM = jbX.Num();
    for (j = 1; j <= iM; j++)
    {
        popTemp = jbX.Get(j);
        oIDtoa(szInt5, popTemp->oID, 5);
        os << "#" << szInt5 << " -- " << popTemp->szDescription << endl;
        os << "  process time   : " << popTemp->fProcessTime << endl;
        os << "  release time   : ";
        if (popTemp->fReleaseTime <= jbX.fReleaseTime)
            os << "-" << endl;
        else
            os << popTemp->fReleaseTime << endl;
        os << "  prior operation: ";
        iL = (popTemp->stkPriorOp).Num();
        if (iL > 0)
            os << ((TOperation*)(popTemp->stkPriorOp)[1])->oID;
        else
            os << "-";
        for (k = 2; k <= iL; k++) os << ", " << ((TOperation*)(popTemp->stkPriorOp)[k])->oID;
        os << endl;
        os << "  process machine: ";
        iL = (popTemp->stkMachine).Num();
        if (iL > 0)
            os << ((TMachine*)(popTemp->stkMachine)[1])->mID;
        else
            os << "-";
        for (k = 2; k <= iL; k++) os << ", " << ((TMachine*)(popTemp->stkMachine)[k])->mID;
        os << endl;
        os << "  machine status : " << popTemp->sMC_Status << endl;
        os << "  user           : " << endl;
    }
    return os;
}

ostream& operator<<(ostream& os, TJobList& jblX)
{
    TJob* pjbTemp;
    int iN = jblX.Num();
    for (int i = 1; i <= iN; i++)
    {
        pjbTemp = jblX.Get(i);
        os << (*pjbTemp);
        os << endl;
    }
    return os;
}

ostream& operator<<(ostream& os, TMachine& mchX)
{
    os.precision(10);
    int iN, i;
    os << "Mach. " << mchX.mID << "  : " << mchX.szDescription << endl;
    os << "   Speed     = " << mchX.fSpeed << endl;
    os << "   Operation = ";
    iN = mchX.stkTask.Num();
    if (iN > 0)
        os << (mchX.stkTask[1]);
    else
        os << "None";
    for (i = 2; i <= iN; i++) os << "," << (mchX.stkTask[i]);
    os << endl;
    return os;
}

ostream& operator<<(ostream& os, TWorkcenter& wkcX)
{
    os.precision(10);
    int iM, iL, j, k;
    char szInt5[6];
    TMachine mcTemp;
    wIDtoa(szInt5, wkcX.wID, 5);
    os << "Workcenter: " << szInt5 << " (" << wkcX.szDescription << ")" << endl;
    os << "  task: ";
    iM = (wkcX.stkTask).Num();
    if (iM > 0) os << (wkcX.GetTask(1))->oID;
    for (j = 2; j <= iM; j++) os << ", " << (wkcX.GetTask(j))->oID;
    os << endl;
    os << "Machines" << endl;
    //***** machines in wkcX
    iM = wkcX.Num();
    if (iM == 0) os << ";" << endl;
    for (j = 1; j <= iM; j++)
    {
        mcTemp = wkcX[j];
        mIDtoa(szInt5, mcTemp.mID, 5);
        os << "#" << szInt5 << " -- " << mcTemp.szDescription << endl;
        os << "  batch size   : " << mcTemp.iBatchSize << endl;
        os << "  speed        : " << mcTemp.fSpeed << endl;
        os << "  task         : ";
        iL = (mcTemp.stkTask).Num();
        if (iL) os << (mcTemp.GetTask(1))->oID;
        for (k = 2; k <= iL; k++) os << ", " << (mcTemp.GetTask(k))->oID;
        os << endl;
        os << "  init status  : " << mcTemp.sInit << endl;
        os << "  avail. time  : " << mcTemp.fAvailTime << endl;
        os << "  user         : " << endl;
        os << mcTemp.suSetup;
        os << ";" << endl;  // end of operation
    }
    return os;
}

ostream& operator<<(ostream& os, TWorkcenterList& wklX)
{
    int iN = wklX.Num();
    for (int i = 1; i <= iN; i++)
    {
        os << wklX[i];
        os << endl;
    }
    return os;
}

ostream& operator<<(ostream& os, TSetup& suX)
{
    int iN, iM, i, j;
    os << "  setup default: " << suX.fDefault << endl;
    iN = suX.Num();
    for (i = 1; i <= iN; i++)
    {
        iM = suX[i].Num();
        for (j = 1; j <= iM; j++)
        {
            os << "        (" << suX[i].sCode << "," << suX[i][j].sCode << ")  : ";
            os << suX[i][j].fTime << endl;
        }
    }
    return os;
}

//***** istream overload *****//

istream& operator>>(istream& is, TJob& jbX)
{
    int iM, j;
    char szTemp[200], cTemp;
    TOperation opTemp;
    TOperationID oIDFrom;

    class cDD
    {
    public:
        TOperationID oFrom, oTo;
    } * pddTemp;

    TStackP stkOp;
    jbX.Clear();
    if (is.ignore(5))
    {
        is.getline(szTemp, 6, ' ');
        jbX.jID = (TJobID)atoi(szTemp);
        is.ignore(1);
        is.getline(szTemp, 31, ')');
        _tcscpy(jbX.szDescription, szTemp);
        is.ignore(13);
        is.getline(szTemp, 20);
        jbX.fReleaseTime = atof(szTemp);
        is.ignore(12);
        is.getline(szTemp, 20);
        jbX.fDueDate = atof(szTemp);
        is.ignore(12);
        is.getline(szTemp, 20, '/');
        jbX.fWeight = atof(szTemp);
        is.getline(szTemp, 20);
        jbX.fEWeight = atof(szTemp);
        is.ignore(11);
        cTemp = is.get();

        while (cTemp == '#')
        {
            opTemp.Clear();
            is.getline(szTemp, 6, ' ');
            opTemp.oID = atoi(szTemp);
            is.ignore(3);
            is.getline(szTemp, 31);
            _tcscpy(opTemp.szDescription, szTemp);
            is.ignore(19);
            is.getline(szTemp, 20);
            opTemp.fProcessTime = atof(szTemp);
            is.ignore(19);
            is.getline(szTemp, 20);
            opTemp.fReleaseTime = atof(szTemp);
            if (opTemp.fReleaseTime == 0) opTemp.fReleaseTime = jbX.fReleaseTime;
            is.ignore(19);
            //***** read prior operation & add them to the stkOp
            is.getline(szTemp, 200);
            iM = strcount(szTemp, ',') + 1;
            for (j = 1; j <= iM; j++)
            {
                oIDFrom = strextc_OperationID(szTemp, ',');
                pddTemp = new cDD;
                pddTemp->oFrom = oIDFrom;
                pddTemp->oTo = opTemp.oID;
                stkOp.Push(pddTemp);
            }

            is.ignore(19);
            is.getline(szTemp, 200);  // skip process machine
            is.ignore(19);
            opTemp.sMC_Status = (TStatus)is.get();

            is.ignore(10);  // ignore "User"
            is.getline(szTemp, 200);

            opTemp.fDueDate = jbX.fDueDate;
            opTemp.fWeight = jbX.fWeight;
            opTemp.fEWeight = jbX.fEWeight;
            jbX.Append(opTemp);
            if (!is) return is;
            cTemp = is.get();
        }
    }
    //***** add precedent relations
    iM = stkOp.Num();
    for (j = 1; j <= iM; j++)
    {
        pddTemp = (cDD*)stkOp.Pop();
        if (pddTemp->oFrom > 0) jbX.Link(pddTemp->oFrom, pddTemp->oTo);
        delete pddTemp;
    }
    return is;
}

istream& operator>>(istream& is, TJobList& jblX)
{
    jblX.Clear();
    TJob jbTemp;
    while (is)
    {
        is >> jbTemp;
        if (jbTemp.jID > 0) jblX.Append(jbTemp);
    }
    return is;
}

istream& operator>>(istream& is, TWorkcenter& wkcX)
{
    /* Tasks will not be read. It will be assigned when the jobs are input.
     */
    char szTemp[200], cTemp;
    TMachine mcTemp;
    TStatus sA, sB;
    wkcX.Clear();
    if (is.ignore(12))
    {
        is.getline(szTemp, 6, ' ');
        wkcX.wID = (TWorkcenterID)atoi(szTemp);
        is.ignore(1);
        is.getline(szTemp, 40, ')');
        _tcscpy(wkcX.szDescription, szTemp);
        is.ignore(9);
        is.getline(szTemp, 200);  // ignore task
        is.getline(szTemp, 20);   // Machines
        cTemp = is.get();
        if (cTemp == ';')  // if no machine, skip 2 spaces.
            is.ignore(2);
        while (cTemp == '#')
        {
            mcTemp.Clear();
            is.getline(szTemp, 6, ' ');
            mcTemp.mID = (TMachineID)atoi(szTemp);
            is.ignore(3);
            is.getline(szTemp, 40);
            _tcscpy(mcTemp.szDescription, szTemp);
            is.ignore(17);
            is.getline(szTemp, 6);
            mcTemp.iBatchSize = atoi(szTemp);
            is.ignore(17);
            is.getline(szTemp, 200);
            mcTemp.fSpeed = atof(szTemp);
            is.ignore(17);
            is.getline(szTemp, 200);  // ignore task
            is.ignore(17);
            is.getline(szTemp, 20);
            mcTemp.sInit = szTemp[0];
            is.ignore(17);
            is.getline(szTemp, 20);
            mcTemp.fAvailTime = atof(szTemp);

            is.ignore(10);  // ignore "User"
            is.getline(szTemp, 200);

            is.ignore(17);
            is.getline(szTemp, 20);
            mcTemp.suSetup.fDefault = atof(szTemp);
            cTemp = is.get();
            while (cTemp != ';')
            {
                is.ignore(8);
                sA = is.get();
                is.ignore(1);
                sB = is.get();
                is.ignore(5);
                is.getline(szTemp, 20);
                (mcTemp.suSetup).Set(sA, sB, atof(szTemp));
                cTemp = is.get();
            }
            wkcX.Append(mcTemp);
            is.ignore(1);
            cTemp = is.get();
        }
    }
    return is;
}

istream& operator>>(istream& is, TWorkcenterList& wklX)
{
    wklX.Clear();
    TWorkcenter wkcTemp;
    while (is)
    {
        is >> wkcTemp;
        if (wkcTemp.wID > 0) wklX.Append(wkcTemp);
    }
    return is;
}
