#include "StdAfx.h"

#include "Stack.h"

////////////////////////////////////////

TStack::TStack()
{
    iNum = 0;
    pHead = new TStackElemt(0, 0, 0);
    pTail = pHead;
    pCurrent = pHead;
    iCurrent = 0;
}

TStack::TStack(TStack& stack)
{
    int iN;
    iNum = 0;
    pHead = new TStackElemt(0, 0, 0);
    pTail = pHead;
    pCurrent = pHead;
    iCurrent = 0;
    iN = stack.iNum;
    stack.Head();
    for (int i = 1; i <= iN; i++) Push(stack.Next());
}

TStack& TStack::operator=(TStack& stkX)
{
    int iN;
    Clear();
    iN = stkX.Num();
    stkX.Head();
    for (int i = 1; i <= iN; i++) Push(stkX.Next());
    return *this;
}

void TStack::Clear()
{
    TStackElemt* pE;
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        pE = pCurrent->pNext;
        delete pCurrent;
        pCurrent = pE;
    }
    pHead->pNext = 0;
    pTail = pHead;
    iNum = 0;
}

void TStack::Push(SData sDataI)
{
    iNum++;
    pTail->pNext = new TStackElemt(sDataI, pTail, 0);
    pTail = pTail->pNext;
    pCurrent = pTail;
    iCurrent = iNum;
}

SData TStack::Pop()
{
    Tail();
    SData sReturn = pTail->sData;
    if (iNum > 0)
    {
        pTail = pTail->pPrevious;
        delete pTail->pNext;
        pTail->pNext = 0;
        iCurrent = --iNum;
        pCurrent = pTail;
    }
    return sReturn;
}

SData TStack::Pop(int iLocation)
{
    SData sReturn = 0;
    if (iLocation == 0) iLocation = iNum;
    operator[](iLocation);
    sReturn = pCurrent->sData;
    Delete();
    return sReturn;
}

SData TStack::PopFirst()
{
    SData sReturn = 0;
    if (iNum > 0)
    {
        sReturn = pHead->pNext->sData;
        pHead = pHead->pNext;
        delete (pHead->pPrevious);
        pHead->Set(0, 0, pHead->pNext);
        iNum--;
        iCurrent = 0;
        pCurrent = pHead;
    }
    return sReturn;
}

SData& TStack::operator[](int iID)
{
    int i;
    int iFrom = 0, iDistance, iTemp;
    if (iCurrent > iNum)
    {
        iCurrent = iNum;
        pCurrent = pTail;
    }
    if (iNum > 0)
    {
        if (iID != iCurrent)
        {
            if (iID > iNum)
                iID = iNum;
            else if (iID < 0)
                iID = 1;
            // find where to start 0:Head 1:Tail 2:Current- 3:Current+
            iDistance = iID;
            iTemp = iNum - iID;
            if (iTemp < iDistance)
            {
                iFrom = 1;
                iDistance = iTemp;
            }
            iTemp = iID - iCurrent;
            if (iTemp < 0)
            {
                iTemp = -iTemp;
                if (iTemp < iDistance)
                {
                    iFrom = 2;
                    iDistance = iTemp;
                }
            }
            else if (iTemp < iDistance)
            {
                iFrom = 3;
                iDistance = iTemp;
            }
            iCurrent = iID;
            switch (iFrom)
            {
                case 0:
                    pCurrent = pHead;
                    if (iDistance > 0)
                        for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pNext;
                    else
                        return pHead->sData;
                    break;
                case 1:
                    pCurrent = pTail;
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pPrevious;
                    break;
                case 2:
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pPrevious;
                    break;
                case 3:
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pNext;
                    break;
            }
        }
        return (pCurrent->sData);
    }
    else
        return pHead->sData;
}

int TStack::Delete()
{
    TStackElemt* pNewLocation;
    if (pCurrent)
    {
        if (pCurrent->pNext)
        {
            pCurrent->pNext->pPrevious = pCurrent->pPrevious;
            pCurrent->pPrevious->pNext = pCurrent->pNext;
        }
        else
        {
            pCurrent->pPrevious->pNext = 0;
            pTail = pCurrent->pPrevious;
        }
        pNewLocation = pCurrent->pPrevious;
        delete pCurrent;
        iCurrent--;
        pCurrent = pNewLocation;
        iNum--;
        return 1;
    }
    return 0;
}

