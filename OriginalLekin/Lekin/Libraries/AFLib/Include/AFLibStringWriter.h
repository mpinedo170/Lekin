#pragma once

#include "AFLibConcatHandler.h"
#include "AFLibPointers.h"
#include "AFLibStringBuffer.h"
#include "AFLibStrings.h"

/////////////////////////////////////////////////////////////////////////////
// TStringWriter
// Writes into a string, inserting CR-LF where appropriate.
// Handles indentations.

namespace AFLibIO {
class AFLIB TStringWriter : public AFLibPrivate::TConcatHandler
{
private:
    DEFINE_COPY_AND_ASSIGN(TStringWriter);

    // delimiter to insert between words (usually space)
    CString m_delimiter;

    // character to use for indentation (usually space)
    TCHAR m_indenter;

    // desired line length
    int m_lineLength;

    // indentation after deliberate line break (NewLine())
    int m_indentReg;

    // indentation for breaking long lines
    int m_indentRoll;

    // character buffer
    TStringBuffer m_buffer;

    // delimiter to insert before the next word
    TStringBuffer m_nextDelim;

    // last word
    TStringBuffer m_word;

    // current line length
    int m_currentLength;

    // are we in the beginning of new line?
    bool m_bNewLine;

    // set m_nextDelim to either m_delimiter or a row of indent chars
    void ResetNextDelim();

    // add CR-LF to buffer
    void AddEol();

    // add next delimiter to buffer
    void AddNextDelim();

    // check if string of given length fits current line
    bool Fits(int length);

    // add m_word to buffer, empty it
    void ResetWord();

    // convert one character to a C-like sequence; move pointer
    // character may be Ascii (size=1) or Unicode (size=2)
    void ConvertToC(LPCBYTE& s, int size, TStringBuffer& letter);

    // write string in C-like format to buffer
    // string may be Ascii (size=1) or Unicode (size=2)
    void PrivateWriteCStr(LPCBYTE s, int size);

protected:
    // roll over to the next line; indent m_indentRoll
    void Rollover();

    // for WriteSafeStr to decide: save as is or as C-string
    virtual bool IsStrSafe(LPCTSTR s);

public:
    explicit TStringWriter(int lineLength = 72, LPCTSTR delimiter = AFLib::strSpace);

    // set delimiter string
    void SetDelimiter(LPCTSTR delimiter);

    // set indenting character
    void SetIndenter(TCHAR indenter);

    // set indent sizes
    void SetIndent(int indentReg, int indentRoll = -1);

    // write string, rolling over at space characters
    void WriteSplit(LPCTSTR s);

    // write string either as is or in C-like format
    void WriteSafeStr(LPCTSTR s);

    // write safe string, do not put a delimiter before it
    void WriteSafeStrNoDelim(LPCTSTR s);

    // start new line
    void NewLine();

    // what has been written so far
    const TStringBuffer& GetBuffer();

    // set grow-by parameter for the buffer
    void SetGrowBy(int growBy)
    {
        m_buffer.SetGrowBy(growBy);
    }

    // write string, do not put a delimiter before it
    void WriteNoDelim(LPCTSTR s)
    {
        m_word += s;
    }

    // write Ascii string in C-like format, no delimiter
    void WriteCStrNoDelim(LPCSTR s)
    {
        PrivateWriteCStr(LPCBYTE(s), sizeof(CHAR));
    }

    // write Unicode string in C-like format, no delimiter
    void WriteCStrNoDelim(LPCWSTR s)
    {
        PrivateWriteCStr(LPCBYTE(s), sizeof(WCHAR));
    }

    // write string
    void Write(LPCTSTR s)
    {
        ResetWord();
        m_word.Empty();
        m_word += s;
    }

    // write Ascii string in C-like format
    void WriteCStr(LPCSTR s)
    {
        ResetWord();
        WriteCStrNoDelim(s);
    }

    // write Unicode string in C-like format
    void WriteCStr(LPCWSTR s)
    {
        ResetWord();
        WriteCStrNoDelim(s);
    }
};
}  // namespace AFLibIO
