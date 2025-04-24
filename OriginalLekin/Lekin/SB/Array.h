#pragma once

/* TArrayP

    This class is quite similar to TStackP.
    Locating a member is fast but appending is slow.
    It can auto expand but not auto collapse. To reduce the size use Pop().

  Note: Array start from 0 but stack start from 1.
*/

struct TElmt
{
    void* data;
    double index;
};

class TArrayP
{
protected:
    TElmt* ptr;
    int iSize;

public:
    TArrayP()
    {
        iSize = 0;
    }
    TArrayP(int iN)
    {
        iSize = 0;
        SetSize(iN);
    }
    TArrayP(TArrayP& ar);
    TArrayP& operator=(TArrayP& ar);
    virtual ~TArrayP()
    {
        Clear();
    }

    virtual void Clear();
    void* Get(int iLoc);
    int Put(int iLoc, void* pt, double indx = 0);
    void* operator[](int iLoc)
    {
        return Get(iLoc);
    }
    int Num()
    {
        return iSize;
    }
    void Push(void* pData, double fIndexI = 0)
    {
        Put(iSize, pData, fIndexI);
    }
    void* Pop();
    int Member(void* pData);
    int Locate(void* pData);
    void SetSize(int iS);
};

class TArrayL
{
private:
    int iNum;
    long* data;

public:
    TArrayL(int iSize = 0);
    TArrayL(const TArrayL& ar);
    TArrayL& operator=(const TArrayL& ar);
    long& operator[](int iLoc);
    operator long*()
    {
        return data;
    }
    virtual ~TArrayL()
    {
        Clear();
    }
    virtual void Clear();
    int Num()
    {
        return iNum;
    }
    void SetSize(int iSize);
};

// TArrayF : array of double with dynamic size

class TArrayF
{
private:
    int iNum;
    double* data;

public:
    TArrayF(int iSize = 0);
    TArrayF(const TArrayF& ar);
    TArrayF& operator=(const TArrayF& ar);
    double& operator[](int iLoc);
    operator double*()
    {
        return data;
    }
    virtual ~TArrayF()
    {
        Clear();
    }
    virtual void Clear();
    int Num()
    {
        return iNum;
    }
    void SetSize(int iSize);
};

class TArrayF2 : public TArrayP
{
public:
    TArrayF2(int iX = 0, int iY = 0);
    TArrayF2(TArrayF2& aX);
    virtual ~TArrayF2();
    TArrayF2& operator=(TArrayF2& aX);
    virtual void Clear();
    void SetSize(int iX, int iY);
    void SetSize(int iX);
    TArrayF& operator[](int index);
};

class TArrayL2 : public TArrayP
{
public:
    TArrayL2(int iX = 0, int iY = 0);
    TArrayL2(TArrayL2& aX);
    virtual ~TArrayL2();
    TArrayL2& operator=(TArrayL2& aX);
    virtual void Clear();
    void SetSize(int iX, int iY);
    void SetSize(int iX);
    TArrayL& operator[](int index);
};