int TStack::Delete(int iPosition)
{
    if ((iPosition <= iNum) && (iPosition > 0))
    {
        if (iPosition != iCurrent) operator[](iPosition);
        return Delete();
    }
    return 0;
}

void TStack::Insert(int iLocation, SData sDataI)
{
    TStackElemt* pTemp;
    if (iLocation < 0)
        iLocation = iCurrent;
    else
        operator[](iLocation);
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemt(sDataI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

void TStack::Insert(SData sDataI)
{
    TStackElemt* pTemp;
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemt(sDataI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

int TStack::Member(SData sDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == sDataI) return 1;
        Next();
    }
    return 0;
}

int TStack::Locate(SData sDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == sDataI) return i;
        Next();
    }
    return 0;
}

int TStack::Swap(int iL1, int iL2)
{
    SData sTemp2;
    TStackElemt* pTemp1;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 > 0) && (iL1 != iL2))
    {
        operator[](iL1);
        pTemp1 = pCurrent;
        operator[](iL2);
        sTemp2 = pCurrent->sData;
        pCurrent->sData = pTemp1->sData;
        pTemp1->sData = sTemp2;
        return 1;
    }
    else
        return 0;
}

int TStack::Move(int iL1, int iL2)
{
    SData sTemp2;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 >= 0) && (iL1 != iL2))
    {
        if (iL1 < iL2) iL2--;
        sTemp2 = Pop(iL1);
        Insert(iL2, sTemp2);
        return 1;
    }
    else
        return 0;
}

int TStack::SortPartition(int iI, int iJ)
{
    SData sX;
    sX = operator[](iI);
    int iCont = 1;
    while (iCont)
    {
        while (operator[](iJ) > sX) iJ--;
        while (operator[](iI) < sX) iI++;
        if (iI < iJ)
        {
            Swap(iI, iJ);
            iJ--;
            iI++;
        }
        else
            iCont = 0;
    }
    return iJ;
}

void TStack::QuickSort(int iI, int iJ)
{
    int iQ;
    if (iI < iJ)
    {
        iQ = SortPartition(iI, iJ);
        QuickSort(iI, iQ);
        QuickSort(++iQ, iJ);
    }
}

SData TStack::Next()
{
    SData rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->sData;
    if (iCurrent < iNum)
    {
        pCurrent = pCurrent->pNext;
        iCurrent++;
    }
    return rtn;
}

SData TStack::Previous()
{
    SData rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->sData;
    pCurrent = pCurrent->pPrevious;
    iCurrent--;
    return rtn;
}

/////////////////////////////////////////////

TStackF::TStackF(TStackF& stack)
{
    int iN;
    iNum = 0;
    pHead = new TStackElemtF(0, 0, 0);
    pTail = pHead;
    pCurrent = pHead;
    iCurrent = 0;
    iN = stack.iNum;
    stack.Head();
    for (int i = 1; i <= iN; i++) Push(stack.Next());
}

TStackF& TStackF::operator=(TStackF& stkX)
{
    int iN;
    Clear();
    iN = stkX.Num();
    stkX.Head();
    for (int i = 1; i <= iN; i++) Push(stkX.Next());
    return *this;
}

void TStackF::Clear()
{
    TStackElemtF* pE;
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        pE = pCurrent->pNext;
        delete pCurrent;
        pCurrent = pE;
    }
    pHead->pNext = 0;
    pTail = pHead;
    iNum = 0;
}

void TStackF::Push(double fDataI)
{
    iNum++;
    pTail->pNext = new TStackElemtF(fDataI, pTail, 0);
    pTail = pTail->pNext;
    pCurrent = pTail;
    iCurrent = iNum;
}

double TStackF::Pop()
{
    Tail();
    double fReturn = pTail->fData;
    if (iNum > 0)
    {
        pTail = pTail->pPrevious;
        delete pTail->pNext;
        pTail->pNext = 0;
        iCurrent = --iNum;
        pCurrent = pTail;
    }
    return fReturn;
}

double TStackF::Pop(int iLocation)
{
    double fReturn = 0;
    if (iLocation == 0) iLocation = iNum;
    operator[](iLocation);
    fReturn = pCurrent->fData;
    Delete();
    return fReturn;
}

double TStackF::PopFirst()
{
    double fReturn = 0;
    if (iNum > 0)
    {
        fReturn = pHead->pNext->fData;
        pHead = pHead->pNext;
        delete (pHead->pPrevious);
        pHead->Set(0, 0, pHead->pNext);
        iNum--;
        iCurrent = 0;
        pCurrent = pHead;
    }
    return fReturn;
}

