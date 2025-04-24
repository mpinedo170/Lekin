// Period.h

#pragma once

#include "Date.h"
#include "Parameter.h"
#include "Stack.h"

/* TPeriod class

   This is a base class for TPeriodList.

   Commands
   ========
   TPeriod(start,end,comment)
 - +=(fx)         : add the starting & ending time with fx.
 f Length()       : return the length of the period
 - UpdateEnd(prd) : add the ending time by the length of period "prd".
 b IsIntersect(prd): return 1 if prd is intersect with this period.
 b IsInclude(prd) : return 1 if prd is inside this period.
 b IsInclude(fT)  : return 1 if the period cover time fT
 b Merge(prd)     : merge this period with prd.
 b Cut(prd)       : remove the intersection period with prd out (except that
                    prd is inside this period).
*/
class TPeriod
{
private:
    enum
    {
        CommentSize = 40
    };

public:
    double fStart, fEnd;
    TCHAR szComment[CommentSize];

    TPeriod(double _fStart = 0, double _fEnd = 0, LPCTSTR _szComment = NULL);
    TPeriod(TPeriod& prdX);
    void operator+=(double fX)
    {
        fStart += fX;
        fEnd += fX;
    };
    double Length()
    {
        return fEnd - fStart;
    }
    void UpdateEnd(TPeriod& prdX)
    {
        fEnd += prdX.Length();
    }
    int IsIntersect(TPeriod& prdX)
    {
        return fEnd > prdX.fStart && fStart < prdX.fEnd;
    }
    int Merge(TPeriod& prdX);
    int IsInclude(TPeriod& prdX)
    {
        return fStart <= prdX.fStart && fEnd >= prdX.fEnd;
    }
    int IsInclude(double fT)
    {
        return fStart <= fT && fEnd >= fT;
    }
    int Cut(TPeriod& prdX);
};

/* TPeriodList

   Commands
   ========
   TPeriodList()
 x [i]              : return the period at location i.
 - Clear()
 - Append(prd)      : Add new period "prd" to the list.
 p Get(iN)
 b Remove(iN)       : Remove the "iN"th period from the list.
 b Remove(px)       : Remove period the ptr px point to.
 p Get(iN)          : Return ptr to the "iN"th period.
 x FirstEvent(fF,fT): Return the 1st event (period) from time fF to fT.
 x Extract(fF,fT)   : Return a section of the list from time fF to fT.
 - AddSpace(fF,fx)  : Push the starting & ending time of the periods.
                      after time fF for fx.
 - IndexByDate()    : Set the index by period starting time
 p FindPeriod(fT)   : return period in the list that cover time fT
 p FindPeriod(prd)  : return period in the list that intersect with prd
 i Normalize()      : Merge intersect periods; return 1 if some periods are merged
*/
class TPeriodList : public TStackP
{
public:
    TPeriodList(){};
    TPeriodList(TPeriodList& pdlX);
    TPeriodList& operator=(TPeriodList& pdlX);
    TPeriod& operator[](int iLocation)
    {
        return *(Get(iLocation));
    };
    void Clear();
    virtual void Append(TPeriod& prdX);
    virtual void Append(TPeriodList& pdlX);
    TPeriod* Get(int iLocation)
    {
        return (TPeriod*)TStackP::operator[](iLocation);
    };
    int Remove(int iLocation);
    int Remove(TPeriod* pprdX);
    virtual TPeriod& FirstEvent(double fFrom, double fTo = 99999);  // fFrom<fTo
    virtual TPeriodList& Extract(double fFrom, double fTo);
    void AddSpace(double fFrom, double fTo);
    void IndexByDate();
    TPeriod* FindPeriod(double fTime);
    TPeriod* FindPeriod(TPeriod& pdInclude);
    int Normalize();
    ~TPeriodList()
    {
        Clear();
    };
};

/*
  TPeriodListDerv

  It is used as a calculation tool in finding the starting/ending time of the
  operations when concerning about the machine NOT available time.

 - Init()           : Clear the list and set the starting time
 - Append(prd)      : Add the period to the list. If there is any intersection,
                      merge them.
 - Append(prdl)     : Append periods in prdl to the list
 f TimeTo(fTime)    : return the amount of time in periods from fStart until
                      fTime.
 f Convert(fTime)   : convert the time to include the not avail time.
*/

class TPeriodListDerv : public TPeriodList
{
private:
    double fStart;

public:
    void Init(double fTime);
    void Append(TPeriod& prdX);
    void Append(TPeriodList& pdlX);
    double TimeTo(double fTime);
    double Convert(double fTime);
};

/* TWeekPeriod class

   This class store weekly schedule that is repeated. The period stored in
   this class must start counting from Saturday night (Sunday 0am).
   The data stored is the working hours.

   COMMANDS
   ========
   TWeekPeriod();
 x FirstEvent(fF,fT)
 x Gen7daysList(fTime)  : return a list of NOT working time from fTime for 7 days
*/

class TWeekPeriod : public TPeriodList
{
public:
    TPeriod& FirstEvent(double fFrom, double fTo);  // fFrom<fTo
    TPeriodList& Gen7daysList(double fTime);
};

/* TAvailability class

   COMMANDS
   ========
 - Clear()        : clear all the data
 - Init(fT)       : set the starting time of the first operation
 f Convert(fT)    : convert the always-avail-time to actual time
*/
class TAvailability
{
private:
    double fLastExpand;        // last weekly expandsion time
    TPeriodListDerv NonAvail;  // working list
public:
    TWeekPeriod Week;
    TPeriodList Holiday;
    void Clear()
    {
        Week.Clear();
        Holiday.Clear();
        NonAvail.Clear();
    };
    void Init(double fTime);
    void ExpandWeek();
    double Convert(double fTime);
};

//*** overload iostream ***//
ostream& operator<<(ostream& os, TPeriodList& plX);
ostream& operator<<(ostream& os, TWeekPeriod& plX);
ostream& operator<<(ostream& os, TAvailability& avX);

//*** overload istream ***//
istream& operator>>(istream& is, TAvailability& avX);
