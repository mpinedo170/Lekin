#include "StdAfx.h"

#include "Def.h"
#include "Start.h"

TQueueElement::TQueueElement() : m_release(0)
{}

TQueueElement::TQueueElement(const TOper& oper, int release) : m_oper(oper), m_release(release)
{}

TMachineInfo::TMachineInfo() : m_heapIndex(-1)
{
    int index = MC_ARRAY.GetSize();
    m_timeToChange = MACHINES[index]->m_release;
    m_bBusy = m_timeToChange > 0;

    MC_ARRAY.Add(this);
    MC_HEAP.Add(this);
}

void TMachineInfo::Adjust()
{
    MC_HEAP.Adjust(m_heapIndex);
}

void TMachineInfo::AddOper(int systemTime, const TOper& oper, int release)
{
    int timeToChange = 0;

    if (release <= systemTime)
    {
        m_ready.Add(new TOper(oper));
        timeToChange = systemTime;
    }
    else
    {
        m_pending.Add(new TQueueElement(oper, release));
        timeToChange = release;
    }

    if (!m_bBusy && timeToChange < m_timeToChange)
    {
        m_timeToChange = timeToChange;
        Adjust();
    }
}

TOper TMachineInfo::Change()
{
    while (!m_pending.IsEmpty() && m_pending[0]->m_release <= m_timeToChange)
    {
        TQueueElement* pElem = m_pending.Pop();
        m_ready.Add(new TOper(pElem->m_oper));
        delete pElem;
    }

    if (m_ready.IsEmpty())
    {
        m_bBusy = false;
        m_timeToChange = m_pending.IsEmpty() ? INT_MAX : m_pending[0]->m_release;
        Adjust();
        return TOper();
    }

    m_bBusy = true;
    TOper oper = *m_ready[0];
    m_ready.DestroyPop();
    m_timeToChange += oper.m_processingTime;
    Adjust();
    return oper;
}