double& TStackF::operator[](int iID)
{
    int i;
    int iFrom = 0, iDistance, iTemp;
    if (iNum > 0)
    {
        if (iID != iCurrent)
        {
            if (iID > iNum)
                iID = iNum;
            else if (iID < 0)
                iID = 1;
            // find where to start 0:Head 1:Tail 2:Current- 3:Current+
            iDistance = iID;
            iTemp = iNum - iID;
            if (iTemp < iDistance)
            {
                iFrom = 1;
                iDistance = iTemp;
            }
            iTemp = iID - iCurrent;
            if (iTemp < 0)
            {
                iTemp = -iTemp;
                if (iTemp < iDistance)
                {
                    iFrom = 2;
                    iDistance = iTemp;
                }
            }
            else if (iTemp < iDistance)
            {
                iFrom = 3;
                iDistance = iTemp;
            }
            iCurrent = iID;
            switch (iFrom)
            {
                case 0:
                    pCurrent = pHead;
                    if (iDistance > 0)
                        for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pNext;
                    else
                        return pHead->fData;
                    break;
                case 1:
                    pCurrent = pTail;
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pPrevious;
                    break;
                case 2:
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pPrevious;
                    break;
                case 3:
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pNext;
                    break;
            }
        }
        return (pCurrent->fData);
    }
    else
        return pHead->fData;
}

int TStackF::Delete()
{
    TStackElemtF* pNewLocation;
    if (pCurrent)
    {
        if (pCurrent->pNext)
        {
            pCurrent->pNext->pPrevious = pCurrent->pPrevious;
            pCurrent->pPrevious->pNext = pCurrent->pNext;
        }
        else
        {
            pCurrent->pPrevious->pNext = 0;
            pTail = pCurrent->pPrevious;
        }
        pNewLocation = pCurrent->pPrevious;
        delete pCurrent;
        iCurrent--;
        pCurrent = pNewLocation;
        iNum--;
        return 1;
    }
    return 0;
}

int TStackF::Delete(int iPosition)
{
    if ((iPosition <= iNum) && (iPosition > 0))
    {
        if (iPosition != iCurrent) operator[](iPosition);
        return Delete();
    }
    return 0;
}

void TStackF::Insert(int iLocation, double fDataI)
{
    TStackElemtF* pTemp;
    if (iLocation < 0)
        iLocation = iCurrent;
    else
        operator[](iLocation);
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemtF(fDataI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

void TStackF::Insert(double fDataI)
{
    TStackElemtF* pTemp;
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemtF(fDataI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

int TStackF::Member(double fDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == fDataI) return 1;
        Next();
    }
    return 0;
}

int TStackF::Locate(double fDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == fDataI) return i;
        Next();
    }
    return 0;
}

int TStackF::Swap(int iL1, int iL2)
{
    double fTemp2;
    TStackElemtF* pTemp1;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 > 0) && (iL1 != iL2))
    {
        operator[](iL1);
        pTemp1 = pCurrent;
        operator[](iL2);
        fTemp2 = pCurrent->fData;
        pCurrent->fData = pTemp1->fData;
        pTemp1->fData = fTemp2;
        return 1;
    }
    else
        return 0;
}

int TStackF::Move(int iL1, int iL2)
{
    double fTemp2;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 >= 0) && (iL1 != iL2))
    {
        if (iL1 < iL2) iL2--;
        fTemp2 = Pop(iL1);
        Insert(iL2, fTemp2);
        return 1;
    }
    else
        return 0;
}

int TStackF::SortPartition(int iI, int iJ)
{
    double fX;
    fX = operator[](iI);
    int iCont = 1;
    while (iCont)
    {
        while (operator[](iJ) > fX) iJ--;
        while (operator[](iI) < fX) iI++;
        if (iI < iJ)
        {
            Swap(iI, iJ);
            iJ--;
            iI++;
        }
        else
            iCont = 0;
    }
    return iJ;
}

void TStackF::QuickSort(int iI, int iJ)
{
    int iQ;
    if (iI < iJ)
    {
        iQ = SortPartition(iI, iJ);
        QuickSort(iI, iQ);
        QuickSort(++iQ, iJ);
    }
}

double TStackF::Next()
{
    double rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->fData;
    if (iCurrent < iNum)
    {
        pCurrent = pCurrent->pNext;
        iCurrent++;
    }
    return rtn;
}

