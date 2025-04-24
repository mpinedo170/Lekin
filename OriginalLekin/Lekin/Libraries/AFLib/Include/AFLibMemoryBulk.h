#pragma once

#include "AFLibDefine.h"

/////////////////////////////////////////////////////////////////////////////
// TMemoryBulk
// The class that allocates memory in bulk.
// No way to free individual memory blocks;
//   ALL memory is freed by Clear()

namespace AFLib {
class AFLIB TMemoryBulk
{
private:
    DEFINE_COPY_AND_ASSIGN(TMemoryBulk);

    bool m_bLocal;                      // memory allocated in the thread-local heap
    CArray<LPBYTE, LPBYTE> m_arrBlock;  // memory blocks
    LPBYTE m_pMem;                      // current block
    int m_nBytesLeft;                   // bytes left in the last block
    int m_nAllocDefault;                // default amt of mem to allocate in a new block

    // initialize some members
    void PrivateInit();

public:
    explicit TMemoryBulk(bool bLocal = false);
    ~TMemoryBulk();

    // allocate nBytes in current block; get new block if necessary
    LPBYTE Allocate(int nBytes);

    // allocate new block
    void AllocateBlock(int nBytes);

    // set the default block size
    void SetDefaultAlloc(int nAllocDefault);

    // clear all memory
    void Clear();

    // get the default block size
    int GetDefaultAlloc() const
    {
        return m_nAllocDefault;
    }
};
}  // namespace AFLib
