#include "StdAfx.h"

#include "AFLibBlockMBCounter.h"

using AFLibGui::TBlockMBCounter;

int TBlockMBCounter::m_nBlockMB = 0;

TBlockMBCounter::TBlockMBCounter()
{
    ++m_nBlockMB;
}

TBlockMBCounter::~TBlockMBCounter()
{
    ASSERT(m_nBlockMB > 0);
    --m_nBlockMB;
}

bool TBlockMBCounter::IsMsgBoxBlocked()
{
    return m_nBlockMB > 0;
}
