#pragma once

/*  Stack.h  , February 16, 1996
    There are 3 types of stack classes :
        TStack   : integer stack
        TStackF : double stack
        TStackP : (void)pointer stack
    Commands
    ========
  d operator[](x): move pCurrent to the xth element & return the value {0-Num()}.
  - Clear()      : clear the stack
  i Num()        : return number of item in the stack.
  - Push(xx)     : append data and move pCurrent to the Tail.
  d Pop()        : return the tail element & remove it.
  d Pop(x)       : return the x element & remove it.
  d PopFirst()   : return the head element & remove it.
  b Delete()     : remove the element at pCurrent; return 1=success,0=failure
  b Delete(x)    : remove the x element; return 1=success,0=failure
  - Set(xx)      : set data at iCurrent position
  - Set(x,xx)    : set data at x position
  - Insert(xx)   : insert data at the iCurrent(pCurrent) position.
  - Insert(x,xx) : insert data xx at x position.
  b Member(xx)   : check if xx is a member in the stack; return 1/0;
  i Locate(xx)   : return the location that contain xx, return 0 if not found.
  b Swap(x,y)    : swap the data in location x & y; return 1/0.
  b Move(x,y)    : move data from x to the location just after y; return 1/0.
  b Move(0,Num()): move the data at pCurrent to the tail; return 1/0.
  b Move(0,y)    : move the data at pCurrent to the location just after y; return 1/0.
  - Sort()       : quicksort the data (or fIndex for TStackP) ascendently.
  - Head()       : move pCurrent to head of the list.
  - Tail()       : move pCurrent to tail.
  d Next()       : return the data & move to the next location.
  d Previous()   : return the data then move back.
  d Current()    : return the data.
  i Pos()        : return the position.

  TStackP
  - Push(px,fy)  : append data (px) and index (fy) and move pCurrent to the Tail.
  - Push(px)     : append data (px) /w index = 0.
  - Insert(px,fx): insert data (px) and index (fx) at current position.
  - Insert(px)   : insert data (px) /w index = 0 at current position.
  - Insert(x,px,fx) : insert data (px) and index (fx) at x position.
  - Insert(x,px)    : insert data (px) /w index = 0 at x position.
  d Index()       : return the fIndex in pCurrent location.
  d Index(x)      : return the fIndex at location x.
  b SetIndex(xx)  : assign pCurrent location the xx index value; return 1/0
  b SetIndex(x,xx): assign the x location, the xx index value; return 1/0
  - InsertSort(px,fx): insert data (px) and index (fx) in a way to create ascending order
*/

typedef int SData;

class TStackElemt
{
    friend class TStack;
    SData sData;
    TStackElemt* pNext;
    TStackElemt* pPrevious;

public:
    TStackElemt(SData _sData, TStackElemt* pP, TStackElemt* pN)
    {
        sData = _sData;
        pNext = pN;
        pPrevious = pP;
    }
    void Set(SData _sData, TStackElemt* pP, TStackElemt* pN)
    {
        sData = _sData;
        pNext = pN;
        pPrevious = pP;
    }
};

class TStack
{
private:
    int iNum;
    int iCurrent;  // current position
    TStackElemt* pHead;
    TStackElemt* pTail;
    TStackElemt* pCurrent;

    int SortPartition(int iI, int iJ);
    void QuickSort(int iI, int iJ);

public:
    TStack();
    TStack(TStack& stack);
    virtual ~TStack()
    {
        Clear();
        delete pHead;
    };
    virtual TStack& operator=(TStack& stkX);
    virtual void Clear();
    void Destroy()
    {
        Clear();
        delete pHead;
    }
    void PopHead()
    {
        delete pHead;
    }
    int Num() const
    {
        return iNum;
    }
    void Push(SData sDataI);  // add to the tail
    SData Pop();
    SData Pop(int iLacation);
    SData PopFirst();
    SData& operator[](int iID);
    void Set(SData sData)
    {
        pCurrent->sData = sData;
    }
    void Set(int iLoc, SData sData)
    {
        (*this)[iLoc];
        Set(sData);
    }

    int Delete();               // delete current position
    int Delete(int iPosition);  // return 1 = ok, 0 = error (not delete)
    void Insert(int iLocation, SData sDataI);
    void Insert(SData sDataI);
    int Member(SData sDataI);
    int Locate(SData sDataI);
    int Swap(int iL1, int iL2);
    int Move(int iL1, int iL2);
    SData Next();
    SData Previous();

    void Sort()
    {
        QuickSort(1, iNum);
    }
    void Head()
    {
        pCurrent = pHead;
        iCurrent = 0;
        if (iNum > 0) Next();
    }
    void Tail()
    {
        pCurrent = pTail;
        iCurrent = iNum;
    }
    SData Current()
    {
        return pCurrent ? pCurrent->sData : 0;
    }
    int Pos()
    {
        return iCurrent;
    }
};

