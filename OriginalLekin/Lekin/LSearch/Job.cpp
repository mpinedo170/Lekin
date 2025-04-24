#include "StdAfx.h"

#include "Def.h"

TNetWorkingPlanNode::TNetWorkingPlanNode() : m_machineID(0), m_processingTime(0)
{}

TJob::TJob()
{
    m_release = m_due = m_weight = 0;
}
