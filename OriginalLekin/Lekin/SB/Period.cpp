#include "StdAfx.h"

#include "Period.h"

TPeriod::TPeriod(double _fStart, double _fEnd, LPCSTR _szComment)
{
    fStart = min(_fStart, _fEnd);
    fStart = max(_fStart, _fEnd);

    if (_szComment)
        memcpy(szComment, _szComment, CommentSize);
    else
        memset(szComment, 0, CommentSize);
}

TPeriod::TPeriod(TPeriod& prdX)
{
    fStart = prdX.fStart;
    fEnd = prdX.fEnd;
    memcpy(szComment, prdX.szComment, CommentSize);
}

int TPeriod::Merge(TPeriod& prdX)
{
    if (!IsIntersect(prdX)) return 0;
    fStart = min(fStart, prdX.fStart);
    fEnd = max(fEnd, prdX.fEnd);
    return 1;
}

int TPeriod::Cut(TPeriod& prdX)
{
    if (!IsIntersect(prdX)) return 0;
    if (IsInclude(prdX)) return 1;  // this period covers prdX
    if (fStart <= prdX.fStart)
        fEnd = prdX.fStart;
    else
        fStart = prdX.fEnd;
    return 1;
}

//************************************

TPeriodList& TPeriodList::operator=(TPeriodList& pdlX)
{
    Clear();
    int iN = pdlX.Num();
    for (int i = 1; i <= iN; i++) Append(pdlX[i]);
    return *this;
}

TPeriodList::TPeriodList(TPeriodList& pdlX)
{
    int iN = pdlX.Num();
    for (int i = 1; i <= iN; i++) Append(pdlX[i]);
}

void TPeriodList::Clear()
{
    TPeriod* pprdTemp;
    int iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pprdTemp = (TPeriod*)Pop();
        delete pprdTemp;
    }
}

void TPeriodList::Append(TPeriod& prdX)
{
    // find the location to insert
    int i;
    int iLoc = 0;
    TPeriod* pInList;
    TPeriod* pprd = new TPeriod(prdX);
    Head();
    for (i = 1; i <= Num(); i++)
    {
        pInList = (TPeriod*)Current();
        if (pInList->fStart > prdX.fStart) break;
        Next();
    }
    if (i > Num())  // append end
    {
        Push(pprd);
    }
    else
    {
        Previous();
        Insert(pprd);
    }
}

void TPeriodList::Append(TPeriodList& prdl)
{
    TPeriod* pPeriod;
    prdl.Head();
    for (int i = 1; i <= prdl.Num(); i++)
    {
        pPeriod = (TPeriod*)Next();
        Append(*pPeriod);
    }
}

int TPeriodList::Remove(int iLocation)
{
    if (iLocation < 1 || iLocation > Num()) return 0;
    TPeriod* pprdTemp = (TPeriod*)Pop(iLocation);
    delete pprdTemp;
    return 1;
}

int TPeriodList::Remove(TPeriod* pprdX)
{
    int iLocation = Locate(pprdX);
    return Remove(iLocation);
}

TPeriod& TPeriodList::FirstEvent(double fFrom, double fTo)
{
    double fTemp;
    if (fFrom > fTo)
    {
        fTemp = fFrom;
        fFrom = fTo;
        fTo = fTemp;
    }
    TPeriod *pprdBest = new TPeriod, *pprdTemp;
    double fBest = fTo;
    int iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pprdTemp = Get(i);
        if ((pprdTemp->fStart < fFrom) && (pprdTemp->fEnd > fFrom))
        {
            *pprdBest = *pprdTemp;
            pprdBest->fStart = fFrom;
            return *pprdBest;
        }
        if ((pprdTemp->fStart >= fFrom) && (pprdTemp->fStart <= fBest))
        {
            fBest = pprdTemp->fStart;
            *pprdBest = *pprdTemp;
        }
    }
    return *pprdBest;
}