double TStackF::Previous()
{
    double rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->fData;
    pCurrent = pCurrent->pPrevious;
    iCurrent--;
    return rtn;
}

/////////////////////////////////////

TStackP::TStackP(TStackP& stkX)
{
    int iN;
    iNum = 0;
    pHead = new TStackElemtP(0, 0, 0, 0);
    pTail = pHead;
    pCurrent = pHead;
    iCurrent = 0;
    iN = stkX.iNum;
    stkX.Head();
    for (int i = 1; i <= iN; i++)
    {
        Push(stkX.Current());
        SetIndex(stkX.pCurrent->fIndex);
        stkX.Next();
    }
}

TStackP& TStackP::operator=(TStackP& stkX)
{
    int iN;
    Clear();
    iN = stkX.Num();
    stkX.Head();
    for (int i = 1; i <= iN; i++)
    {
        Push(stkX.Current());
        SetIndex(stkX.pCurrent->fIndex);
        stkX.Next();
    }
    return *this;
}

void TStackP::Push(void* pDataI, double fIndexI)
{
    iNum++;
    pTail->pNext = new TStackElemtP(pDataI, fIndexI, pTail, 0);
    pTail = pTail->pNext;
    pCurrent = pTail;
    iCurrent = iNum;
}

void TStackP::Clear()
{
    TStackElemtP* pE;
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        pE = pCurrent->pNext;
        delete pCurrent;
        pCurrent = pE;
    }
    pHead->pNext = 0;
    pTail = pHead;
    iNum = 0;
}

void* TStackP::Pop()
{
    Tail();
    void* pReturn = pTail->pData;
    if (iNum > 0)
    {
        pTail = pTail->pPrevious;
        delete pTail->pNext;
        pTail->pNext = 0;
        iCurrent = --iNum;
        pCurrent = pTail;
    }
    return pReturn;
}

void* TStackP::Pop(int iLocation)
{
    void* pReturn = 0;
    if (iLocation == 0) iLocation = iNum;
    if ((iLocation > 0) && (iLocation <= iNum)) operator[](iLocation);
    pReturn = pCurrent->pData;
    Delete();
    return pReturn;
}

void* TStackP::PopFirst()
{
    void* pReturn = 0;
    if (iNum > 0)
    {
        pReturn = pHead->pNext->pData;
        pHead = pHead->pNext;
        delete (pHead->pPrevious);
        pHead->Set(0, 0, pHead->pNext);
        iNum--;
        iCurrent = 0;
        pCurrent = pHead;
    }
    return pReturn;
}

void* TStackP::operator[](int iID)
{
    int i;
    int iFrom = 0, iDistance, iTemp;
    if (iNum > 0)
    {
        if (iID != iCurrent)
        {
            if (iID > iNum)
                iID = iNum;
            else if (iID < 0)
                iID = 1;
            // find where to start 0:Head 1:Tail 2:Current- 3:Current+
            iDistance = iID;
            iTemp = iNum - iID;
            if (iTemp < iDistance)
            {
                iFrom = 1;
                iDistance = iTemp;
            }
            iTemp = iID - iCurrent;
            if (iTemp < 0)
            {
                iTemp = -iTemp;
                if (iTemp < iDistance)
                {
                    iFrom = 2;
                    iDistance = iTemp;
                }
            }
            else if (iTemp < iDistance)
            {
                iFrom = 3;
                iDistance = iTemp;
            }
            iCurrent = iID;
            switch (iFrom)
            {
                case 0:
                    pCurrent = pHead;
                    if (iDistance > 0)
                        for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pNext;
                    else
                        return 0;
                    break;
                case 1:
                    pCurrent = pTail;
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pPrevious;
                    break;
                case 2:
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pPrevious;
                    break;
                case 3:
                    for (i = 1; i <= iDistance; i++) pCurrent = pCurrent->pNext;
                    break;
            }
        }
        return (pCurrent->pData);
    }
    else
        return 0;
}

int TStackP::Delete()
{
    TStackElemtP* pNewLocation;
    if (pCurrent)
    {
        if (pCurrent->pNext)
        {
            pCurrent->pNext->pPrevious = pCurrent->pPrevious;
            pCurrent->pPrevious->pNext = pCurrent->pNext;
        }
        else
        {
            pCurrent->pPrevious->pNext = 0;
            pTail = pCurrent->pPrevious;
        }
        pNewLocation = pCurrent->pPrevious;
        delete pCurrent;
        iCurrent--;
        pCurrent = pNewLocation;
        iNum--;
        return 1;
    }
    return 0;
}

