#pragma once

#include "AFLibDefine.h"
#include "AFLibIOGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibWorc.h"

namespace AFLibPrivate {
class TWorcerRead;
}

/////////////////////////////////////////////////////////////////////////////
// TFileParserAbstract structure
// Splits the file into tokens.
// Abstract class; the logic to split the file is defined in descendants.

namespace AFLibIO {
class AFLIB TFileParserAbstract : protected AFLibThread::TWorc
{
private:
    typedef AFLibThread::TWorc super;
    friend AFLibPrivate::TWorcerRead;
    DEFINE_COPY_AND_ASSIGN(TFileParserAbstract);

    CFile& m_file;         // underlying file
    INT64 m_length;        // file length
    INT64 m_position;      // current position
    TTypeTextFile m_ttf;   // type of the text file being read
    int m_lenSig;          // length of the byte-order mark at the start of the file
    int m_lenChar;         // character length (1 or 2)
    TStringBuffer m_line;  // current line
    bool m_bEof;           // set by GetNext() synchronously

    TStringBuffer m_remainder;    // remainder from the last FileRead
    volatile int m_nEol;          // characters to end-of-line in m_remainder
    volatile int m_nSkip;         // characters to skip after m_nEol
    volatile bool m_bReachedEof;  // have we reached the end of file?

    // overridden from TWokc
    virtual AFLibThread::TWorcer* CreateWorcer(int index, int nWorcers);

protected:
    // start the reading thread (call in the end of the derived constructor)
    void StartReader();

    // find the delimiter in the input line
    virtual bool FindDelimiter(const TStringBuffer& line, int& position, int& skip) = 0;

    // line is the last segment of the file;
    //   FindDelimiter() did not find anything in it
    virtual int FindLastSegment(const TStringBuffer& line);

public:
    // default block size for reading the file into memory
    static const int DefBlockSize = 1 << 12;

    explicit TFileParserAbstract(CFile& file, int blockSize = DefBlockSize);

    // move file pointer
    void SetPosition(INT64 pos);

    // read next token
    const TStringBuffer& GetNext();

    // get the full path of the parsed file
    CString GetFilePath();

    // get the name of the parsed file
    CString GetFileName();

    // return the current token
    const TStringBuffer& GetCurrent()
    {
        return m_line;
    }

    // get file size
    INT64 GetLength()
    {
        return m_length;
    }

    // get current file position
    INT64 GetPosition()
    {
        return m_position;
    }

    // reached end-of-file?
    bool IsEof()
    {
        return m_bEof;
    }
};
}  // namespace AFLibIO
