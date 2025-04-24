#pragma once

#include "AFLibStringParserAbstract.h"

/////////////////////////////////////////////////////////////////////////////
// TStringParserGeneral
// Parent for TStringSpacer and TStringParser

namespace AFLibIO {
class AFLIB TStringParserGeneral : public TStringParserAbstract
{
private:
    typedef TStringParserAbstract super;
    DEFINE_COPY_AND_ASSIGN(TStringParserGeneral);

    bool m_bEaterOn;  // space eater flag

protected:
    TStringParserGeneral(LPCTSTR sPtr, bool bEaterOn);

    // eat space: skip till the next non-space character
    void ES();

    virtual void PostRead();

public:
    // turn the space eater on/off
    virtual void SetEater(bool bOn);

    // get the value of the space eater flag
    bool GetEater()
    {
        return m_bEaterOn;
    }

    // set current pointer position
    virtual void SetPos(int pos);

    // skip symbols
    virtual void Skip(int n);

    // skip one symbol
    virtual void Next();

    // skip whitespace
    virtual void EatSpace();

    // read up to the first space character
    CString ReadToSpace();

    // read double number, ignore commas
    double NoCommasDouble();

    // test if sPtr matches str, move pointer if yes
    bool TestSymbol(LPCTSTR str);

    // test if sPtr matches str, do not move the pointer
    bool TestSymbolNoMove(LPCTSTR str);

    // test symbol, move pointer, throw an exception if not matched
    void TestSymbolHard(LPCTSTR str);

    // test array of keywords
    int TestSymbolArray(const LPCTSTR arrStr[]);
    int TestSymbolArray(const CStringArray& arrStr);

    // test array of keywords, throw an exception if none matches
    int TestSymbolArrayHard(const LPCTSTR arrStr[]);
    int TestSymbolArrayHard(const CStringArray& arrStr);

    // find the substring and skip it
    CString FindFirst(LPCTSTR strWhat);

    // read the exact number of characters
    CString ReadExact(int count);

    // class for manipulating the space eater flag:
    // constructor sets it to new value,
    // destructor restores the old value

    class AFLIB TEaterManipulator
    {
    private:
        // object to which the manipulator is being applied
        TStringParserGeneral& m_host;

        // old value of the space eater flag
        bool m_bEaterOnSaved;

    public:
        TEaterManipulator(TStringParserGeneral& host, bool bEaterOn);
        ~TEaterManipulator();
    };
};
}  // namespace AFLibIO