TPeriodList& TPeriodList::Extract(double fFrom, double fTo)
{
    double fTemp;
    TPeriod* pprdTemp;
    if (fFrom > fTo)
    {
        fTemp = fFrom;
        fFrom = fTo;
        fTo = fTemp;
    }
    TPeriodList* pplNew = new TPeriodList;
    int iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pprdTemp = Get(i);
        if (((pprdTemp->fStart >= fFrom) && (pprdTemp->fStart <= fTo)) ||
            ((pprdTemp->fEnd >= fFrom) && (pprdTemp->fEnd <= fTo)))
            pplNew->Append(*pprdTemp);
    }
    return *pplNew;
}

void TPeriodList::AddSpace(double fFrom, double fTo)
{
    double fTemp;
    if (fFrom > fTo)
    {
        fTemp = fFrom;
        fFrom = fTo;
        fTo = fTemp;
    }
    TPeriod* pprdTemp;
    double fLength = fTo - fFrom;
    int iN = Num();
    for (int i = 1; i <= iN; i++)
    {
        pprdTemp = Get(i);
        if (pprdTemp->fStart >= fFrom)
        {
            pprdTemp->fStart += fLength;
            pprdTemp->fEnd += fLength;
        }
        if ((pprdTemp->fStart < fFrom) && (pprdTemp->fEnd > fFrom))
            pprdTemp->fEnd += (pprdTemp->fEnd - fFrom);
    }
}

void TPeriodList::IndexByDate()
{
    int i, iN = Num();
    for (i = 1; i <= iN; i++)
    {
        SetIndex(i, (Get(i)->fStart));
    }
}

TPeriod* TPeriodList::FindPeriod(double fTime)
{
    int i;
    TPeriod* prd;
    Head();
    for (i = 1; i <= Num(); i++)
    {
        prd = (TPeriod*)Next();
        if (prd->IsInclude(fTime)) return prd;
    }
    return 0;
}

TPeriod* TPeriodList::FindPeriod(TPeriod& pdInclude)
{
    int i;
    TPeriod* prd;
    Head();
    for (i = 1; i <= Num(); i++)
    {
        prd = (TPeriod*)Next();
        if (prd->IsIntersect(pdInclude)) return prd;
    }
    return 0;
}

int TPeriodList::Normalize()
{  // merge intersect periods
    int i = 1, iReturn = 0;
    TPeriod *pPeriodPrior, *pPeriodPost;
    while (i < Num())
    {
        pPeriodPrior = (TPeriod*)Get(i);
        pPeriodPost = (TPeriod*)Get(i + 1);
        if (pPeriodPrior->fEnd > pPeriodPost->fStart)
        {  // merge
            if (pPeriodPost->fEnd > pPeriodPrior->fEnd) pPeriodPrior->fEnd = pPeriodPost->fEnd;
            Pop(i + 1);
            iReturn = 1;
        }
        else
            i++;
    }
    return iReturn;
}

//********************************************************************

void TPeriodListDerv::Init(double fTime)
{
    Clear();
    fStart = fTime;
}

void TPeriodListDerv::Append(TPeriod& prdX)
{                            // no sorting
    if (prdX.fEnd < fStart)  // reject the period that older than starting time
        return;
    TPeriod *pExisting = FindPeriod(prdX), prdCombine = prdX;

    while (pExisting)
    {
        prdCombine.Merge(*pExisting);
        Remove(pExisting);
        pExisting = FindPeriod(prdX);
    }
    TPeriod* pprd = new TPeriod(prdCombine);
    Push(pprd);
}

void TPeriodListDerv::Append(TPeriodList& pdlX)
{
    TPeriod* pPeriod;
    pdlX.Head();
    for (int i = 1; i <= pdlX.Num(); i++)
    {
        pPeriod = (TPeriod*)pdlX.Next();
        Append(*pPeriod);
    }
}

