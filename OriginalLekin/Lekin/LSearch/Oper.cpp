#include "StdAfx.h"

#include "Def.h"

TOper::TOper()
{
    m_jobID = m_nodeID = m_machineID = m_processingTime = -1;
    m_bLast = true;
}

TOper::TOper(int jobID, int nodeID) : m_jobID(jobID), m_nodeID(nodeID)
{
    const TJob* pJob = JOBS[m_jobID];

    m_machineID = pJob->m_plan[m_nodeID].m_machineID;
    m_processingTime = pJob->m_plan[m_nodeID].m_processingTime;
    m_bLast = m_nodeID >= pJob->GetOperCount() - 1;
}
