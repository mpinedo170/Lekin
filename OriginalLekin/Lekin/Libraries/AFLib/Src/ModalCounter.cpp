#include "StdAfx.h"

#include "AFLibModalCounter.h"

using AFLibGui::TModalCounter;

int TModalCounter::m_nModalDlgs = 0;

TModalCounter::TModalCounter()
{
    ++m_nModalDlgs;
}

TModalCounter::~TModalCounter()
{
    ASSERT(m_nModalDlgs > 0);
    --m_nModalDlgs;
}

bool TModalCounter::IsModal()
{
    return m_nModalDlgs > 0;
}