double TPeriodListDerv::TimeTo(double fTime)
{
    double fTotal = 0, fB, fE;
    TPeriod* pPeriod;
    Head();
    for (int i = 1; i <= Num(); i++)
    {
        pPeriod = (TPeriod*)Next();
        fB = max(fStart, pPeriod->fStart);
        fE = min(pPeriod->fEnd, fTime);
        if (fE > fB)  // screen the period before fStart & after fTime out
            fTotal += fE - fB;
    }
    return fTotal;
}

double TPeriodListDerv::Convert(double fTime)
{
    double fNeeded = fTime, fTemp = fTime;
    TPeriod* pPeriod;
    while ((fTemp - TimeTo(fTemp)) < fNeeded)
    {
        pPeriod = FindPeriod(fTemp);
        if (pPeriod) fTemp = pPeriod->fEnd;
        fTemp = TimeTo(fTemp) + fNeeded;
    }
    return fTemp;
}

//********************************************************************
TPeriod& TWeekPeriod::FirstEvent(double fFrom, double fTo)
{
    TPeriod *pprdRtn = new TPeriod, *pprdTemp, *pprdBest = new TPeriod;
    if (Num() < 1) return *pprdRtn;
    double fTemp;
    if (fFrom > fTo)
    {
        fTemp = fFrom;
        fFrom = fTo;
        fTo = fTemp;
    }
    TDate dtTemp(fFrom);
    double fLength = fTo - fFrom, fDay = dtTemp.Day(), fNewFrom = fDay + (fFrom - (int)fFrom),
           fNewTo = fNewFrom + fLength, fMove = fFrom - fNewFrom, fBest = 99999;
    int iN = Num();
    fBest = fNewTo;
    for (int i = 1; i <= iN; i++)
    {
        pprdTemp = Get(i);
        if ((pprdTemp->fStart < fNewFrom) && (pprdTemp->fEnd > fNewFrom))
        {
            *pprdBest = *pprdTemp;
            pprdBest->fStart = fNewFrom;
            fBest = pprdTemp->fStart;
            i = iN;
        }
        if ((pprdTemp->fStart >= fNewFrom) && (pprdTemp->fStart <= fBest))
        {
            fBest = pprdTemp->fStart;
            *pprdBest = *pprdTemp;
        }
    }
    if (pprdTemp == 0)
    {
        /*** not found, start from the beginning ***/
        fNewFrom = 0;
        fNewTo -= 7;
        fMove += 7;
        for (int i = 1; i <= iN; i++)
        {
            pprdTemp = Get(i);
            if ((pprdTemp->fStart >= fNewFrom) && (pprdTemp->fStart <= fBest))
            {
                fBest = pprdTemp->fStart;
                *pprdBest = *pprdTemp;
            }
        }
    }
    *pprdBest += fMove;
    return *pprdBest;
}

TPeriodList& TWeekPeriod::Gen7daysList(double fTime)
{
    TDate dCurrent(fTime);
    int iDay = dCurrent.Day();
    double fEnding = fTime + 7;
    TPeriod *pPeriod1, *pPeriod2, prdTemp1, prdTemp2, prdNonWorking;
    TPeriodList* pPrdList = new TPeriodList;
    if (Num() == 0) return *pPrdList;
    for (int i = 0; i <= Num(); i++)
    {
        if (i == 0)
        {
            Tail();
            pPeriod1 = (TPeriod*)Current();
            prdTemp1.fStart = pPeriod1->fStart - 7;
            prdTemp1.fEnd = pPeriod1->fEnd - 7;
            Head();
        }
        else
        {
            pPeriod1 = (TPeriod*)Next();
            prdTemp1 = *pPeriod1;
        }
        if (i < Num())
        {
            pPeriod2 = (TPeriod*)Current();
            prdTemp2 = *pPeriod2;
        }
        else
        {
            Head();
            pPeriod2 = (TPeriod*)Next();
            prdTemp2.fStart = pPeriod2->fStart + 7;
            prdTemp2.fEnd = pPeriod2->fEnd + 7;
        }

        prdNonWorking.fStart = prdTemp1.fEnd;
        prdNonWorking.fEnd = prdTemp2.fStart;
        if (prdNonWorking.fEnd < iDay)
        {
            prdNonWorking.fStart += 7;
            prdNonWorking.fEnd += 7;
        }
        prdNonWorking.fStart += (fTime - iDay);
        prdNonWorking.fEnd += (fTime - iDay);
        pPrdList->Append(prdNonWorking);
    }
    return *pPrdList;
}

