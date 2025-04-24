#pragma once

#include "AFLibArrays.h"
#include "AFLibDefine.h"
#include "AFLibMaps.h"
#include "AFLibStringBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// TLexAnalyzer class
// Lexical analyzer for TCompiler

namespace AFLibCalc {
class TLexParser;

// standard lexems
enum
{
    lexEOF,
    lexComment,
    lexError,
    lexID,
    lexInt,
    lexDouble,
    lexString,
    lexUser = 256
};

class AFLIB TLexAnalyzer
{
private:
    DEFINE_COPY_AND_ASSIGN(TLexAnalyzer);

    bool m_bIgnoreKeywordCase;    // ignore lettercase in keywords?
    CStringArray m_arrSLex;       // array of lexems (sorted by length (desc), then alphabetically)
    AFLib::CIntArray m_arrIDLex;  // corresponding lexem IDs
    AFLib::CIntMap<CString, LPCTSTR> m_mapKeyword;  // map of keywords

    // helper for Next() and SlackyNext()
    void PrivateNext(bool bStopAtComment);

protected:
    TLexParser* m_pSp;             // string parser
    AFLibIO::TStringBuffer m_str;  // current string or identifier

    // add a lexem
    void AddLexem(LPCTSTR sLex, int IDLex, bool bThrowOnDup = true);

    // remove a lexem
    void DelLexem(LPCTSTR sLex, bool bThrowOnNotFound = true);

    // remove all lexems
    void ClearLexems();

    // set whether to ignore case in keywords
    void SetKeywordIC(bool bIgnoreKeywordCase);

    // check if lettercase in keywords is ignored
    bool GetKeywordIC() const
    {
        return m_bIgnoreKeywordCase;
    }

    // add keyword
    void AddKeyword(LPCTSTR sKey, int IDLex, bool bThrowOnDup = true);

    // remove keyword
    void DelKeyword(LPCTSTR sKey, bool bThrowOnNotFound = true);

    // remove all keywords
    void ClearKeywords();

    // clear the public fields that are filled by Next()
    void ClearCurrent();

    // overridable: create the string parser
    virtual void CreateParser(LPCTSTR buffer);

    // skip comments in the input string
    virtual bool SkipComments();

    // convert an identifier to a keyword
    virtual void ConvertKeyword();

    // read a lexem from m_arrSLex
    virtual bool ReadStandardLexem();

    // read a number, quoted string, etc.
    virtual bool ReadOtherLexem();

public:
    // These fields are filled by Next()

    int m_pos;    // position of the current lexem
    int m_iLine;  // line # of the current lexem
    int m_iCol;   // column # of the current lexem

    int m_IDLex;  // ID of the current lexem
    int m_i;      // integer number
    double m_d;   // floating-point number

    TLexAnalyzer();
    virtual ~TLexAnalyzer();

    // start analyzing
    void Start(LPCTSTR buffer, int pos);

    // test for a lexem, throw if it ain't there
    void Test(int IDLex);

    // read next lexem; may be a comment; do not throw
    void SlackyNext();

    // read next lexem
    void Next()
    {
        PrivateNext(false);
    }

    // current string or identifier
    LPCTSTR GetStr() const
    {
        return m_str;
    }

    // is last lexem a number?
    bool IsNumber()
    {
        return m_IDLex == lexInt || m_IDLex == lexDouble;
    }

    // convert IDLex to text (may not be efficient)
    virtual CString GetLexemText(int IDLex);
};
}  // namespace AFLibCalc
