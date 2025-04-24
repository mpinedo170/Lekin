#include "StdAfx.h"

#include "StackL.h"

////////////////////////////////////////

TStackL::TStackL()
{
    iNum = 0;
    pHead = new TStackElemtL(0, 0, 0);
    pTail = pHead;
    pCurrent = pHead;
    iCurrent = 0;
}

TStackL::TStackL(TStackL& stack)
{
    int iN;
    iNum = 0;
    pHead = new TStackElemtL(0, 0, 0);
    pTail = pHead;
    pCurrent = pHead;
    iCurrent = 0;
    iN = stack.iNum;
    stack.Head();
    for (int i = 1; i <= iN; i++) Push(stack.Next());
}

TStackL& TStackL::operator=(TStackL& stkX)
{
    int iN;
    Clear();
    iN = stkX.Num();
    stkX.Head();
    for (int i = 1; i <= iN; i++) Push(stkX.Next());
    return *this;
}

void TStackL::Clear()
{
    int iN = iNum;
    for (int i = 1; i <= iN; i++) Pop();
}

void TStackL::Push(LData sDataI)
{
    iNum++;
    pTail->pNext = new TStackElemtL(sDataI, pTail, 0);
    pTail = pTail->pNext;
    pCurrent = pTail;
    iCurrent = iNum;
}

LData TStackL::Pop(int iLocation)
{
    LData sReturn = 0;
    if (iLocation == 0) iLocation = iNum;
    operator[](iLocation);
    sReturn = pCurrent->data;
    Delete();
    return sReturn;
}

LData TStackL::PopFirst()
{
    LData sReturn = 0;
    if (iNum > 0)
    {
        sReturn = pHead->pNext->data;
        pHead = pHead->pNext;
        delete (pHead->pPrevious);
        pHead->Set(0, 0, pHead->pNext);
        iNum--;
        iCurrent = 0;
        pCurrent = pHead;
    }
    return sReturn;
}

LData& TStackL::operator[](int iID)
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
                        return pHead->data;
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
        return (pCurrent->data);
    }
    else
        return pHead->data;
}

int TStackL::Delete()
{
    TStackElemtL* pNewLocation;
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

int TStackL::Delete(int iPosition)
{
    if ((iPosition <= iNum) && (iPosition > 0))
    {
        if (iPosition != iCurrent) operator[](iPosition);
        return Delete();
    }
    return 0;
}

void TStackL::Insert(int iLocation, LData sDataI)
{
    TStackElemtL* pTemp;
    if (iLocation < 0)
        iLocation = iCurrent;
    else
        operator[](iLocation);
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemtL(sDataI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

void TStackL::Insert(LData sDataI)
{
    TStackElemtL* pTemp;
    iNum++;
    pTemp = pCurrent->pNext;
    pCurrent->pNext = new TStackElemtL(sDataI, pCurrent, pTemp);
    if (pTemp)
        pTemp->pPrevious = pCurrent->pNext;
    else
        pTail = pTail->pNext;
}

int TStackL::Member(LData sDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == sDataI) return 1;
        Next();
    }
    return 0;
}

int TStackL::Locate(LData sDataI)
{
    Head();
    for (int i = 1; i <= iNum; i++)
    {
        if (Current() == sDataI) return i;
        Next();
    }
    return 0;
}

int TStackL::Swap(int iL1, int iL2)
{
    LData sTemp2;
    TStackElemtL* pTemp1;
    if ((iL1 <= iNum) && (iL1 > 0) && (iL2 <= iNum) && (iL2 > 0) && (iL1 != iL2))
    {
        operator[](iL1);
        pTemp1 = pCurrent;
        operator[](iL2);
        sTemp2 = pCurrent->data;
        pCurrent->data = pTemp1->data;
        pTemp1->data = sTemp2;
        return 1;
    }
    else
        return 0;
}

int TStackL::Move(int iL1, int iL2)
{
    LData sTemp2;
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

int TStackL::SortPartition(int iI, int iJ)
{
    LData sX;
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

void TStackL::QuickSort(int iI, int iJ)
{
    int iQ;
    if (iI < iJ)
    {
        iQ = SortPartition(iI, iJ);
        QuickSort(iI, iQ);
        QuickSort(++iQ, iJ);
    }
}

void TStackL::Head()
{
    pCurrent = pHead;
    iCurrent = 0;
    if (iNum > 0) Next();
}

void TStackL::Tail()
{
    pCurrent = pTail;
    iCurrent = iNum;
}

LData TStackL::Next()
{
    LData rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->data;
    pCurrent = pCurrent->pNext;
    iCurrent++;
    return rtn;
}

LData TStackL::Previous()
{
    LData rtn = 0;
    if (!pCurrent) return 0;
    rtn = pCurrent->data;
    pCurrent = pCurrent->pPrevious;
    iCurrent--;
    return rtn;
}

/////////////////////////////////////////////
