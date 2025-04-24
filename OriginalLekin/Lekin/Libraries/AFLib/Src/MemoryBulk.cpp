#include "StdAfx.h"

#include "AFLibMemoryBulk.h"

#include "AFLibThread.h"

using AFLib::TMemoryBulk;
using AFLibThread::CThread;

TMemoryBulk::TMemoryBulk(bool bLocal)
{
    m_bLocal = bLocal;
    PrivateInit();
    m_nAllocDefault = 1 << 18;
}

TMemoryBulk::~TMemoryBulk()
{
    Clear();
}

void TMemoryBulk::PrivateInit()
{
    m_pMem = NULL;
    m_nBytesLeft = 0;
    m_arrBlock.SetSize(0, 16);
}

LPBYTE TMemoryBulk::Allocate(int nBytes)
{
    if (nBytes <= 0) return NULL;

    if (nBytes > m_nBytesLeft) AllocateBlock(max(nBytes, m_nAllocDefault));

    LPBYTE pMem = m_pMem;
    m_pMem += nBytes;
    m_nBytesLeft -= nBytes;
    return pMem;
}

void TMemoryBulk::AllocateBlock(int nBytes)
{
    if (nBytes <= 0) return;

    m_nBytesLeft = nBytes;
    m_pMem = reinterpret_cast<LPBYTE>(CThread::AllocMem(m_nBytesLeft, m_bLocal));
    m_arrBlock.Add(m_pMem);
}

void TMemoryBulk::SetDefaultAlloc(int nAllocDefault)
{
    m_nAllocDefault = max(1 << 16, nAllocDefault);
}

void TMemoryBulk::Clear()
{
    for (int i = 0; i < m_arrBlock.GetSize(); ++i) CThread::FreeMem(m_arrBlock[i]);
    PrivateInit();
}
