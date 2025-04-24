#include "StdAfx.h"

#include "AFLibRandom.h"

using AFLibMath::TRandom;

TRandom* TRandom::m_pRndObj = NULL;
TRandom::TInit TRandom::m_init;

TRandom::TInit::~TInit()
{
    delete m_pRndObj;
    m_pRndObj = NULL;
}

TRandom::TRandom()
{
    m_iseed = int(time(NULL));
    m_iseed = (m_iseed << 16) ^ (m_iseed << 8) ^ m_iseed;
}

TRandom::TRandom(int iseed)
{
    m_iseed = iseed;
}

TRandom& TRandom::Get()
{
    if (m_pRndObj == NULL) m_pRndObj = new TRandom;
    return *m_pRndObj;
}

void TRandom::SetSeed(int iseed)
{
    m_iseed = iseed;
}

double TRandom::Uniform()
{
    if (m_iseed == 0) m_iseed = 0x1ABFABF1;
    int k1 = m_iseed / 127773;
    m_iseed = (m_iseed - k1 * 127773) * 16807 - k1 * 2836;
    if (m_iseed < 0) m_iseed += 2147483647;
    return m_iseed * 4.656612875e-10;
}

int TRandom::Uniform(int x)
{
    return int(Uniform() * x);
}

double TRandom::Uniform(double from, double to)
{
    return Uniform() * (to - from) + from;
}
