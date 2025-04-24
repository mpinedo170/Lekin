#include "StdAfx.h"

#include "Array.h"

//////////////////////////////// TArrayP ////////////////////////////////
TArrayP::TArrayP(TArrayP& ar)
{
    iSize = 0;
    for (int i = ar.iSize; i >= 0; i--)
    {
        Put(i, ar.Get(i));
    }
}

TArrayP& TArrayP::operator=(TArrayP& ar)
{
    Clear();
    for (int i = ar.iSize - 1; i >= 0; i--)
    {
        Put(i, ar.Get(i));
    }
    return *this;
}

void TArrayP::Clear()
{
    if (iSize > 0) delete[] ptr;
    iSize = 0;
}

void* TArrayP::Get(int iLoc)
{
    if (iLoc < iSize)
        return ptr[iLoc].data;
    else
        return 0;
}

int TArrayP::Put(int iLoc, void* pt, double indx)
{
    TElmt* ptrNew;
    if (iLoc < 0) return 0;  // error
    if (iLoc < iSize)
    {
        ptr[iLoc].data = pt;
        ptr[iLoc].index = indx;
        return 0;  // no expansion
    }
    else
    {
        int i;
        ptrNew = new TElmt[iLoc + 1];
        for (i = 0; i < iSize; i++)
        {
            ptrNew[i].data = ptr[i].data;
            ptrNew[i].index = ptr[i].index;
        }
        for (; i < iLoc; i++)
        {
            ptrNew[i].data = 0;
            ptrNew[i].index = 0;
        }
        if (iSize > 0) delete[] ptr;
        ptr = ptrNew;
        ptr[iLoc].data = pt;
        ptr[iLoc].index = indx;
        iSize = iLoc + 1;
        return 1;  // expanded
    }

    /*
    TElmt *ptrOld;
    if(iLoc<iSize)
    {
      ptr[iLoc].data  = pt;
      ptr[iLoc].index = indx;
      return 0;
    } else
    {
      ptrOld = ptr;
      ptr = new TElmt[iLoc+1];
      for(int i=0;i<iSize;i++)
      {
        ptr[i].data  = ptrOld[i].data;
        ptr[i].index = ptrOld[i].index;
      }
      for(;i<iLoc;i++)
      {
        ptr[i].data  = 0;
        ptr[i].index = 0;
      }
      if(iSize>0)
        delete [] ptrOld;
      ptr[iLoc].data  = pt;
      ptr[iLoc].index = indx;
      iSize = iLoc+1;
      return 1; // expanded
    }
    */
}

void* TArrayP::Pop()
{
    if (iSize == 1)
    {
        delete[] ptr;
        iSize = 0;
    }
    void* ptrI = (*this)[iSize - 1];
    iSize--;
    return ptrI;
}

int TArrayP::Member(void* pData)
{
    for (int i = 0; i <= iSize; i++)
    {
        if (Get(i) == pData) return 1;
    }
    return 0;
}

int TArrayP::Locate(void* pData)
{
    for (int i = 0; i <= iSize; i++)
    {
        if (Get(i) == pData) return i;
    }
    return 0;
}

void TArrayP::SetSize(int iS)
{
    if (iSize == 0)
    {
        ptr = new TElmt[iS];
        iSize = iS;
    }
    else
    {
        TElmt* ptrNew = new TElmt[iS];
        int iLoop = min(iSize, iS);
        for (int i = 0; i < iLoop; i++)
        {
            ptrNew[i].data = ptr[i].data;
            ptrNew[i].index = ptr[i].index;
        }
        delete[] ptr;
        ptr = ptrNew;
        iSize = iS;
    }
}

////////////////////////////// TArrayL ///////////////////////////////

TArrayL::TArrayL(int iSize)
{
    iNum = 0;
    if (iSize > 0) SetSize(iSize);
}

TArrayL::TArrayL(const TArrayL& ar)
{
    iNum = ar.iNum;
    data = new long[iNum];
    for (int i = 0; i < iNum; i++) data[i] = ar.data[i];
}

TArrayL& TArrayL::operator=(const TArrayL& ar)
{
    Clear();
    iNum = ar.iNum;
    data = new long[iNum];
    for (int i = 0; i < iNum; i++) data[i] = ar.data[i];
    return *this;
}

