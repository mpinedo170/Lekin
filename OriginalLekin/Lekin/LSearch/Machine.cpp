#include "StdAfx.h"

#include "Def.h"

TMachine::TMachine() : m_release(0)
{}

TMachine::~TMachine()
{
    m_graph.DestroyAll();
}

void TMachine::ClearMark()
{
    for (int j = 0; j < GetOperCount(); ++j) m_graph[j]->m_bMarked = false;
}

void TMachine::SaveConfiguration()
{
    m_bestJobOrder.RemoveAll();
    for (int j = 0; j < GetOperCount(); ++j)
        m_bestJobOrder.Add(TOper(m_graph[j]->m_jobID, m_graph[j]->m_nodeID));
}

void TMachine::ReadBestPlan()
{
    m_activeJobOrder.Copy(m_bestJobOrder);
}
