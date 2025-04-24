#include "StdAfx.h"

#include "Sequence.h"

void Trace(LPCSTR szName, TSequenceList& sql)
{
    ofstream myfile(szName);
    myfile << sql;
    myfile.close();
}

void Trace(LPCSTR szName, TSequence& sq)
{
    ofstream myfile(szName);
    myfile << sq;
    myfile.close();
}

TSequence::TSequence(TSequence& seqX)
{
    int iN = seqX.Num();
    mID = seqX.mID;
    for (int i = 1; i <= iN; i++) Push(seqX[i]);
}

TSequence& TSequence::operator=(TSequence& seqX)
{
    Clear();
    int iN = seqX.Num();
    mID = seqX.mID;
    for (int i = 1; i <= iN; i++) Push(seqX[i]);
    return (*this);
}

int TSequence::operator==(TSequence& seqX)
{
    // return 1 only if the sequences are the same regardless of machine
    if (Num() != seqX.Num()) return 0;
    int i;
    for (i = 1; i <= Num(); i++)
    {
        if ((*this)[i] != seqX[i]) return 0;
    }
    return 1;
}

void TSequence::Append(TSequence& szX)
{
    for (int i = 1; i <= szX.Num(); i++) Append(szX[i]);
}

void TSequence::Remove0()
{
    int i;
    Head();
    for (i = 1; i <= Num(); i++)
    {
        if (Current() == 0)
        {
            Delete();
            i = max(i - 1, 1);
        }
        Next();
    }
}

//////////////////////////// TSequenceList //////////////////////////
TSequenceList::TSequenceList(TSequenceList& sql)
{
    Append(sql);
}

TSequenceList& TSequenceList::operator=(TSequenceList& sql)
{
    Clear();
    Append(sql);
    return *this;
}

void TSequenceList::Append(TMachineID mIDI, TOperationID oIDI)
{
    TSequence* pseqTemp;
    pseqTemp = Gets_mID(mIDI);
    if (!pseqTemp)
    {
        pseqTemp = new TSequence(mIDI);
        Push(pseqTemp);
    }
    // if(!pseqTemp->Member(oIDI))
    pseqTemp->Append(oIDI);
}

void TSequenceList::Append(TMachineID mIDI, TStack stkOper)
{
    int iM = stkOper.Num();
    TOperationID oIDI;
    TSequence* pseqTemp;
    pseqTemp = Gets_mID(mIDI);
    if (!pseqTemp)
    {
        pseqTemp = new TSequence(mIDI);
        Push(pseqTemp);
    }
    for (int i = 1; i <= iM; i++)
    {
        oIDI = (TOperationID)stkOper[i];
        //  if(!pseqTemp->Member(oIDI))
        pseqTemp->Append(oIDI);
    }
}

TSequence* TSequenceList::Append(TSequence& sqX)
{
    if (&sqX == 0) return 0;
    int iM = sqX.Num();
    TOperationID oIDI;
    TSequence* pseqTemp;
    pseqTemp = Gets_mID(sqX.mID);
    if (!pseqTemp)
    {
        pseqTemp = new TSequence(sqX.mID);
        Push(pseqTemp);
    }
    for (int i = 1; i <= iM; i++)
    {
        oIDI = (TOperationID)sqX[i];
        //  if(!pseqTemp->Member(oIDI))
        pseqTemp->Append(oIDI);
    }
    return pseqTemp;
}

void TSequenceList::Append(TSequenceList& sqlX)
{
    if (&sqlX == 0) return;
    int iN = sqlX.Num();
    for (int i = 1; i <= iN; i++)
    {
        Append(sqlX[i]);
    }
}

TSequence* TSequenceList::Get(int iLocation)
{
    return ((TSequence*)TStackP::operator[](iLocation));
}

TSequence* TSequenceList::Gets_mID(TMachineID mIDI)
{
    int iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        if (Get(i)->mID == mIDI) return (Get(i));
    }
    return 0;
}

TSequence* TSequenceList::Gets_oID(TOperationID oIDI)
{
    TSequence* pSequence;
    int iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pSequence = Get(i);
        if (pSequence->Member(oIDI)) return (pSequence);
    }
    return 0;
}

int TSequenceList::Remove(int iLocation)
{
    TSequence* psqTemp;
    if ((iLocation <= Num()) && (iLocation > 0))
    {
        psqTemp = (TSequence*)Pop(iLocation);
        psqTemp->Clear();
        delete psqTemp;
        return 1;
    }
    return 0;
}

int TSequenceList::Remove(TSequence* psqX)
{
    int iLoc = Locate(psqX);
    return Remove(iLoc);
}

int TSequenceList::RemoveID(TMachineID mIDI)
{
    TSequence* pseqTemp;
    int iN = Num();
    for (int i = 1; i <= iN; i++)
        if (Get(i)->mID == mIDI)
        {
            pseqTemp = (TSequence*)Pop(i);
            pseqTemp->Clear();
            return 1;
        }
    return 0;
}

void TSequenceList::Clear()
{
    int iN = Num();
    TSequence* pseqTemp;
    for (int i = 1; i <= iN; i++)
    {
        pseqTemp = (TSequence*)Pop();
        // pseqTemp->Destroy();
        delete pseqTemp;
    }
}

TOperationID TSequenceList::GetLast(TMachineID mID)
{
    TSequence* pSeq = Gets_mID(mID);
    return pSeq ? pSeq->GetLast() : 0;
}

//*** overload iostream ***//

ostream& operator<<(ostream& os, TSequence& seqX)
{
    int iM, j;
    char szInt5[6];
    mIDtoa(szInt5, seqX.mID, 5);
    os << "Machine " << szInt5 << ": ";
    iM = seqX.Num();
    if (iM > 0) os << seqX[1];
    for (j = 2; j <= iM; j++) os << ", " << seqX[j];
    os << endl;
    return os;
}

ostream& operator<<(ostream& os, TSequenceList& sqlX)
{
    int iN = sqlX.Num();
    for (int i = 1; i <= iN; i++) os << *(sqlX.Get(i));
    return os;
}

//*** overload istream ***//

istream& operator>>(istream& is, TSequenceList& sqlX)
{
    int i, iM, iTemp;
    char szInt5[6], szTemp[200];
    TSequence seqTemp;
    sqlX.Clear();
    while (is)
    {
        seqTemp.Clear();
        if (is.ignore(8))
        {
            is.getline(szInt5, 7, ':');
            seqTemp.mID = (TMachineID)atoi(szInt5);
            is.ignore(1);
            is.getline(szTemp, 200);
            iM = strcount(szTemp, ',') + 1;
            for (i = 1; i <= iM; i++)
            {
                iTemp = strextc_JobID(szTemp, ',');
                if (!((iM == 1) && (iTemp == 0))) seqTemp.Append(iTemp);
            }
            sqlX.Append(seqTemp);
        }
    }
    return is;
}