long& TArrayL::operator[](int iLoc)
{
    if (iLoc > (iNum - 1)) SetSize(iLoc + 1);
    return data[iLoc];
}

void TArrayL::SetSize(int iSize)
{
    long* dataTemp = new long[iSize];
    int iCopySize = (iNum < iSize) ? iNum : iSize;
    for (int i = 0; i < iCopySize; i++) dataTemp[i] = data[i];
    Clear();
    data = dataTemp;
    iNum = iSize;
}

void TArrayL::Clear()
{
    if (iNum > 0)
    {
        delete[] data;
        data = NULL;
        iNum = 0;
    }
}

////////////////////////////// TArrayF ///////////////////////////////

TArrayF::TArrayF(int iSize)
{
    iNum = 0;
    if (iSize > 0) SetSize(iSize);
}

TArrayF::TArrayF(const TArrayF& ar)
{
    iNum = ar.iNum;
    data = new double[iNum];
    for (int i = 0; i < iNum; i++) data[i] = ar.data[i];
}

TArrayF& TArrayF::operator=(const TArrayF& ar)
{
    Clear();
    iNum = ar.iNum;
    data = new double[iNum];
    for (int i = 0; i < iNum; i++) data[i] = ar.data[i];
    return *this;
}

double& TArrayF::operator[](int iLoc)
{
    if (iLoc > (iNum - 1)) SetSize(iLoc + 1);
    return data[iLoc];
}

void TArrayF::SetSize(int iSize)
{
    double* dataTemp = new double[iSize];
    int iCopySize = (iNum < iSize) ? iNum : iSize;
    for (int i = 0; i < iCopySize; i++) dataTemp[i] = data[i];
    Clear();
    data = dataTemp;
    iNum = iSize;
}

void TArrayF::Clear()
{
    if (iNum > 0)
    {
        delete[] data;
        data = NULL;
        iNum = 0;
    }
}

////////////////////////////////// TArrayF2 ////////////////////////////////
TArrayF2::TArrayF2(int iX, int iY)
{
    TArrayF* pArr;
    for (int i = iX - 1; i >= 0; i--)
    {
        pArr = new TArrayF(iY);
        Put(i, pArr);
    }
}

TArrayF2::TArrayF2(TArrayF2& aX)
{
    TArrayF* pArr;
    for (int i = aX.Num() - 1; i >= 0; i--)
    {
        pArr = new TArrayF(aX[i]);
        Put(i, pArr);
    }
}

TArrayF2::~TArrayF2()
{
    int i;
    TArrayF* pArr;
    for (i = 0; i < Num(); i++)
    {
        pArr = &((*this)[i]);
        delete pArr;
    }
}

TArrayF2& TArrayF2::operator=(TArrayF2& aX)
{
    Clear();
    TArrayF* pArr;
    for (int i = aX.Num() - 1; i >= 0; i--)
    {
        pArr = new TArrayF(aX[i]);
        Put(i, pArr);
    }
    return *this;
}

void TArrayF2::Clear()
{
    TArrayF* pArr;
    int i;
    for (i = 0; i < Num(); i++)
    {
        pArr = &((*this)[i]);
        pArr->Clear();
    }
    TArrayP::Clear();
    /*
    while(Num()>0)
    {
      pArr = (TArrayF*) Pop();
      pArr->Clear();
    }
    */
}

void TArrayF2::SetSize(int iX, int iY)
{
    TArrayF2 fNew(iX, iY);
    TArrayF* pArr;
    int i, j, iMinX = min(iX, Num()), iMinY;

    for (i = 0; i < iMinX; i++)
    {
        pArr = &((*this)[i]);
        iMinY = min(pArr->Num(), iY);
        for (j = 0; j < iMinY; j++)
        {
            fNew[i][j] = (*pArr)[j];
        }
    }
    *this = fNew;
    /*
    TArrayF2 f2Temp(iX,iY);
    TArrayF  *pArr;
    int i,j,
        iMinX = min(iX,Num()),
        iMinY;
    for(i=0;i<iMinX;i++)
    {
      pArr = &((*this)[i]);
      iMinY = min(pArr->Num(),iY);
      for(j=0;j<iMinY;j++)
      {
        f2Temp[i][j]=(*pArr)[j];
      }
    }
    Clear();
    (*this) = f2Temp;
    for(i=iX-1;i>=iMinX;i--)
    {
      pArr = new TArrayF(iY);
      Put(i,pArr);
    }
    */
}