//****************************************************************
void TAvailability::Init(double fTime)
{
    NonAvail.Init(fTime);
    fLastExpand = fTime;
    NonAvail.Append(Holiday);
}

void TAvailability::ExpandWeek()
{
    TPeriodList* pprdl = &Week.Gen7daysList(fLastExpand);
    NonAvail.Append(*pprdl);
    fLastExpand += 7;
    delete pprdl;
}

double TAvailability::Convert(double fTime)
{
    if (fLastExpand <= fTime)  // need expand weekly non-avail
        ExpandWeek();
    double fOut = NonAvail.Convert(fTime);
    while (fOut >= fLastExpand)
    {
        ExpandWeek();
        fOut = NonAvail.Convert(fTime);
    }
    return fOut;
}

//*** overload iostream ***//
ostream& operator<<(ostream& os, TPeriodList& plX)
{
    os.precision(9);
    TPeriod* pprdTemp;
    int i, iN = plX.Num();
    for (i = 1; i <= iN; i++)
    {
        pprdTemp = plX.Get(i);
        os << "  from " << pprdTemp->fStart << "\t: " << pprdTemp->fEnd;
        os << "\t" << pprdTemp->szComment << endl;
    }
    os << ";" << endl;
    return os;
}

ostream& operator<<(ostream& os, TWeekPeriod& plX)
{
    os << ((TPeriodList)plX);
    return os;
}

ostream& operator<<(ostream& os, TAvailability& avX)
{
    TPeriod* pprdTemp;
    os.precision(10);
    os << "** Weekly **" << endl;
    int i, iN = avX.Week.Num();
    for (i = 1; i <= iN; i++) os << "  from " << avX.Week[i].fStart << "\t: " << avX.Week[i].fEnd << endl;
    os << ";" << endl;
    os << "** Holidays **" << endl;
    iN = avX.Holiday.Num();
    for (i = 1; i <= iN; i++)
    {
        pprdTemp = avX.Holiday.Get(i);
        os << "  from " << pprdTemp->fStart << "\t: " << pprdTemp->fEnd;
        os << "\t" << pprdTemp->szComment << endl;
    }
    os << ";" << endl;
    return os;
}

//***** istream overload *****//

istream& operator>>(istream& is, TAvailability& avX)
{
    char szTemp[200], cTemp;
    TPeriod prdTemp;
    is.getline(szTemp, 200);  // "Weekly"
    cTemp = is.get();
    while (cTemp == ' ')
    {
        is.ignore(6);
        is.getline(szTemp, 20, '\t');
        prdTemp.fStart = (double)atof(szTemp);
        is.ignore(2);
        is.getline(szTemp, 20);
        prdTemp.fEnd = (double)atof(szTemp);
        avX.Week.Append(prdTemp);
        cTemp = is.get();
    }
    is.ignore(1);
    is.getline(szTemp, 20);
    cTemp = is.get();
    while (cTemp == ' ')
    {
        is.ignore(6);
        is.getline(szTemp, 20, '\t');
        prdTemp.fStart = (double)atof(szTemp);
        is.ignore(2);
        is.getline(szTemp, 20, '\t');
        prdTemp.fEnd = (double)atof(szTemp);
        is.getline(szTemp, 50);
        _tcscpy(prdTemp.szComment, szTemp);
        avX.Holiday.Append(prdTemp);
        cTemp = is.get();
    }
    is.ignore(1);
    return is;
}
