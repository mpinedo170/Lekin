#pragma once

/************************************************************************/
/*                                                                      */
/*                   Erzeugung eines ersten Ablaufplanes                */
/*                     mit Hilfe von Priorit„tsregeln.                  */
/*                                                                      */
/************************************************************************/

#include "Def.h"

struct TMachineInfo;

// Lokale Strukturen
struct TQueueElement
{
    TOper m_oper;
    int m_release;

    TQueueElement();
    TQueueElement(const TOper& oper, int release);

    int GetGLKey() const
    {
        return m_release;
    }
};

struct TMachineInfo
{
    int m_heapIndex;
    int m_timeToChange;
    bool m_bBusy;

    CIntHeap<TQueueElement> m_pending;
    CDblHeap<TOper> m_ready;

    TMachineInfo();

    int GetGLKey() const
    {
        return m_timeToChange;
    }

    void Adjust();
    void AddOper(int systemTime, const TOper& oper, int release);
    TOper Change();
};

struct TMachineHeap : public CIntHeap<TMachineInfo>
{
protected:
    virtual void SetIndex(TMachineInfo* pT, int index)
    {
        pT->m_heapIndex = index;
    }
};

extern TMachineHeap MC_HEAP;
extern CSmartArray<TMachineInfo> MC_ARRAY;

void GetFirstSchedule();
void ConstructGraph();
bool FindLongestPath();
void GetResult();
void SaveLP();
void FreeMemory();