int TStackP::Delete(int iPosition)
{
    if ((iPosition <= iNum) && (iPosition > 0))
    {
        if (iPosition != iCurrent) operator[](iPosition);
        return Delete();
    }
    return 0;
}

void TStackP::Insert(int iLocation, void* pDataI, double fIndexI)
{
    TStackElemtP* pTemp;
    if (iLocation < 0)
        iLocation = iCurrent;
    else
        operator[](iLocation);
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemtP(pDataI, fIndexI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

void TStackP::Insert(void* pDataI, double fIndexI)
{
    TStackElemtP* pTemp;
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemtP(pDataI, fIndexI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

int TStackP::Member(void* pDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == pDataI) return 1;
        Next();
    }
    return 0;
}

int TStackP::Locate(void* pDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == pDataI) return i;
        Next();
    }
    return 0;
}

int TStackP::SetIndex(double fIndexI)
{
    if (pCurrent)
    {
        pCurrent->fIndex = fIndexI;
        return 1;
    }
    else
        return 0;
}

int TStackP::SetIndex(int iLocation, double fIndexI)
{
    if (iLocation == 0) iLocation = iCurrent;
    if ((iLocation <= iNum) && (iLocation > 0))
    {
        if (iLocation != iCurrent) operator[](iLocation);
        pCurrent->fIndex = fIndexI;
        return 1;
    }
    else
        return 0;
}

double TStackP::Index(int iLocation)
{
    if (iLocation == 0) iLocation = iCurrent;
    operator[](iLocation);
    return (pCurrent->fIndex);
}

int TStackP::Swap(int iL1, int iL2)
{
    void* pTemp2;
    double fTempIndex;
    TStackElemtP* pTemp1;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 > 0) && (iL1 != iL2))
    {
        operator[](iL1);
        pTemp1 = pCurrent;
        operator[](iL2);
        pTemp2 = pCurrent->pData;
        fTempIndex = pCurrent->fIndex;
        pCurrent->pData = pTemp1->pData;
        pCurrent->fIndex = pTemp1->fIndex;
        pTemp1->pData = pTemp2;
        pTemp1->fIndex = fTempIndex;
        return 1;
    }
    else
        return 0;
}

int TStackP::Move(int iL1, int iL2)
{
    void* pTemp2;
    double fTemp2;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 >= 0) && (iL1 != iL2))
    {
        if (iL1 < iL2) iL2--;
        fTemp2 = Index(iL1);
        pTemp2 = Pop(iL1);
        Insert(iL2, pTemp2);
        SetIndex(++iL2, fTemp2);
        return 1;
    }
    else
        return 0;
}

int TStackP::SortPartition(int iI, int iJ)
{
    double fX;
    fX = Index(iI);
    int iCont = 1;
    while (iCont)
    {
        while (Index(iJ) > fX) iJ--;
        while (Index(iI) < fX) iI++;
        if (iI < iJ)
        {
            Swap(iI, iJ);
            iJ--;
            iI++;
        }
        else
            iCont = 0;
    }
    return iJ;
}

void TStackP::QuickSort(int iI, int iJ)
{
    int iQ;
    if (iI < iJ)
    {
        iQ = SortPartition(iI, iJ);
        QuickSort(iI, iQ);
        QuickSort(++iQ, iJ);
    }
}

void* TStackP::Next()
{
    void* rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->pData;
    if (iCurrent < iNum)
    {
        pCurrent = pCurrent->pNext;
        iCurrent++;
    }
    return rtn;
}

void* TStackP::Previous()
{
    void* rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->pData;
    pCurrent = pCurrent->pPrevious;
    iCurrent--;
    return rtn;
}

void TStackP::InsertSort(void* pDataI, double fIndexI)
{
    if (iNum == 0)
    {
        Push(pDataI, fIndexI);
        return;
    }

    // find position
    double fMax = pTail->fIndex, fMid;

    if (fIndexI >= fMax)
    {
        pCurrent = pTail;
        iCurrent = iNum;
        Push(pDataI, fIndexI);
        return;
    }

    pCurrent = pHead;
    iCurrent = 0;
    while (1)
    {
        pCurrent = pCurrent->pNext;
        iCurrent++;
        fMid = pCurrent->fIndex;
        if (fMid > fIndexI)
        {
            pCurrent = pCurrent->pPrevious;
            iCurrent--;
            Insert(pDataI, fIndexI);
            return;
        }
    }
}