class TStackElemtF
{
    friend class TStackF;
    double fData;
    TStackElemtF *pNext, *pPrevious;

public:
    TStackElemtF(double fDataI, TStackElemtF* pP, TStackElemtF* pN)
    {
        fData = fDataI;
        pNext = pN;
        pPrevious = pP;
    };
    void Set(double fDataI, TStackElemtF* pP, TStackElemtF* pN)
    {
        fData = fDataI;
        pNext = pN;
        pPrevious = pP;
    };
};

class TStackF
{
private:
    int iNum, iCurrent;
    TStackElemtF *pHead, *pTail, *pCurrent;
    int SortPartition(int iI, int iJ);
    void QuickSort(int iI, int iJ);

public:
    TStackF()
    {
        iNum = 0;
        pHead = new TStackElemtF(0, 0, 0);
        pTail = pHead;
        pCurrent = pHead;
        iCurrent = 0;
    };
    TStackF(TStackF& stack);
    ~TStackF()
    {
        Clear();
        delete (TStackElemtF*)pHead;
    };
    virtual TStackF& operator=(TStackF& stkX);
    void Clear();
    void Destroy()
    {
        Clear();
        delete (TStackElemtF*)pHead;
    };
    void PopHead()
    {
        delete (TStackElemtF*)pHead;
    };
    int Num() const
    {
        return iNum;
    };
    void Push(double fDataI);
    double Pop();
    double Pop(int iLocation);
    double PopFirst();
    double& operator[](int iID);
    int Delete();
    int Delete(int iPosition);
    void Set(double fDataI)
    {
        pCurrent->fData = fDataI;
    };
    void Set(int iLoc, double fDataI)
    {
        (*this)[iLoc];
        Set(fDataI);
    };
    void Insert(int iLocation, double fDataI);
    void Insert(double fDataI);
    int Member(double fDataI);
    int Locate(double fDataI);
    int Swap(int iL1, int iL2);
    int Move(int iL1, int iL2);
    void Sort()
    {
        QuickSort(1, iNum);
    };
    void Head()
    {
        pCurrent = pHead;
        iCurrent = 0;
        if (iNum > 0) Next();
    };
    void Tail()
    {
        pCurrent = pTail;
        iCurrent = iNum;
    };
    double Next();
    double Previous();
    double Current()
    {
        if (pCurrent) return pCurrent->fData;
        return 0;
    };
    int Pos()
    {
        return iCurrent;
    };
};

class TStackElemtP
{
    friend class TStackP;
    void* pData;
    double fIndex;
    TStackElemtP *pNext, *pPrevious;

public:
    TStackElemtP(void* pDataI, double fIndexI, TStackElemtP* pP, TStackElemtP* pN)
    {
        pData = pDataI;
        fIndex = fIndexI;
        pNext = pN;
        pPrevious = pP;
    };
    void Set(void* pDataI, TStackElemtP* pP, TStackElemtP* pN)
    {
        pData = pDataI;
        pNext = pN;
        pPrevious = pP;
    };
    void SetIndex(double fIndexI)
    {
        fIndex = fIndexI;
    };
};

class TStackP
{
private:
    int iNum, iCurrent;
    TStackElemtP* pHead;
    TStackElemtP* pTail;
    TStackElemtP* pCurrent;
    int SortPartition(int iI, int iJ);
    void QuickSort(int iI, int iJ);

public:
    TStackP()
    {
        iNum = 0;
        pHead = new TStackElemtP(0, 0, 0, 0);
        pTail = pHead;
        pCurrent = pHead;
        iCurrent = 0;
    }
    TStackP(TStackP& stack);
    virtual ~TStackP()
    {
        Clear();
        delete pHead;
    }
    virtual TStackP& operator=(TStackP& stkX);
    virtual void Clear();
    void Destroy()
    {
        Clear();
        delete (TStackElemtP*)pHead;
    };
    void PopHead()
    {
        delete (TStackElemtP*)pHead;
    };
    int Num() const
    {
        return iNum;
    };
    void Push(void* pDataI, double fIndexI = 0);
    void* Pop();
    void* Pop(int iLocation);
    void* PopFirst();
    void* operator[](int iID);
    int Delete();
    int Delete(int iPosition);
    void Set(void* pDataI)
    {
        pCurrent->pData = pDataI;
    };
    void Set(int iLoc, void* pDataI)
    {
        (*this)[iLoc];
        Set(pDataI);
    };
    void Insert(int iLocation, void* pDataI, double fIndexI = 0);
    void Insert(void* pDataI, double fIndexI = 0);
    int Member(void* pDataI);
    int Locate(void* pDataI);
    int SetIndex(double fIndexI);
    int SetIndex(int iLocation, double fIndexI);
    double Index(int iLocation = 0);
    int Swap(int iL1, int iL2);
    int Move(int iL1, int iL2);
    void Sort()
    {
        QuickSort(1, iNum);
    };
    void Head()
    {
        pCurrent = pHead;
        iCurrent = 0;
        if (iNum > 0) Next();
    };
    void Tail()
    {
        pCurrent = pTail;
        iCurrent = iNum;
    };
    void* Next();
    void* Previous();
    void* Current()
    {
        if (pCurrent) return pCurrent->pData;
        return 0;
    };
    int Pos()
    {
        return iCurrent;
    };
    void InsertSort(void* pDataI, double fIndexI = 0);
};
