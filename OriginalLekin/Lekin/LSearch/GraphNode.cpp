#include "StdAfx.h"

#include "Def.h"

TGraphNode::TGraphNode(const TOper& oper, int position) : TOper(oper), m_position(position)
{
    m_longestPath = m_savedLP = m_inputCounter = 0;
    m_bMarked = false;
    m_pNodePred = m_pNodeSucc = m_pNodePredLP = NULL;
}

void TGraphNode::Swap(int delta)
{  // Vertausche CANDIDATE mit Vorg„ngerknoten
    TMachine* pMachine = MACHINES[m_machineID];
    TGraphNode* pNode = pMachine->m_graph[m_position + delta];
    pMachine->m_graph[m_position + delta] = this;
    pMachine->m_graph[m_position] = pNode;

    // Position in der Auftragsfolge korrigieren
    pNode->m_position = m_position;
    m_position += delta;
}
