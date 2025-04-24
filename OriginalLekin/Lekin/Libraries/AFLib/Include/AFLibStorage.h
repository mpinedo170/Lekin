#pragma once

#include "AFLibFileTemp.h"

/////////////////////////////////////////////////////////////////////////////
// TStorage
// Disk data storage.
// Simple class to store rarely used data (saving RAM).
// Thread-safe.

class CCriticalSection;

namespace AFLibIO {
class AFLIB TStorage
{
private:
    DEFINE_COPY_AND_ASSIGN(TStorage);

    TFileTemp m_file;           // storage file
    CCriticalSection* m_pSync;  // synchronization object

public:
    explicit TStorage(LPCTSTR name);
    virtual ~TStorage();

    // remove all data past certain position
    void Clear(int pos = 0);

    // in Write(), pos is merely a suggestion;
    // if data does not fit, will write in the end of storage space

    // write a chunk of data
    INT64 Write(LPCVOID pData, int size, INT64 pos = -1);

    // write a zero-terminated string
    INT64 Write(LPCTSTR s, INT64 pos = -1);

    // assuming a block of data starts at pos, what is its length?
    int GetBlockSize(INT64 pos);

    // allocate memory and read data
    LPVOID Read(INT64 pos);

    // read data into given space
    void Read(LPVOID pData, INT64 pos);

    // read a string
    void Read(CString& s, INT64 pos);
};
}  // namespace AFLibIO
