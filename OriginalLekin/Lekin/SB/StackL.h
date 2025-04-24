#pragma once

// TStackL  (unsigned long)
//

typedef unsigned long LData;

class TStackElemtL
{
    friend class TStackL;
    LData data;
    TStackElemtL* pNext;
    TStackElemtL* pPrevious;

public:
    TStackElemtL(LData sDataI, TStackElemtL* pP, TStackElemtL* pN)
    {
        data = sDataI;
        pNext = pN;
        pPrevious = pP;
    };
    void Set(LData sDataI, TStackElemtL* pP, TStackElemtL* pN)
    {
        data = sDataI;
        pNext = pN;
        pPrevious = pP;
    };
};

class TStackL
{
private:
    int iNum;
    int iCurrent;  // current position
    TStackElemtL* pHead;
    TStackElemtL* pTail;
    TStackElemtL* pCurrent;
    int SortPartition(int iI, int iJ);
    void QuickSort(int iI, int iJ);

public:
    TStackL();
    TStackL(TStackL& stack);
    ~TStackL()
    {
        Clear();
    }
    virtual TStackL& operator=(TStackL& stkX);
    void Clear();
    int Num() const
    {
        return iNum;
    }
    void Push(LData sDataI);  // add to the tail
    LData Pop(int iLacation = 0);
    LData PopFirst();
    LData& operator[](int iID);
    int Delete();               // delete current position
    int Delete(int iPosition);  // return 1 = ok, 0 = error (not delete)
    void Set(LData lData)
    {
        pCurrent->data = lData;
    }
    void Set(int iLoc, LData lDataI)
    {
        (*this)[iLoc];
        Set(lDataI);
    }
    void Insert(int iLocation, LData sDataI);
    void Insert(LData sDataI);
    int Member(LData sDataI);
    int Locate(LData sDataI);
    int Swap(int iL1, int iL2);
    int Move(int iL1, int iL2);
    void Sort()
    {
        QuickSort(1, iNum);
    }
    void Head();
    void Tail();
    LData Next();
    LData Previous();
    LData Current()
    {
        return pCurrent ? pCurrent->data : 0;
    }
    int Pos()
    {
        return iCurrent;
    }
};