TArrayF& TArrayF2::operator[](int index)
{
    if (index > (Num() - 1)) SetSize(index + 1);
    //  if(index<Num())
    return (*(TArrayF*)(Get(index)));
}

void TArrayF2::SetSize(int iX)
{
    int i;
    TElmt* ptrNew = new TElmt[iX];

    for (i = 0; i < iSize; i++)
    {
        ptrNew[i].data = ptr[i].data;
        ptrNew[i].index = ptr[i].index;
    }
    for (; i < iX; i++)
    {
        ptrNew[i].data = new TArrayF;
        ptrNew[i].index = 0;
    }
    if (iSize > 0) delete[] ptr;
    ptr = ptrNew;
    iSize = iX;

    /*
    TArrayF2 f2Temp;
    TArrayF  *pArr;
    int i,j,
        iMinX = min(iX,Num()),
        iMinY;
    for(i=0;i<iMinX;i++)
    {
      pArr = &((*this)[i]);
      iMinY = pArr->Num();
      for(j=iMinY-1;j>=0;j--)
      {
        f2Temp[i][j]=(*pArr)[j];
      }
    }
    Clear();
    (*this) = f2Temp;
    for(i=iX-1;i>=iMinX;i--)
    {
      pArr = new TArrayF;
      Put(i,pArr);
    }
    */
}

////////////////////////////////// TArrayL2 ////////////////////////////////
TArrayL2::TArrayL2(int iX, int iY)
{
    TArrayL* pArr;
    for (int i = iX - 1; i >= 0; i--)
    {
        pArr = new TArrayL(iY);
        Put(i, pArr);
    }
}

TArrayL2::TArrayL2(TArrayL2& aX)
{
    TArrayL* pArr;
    for (int i = aX.Num() - 1; i >= 0; i--)
    {
        pArr = new TArrayL(aX[i]);
        Put(i, pArr);
    }
}

TArrayL2::~TArrayL2()
{
    int i;
    TArrayL* pArr;
    for (i = 0; i < Num(); i++)
    {
        pArr = &((*this)[i]);
        delete pArr;
    }
}

TArrayL2& TArrayL2::operator=(TArrayL2& aX)
{
    Clear();
    TArrayL* pArr;
    for (int i = aX.Num() - 1; i >= 0; i--)
    {
        pArr = new TArrayL(aX[i]);
        Put(i, pArr);
    }
    return *this;
}

void TArrayL2::Clear()
{
    TArrayL* pArr;
    int i;
    for (i = 0; i < Num(); i++)
    {
        pArr = &((*this)[i]);
        delete pArr;
        // pArr->Clear();
    }
    TArrayP::Clear();
    /*
    while(Num()>0)
    {
      pArr = (TArrayL*) Pop();
      pArr->Clear();
    }
    */
}

void TArrayL2::SetSize(int iX, int iY)
{
    TArrayL2 iNew(iX, iY);
    TArrayL* pArr;
    int i, j, iMinX = min(iX, Num()), iMinY;

    for (i = 0; i < iMinX; i++)
    {
        pArr = &((*this)[i]);
        iMinY = min(pArr->Num(), iY);
        for (j = 0; j < iMinY; j++)
        {
            iNew[i][j] = (*pArr)[j];
        }
    }
    *this = iNew;
}

TArrayL& TArrayL2::operator[](int index)
{
    if (index > (Num() - 1)) SetSize(index + 1);
    //  if(index<Num())
    return (*(TArrayL*)(Get(index)));
}

void TArrayL2::SetSize(int iX)
{
    int i;
    TElmt* ptrNew = new TElmt[iX];
    for (i = 0; i < iSize; i++)
    {
        ptrNew[i].data = ptr[i].data;
        ptrNew[i].index = ptr[i].index;
    }
    for (; i < iX; i++)
    {
        ptrNew[i].data = new TArrayL;
        ptrNew[i].index = 0;
    }
    if (iSize > 0) delete[] ptr;
    ptr = ptrNew;
    iSize = iX;
}
