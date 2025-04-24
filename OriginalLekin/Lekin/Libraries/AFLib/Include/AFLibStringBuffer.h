#pragma once

#include "AFLibDefine.h"
#include "AFLibStringBufferGeneric.h"

/////////////////////////////////////////////////////////////////////////////
// TStringBufferA and TStringBufferW
// Ascii and Unicode strings that only allow +=; do fewer reallocations.

#define DEFINE_COMPARE_OPERATOR(op, type)  \
    bool operator op(type str) const       \
    {                                      \
        return ProtectedCompare(str) op 0; \
    }

#define DEFINE_COMPARE_OPERATOR_A(op)            \
    DEFINE_COMPARE_OPERATOR(op, const CStringA&) \
    DEFINE_COMPARE_OPERATOR(op, LPCSTR)

#define DEFINE_COMPARE_OPERATOR_W(op)            \
    DEFINE_COMPARE_OPERATOR(op, const CStringW&) \
    DEFINE_COMPARE_OPERATOR(op, LPCWSTR)

namespace AFLibIO {
class AFLIB TStringBufferA : public TStringBufferGeneric
{
private:
    typedef TStringBufferGeneric super;

public:
    TStringBufferA() : super(0)
    {}
    TStringBufferA(const TStringBufferA& sb) : super(sb)
    {}
    explicit TStringBufferA(int nGrow) : super(0, nGrow)
    {}

    TStringBufferA& operator=(const TStringBufferA& sb)
    {
        ProtectedCopy(sb);
        return *this;
    }

    TStringBufferA& operator=(const CStringA& s)
    {
        Empty();
        Append(s);
        return *this;
    }

    TStringBufferA& operator=(LPCSTR str)
    {
        Empty();
        Append(str);
        return *this;
    }

    void operator+=(const TStringBufferA& sb)
    {
        Append(sb);
    }

    void operator+=(CHAR ch)
    {
        ProtectedPutChar(ch);
    }

    void operator+=(const CStringA& s)
    {
        Append(s);
    }

    void operator+=(LPCSTR str)
    {
        Append(str);
    }

    operator LPCSTR() const
    {
        return reinterpret_cast<LPCSTR>(ProtectedGetBuffer());
    }

    DEFINE_COMPARE_OPERATOR_A(==)
    DEFINE_COMPARE_OPERATOR_A(!=)
    DEFINE_COMPARE_OPERATOR_A(>)
    DEFINE_COMPARE_OPERATOR_A(<)
    DEFINE_COMPARE_OPERATOR_A(>=)
    DEFINE_COMPARE_OPERATOR_A(<=)

    // append/fill
    void AppendCh(CHAR ch, int count)
    {
        ProtectedFill(ch, count);
    }

    // change a character at a position
    void SetAt(int pos, CHAR ch)
    {
        ProtectedSetChar(pos, ch);
    }

    // find character in the buffer
    int Find(CHAR ch, int pos = 0) const
    {
        return ProtectedFind(ch, pos);
    }

    // find substring in the buffer
    int Find(LPCSTR str, int pos = 0) const
    {
        return ProtectedFind(str, pos);
    }

    // find first occurrence of one of the characters
    int FindOneOf(LPCSTR str, int pos = 0) const
    {
        return ProtectedFindOneOf(str, pos);
    }
};

class AFLIB TStringBufferW : public TStringBufferGeneric
{
private:
    typedef TStringBufferGeneric super;

public:
    TStringBufferW() : super(1)
    {}
    TStringBufferW(const TStringBufferW& sb) : super(sb)
    {}
    explicit TStringBufferW(int nGrow) : super(1, nGrow)
    {}

    TStringBufferW& operator=(const TStringBufferW& sb)
    {
        ProtectedCopy(sb);
        return *this;
    }

    TStringBufferW& operator=(const CStringW& s)
    {
        Empty();
        Append(s);
        return *this;
    }

    TStringBufferW& operator=(LPCWSTR str)
    {
        Empty();
        Append(str);
        return *this;
    }

    void operator+=(const TStringBufferW& sb)
    {
        Append(sb);
    }

    void operator+=(WCHAR ch)
    {
        ProtectedPutChar(ch);
    }

    void operator+=(const CStringW& s)
    {
        Append(s);
    }

    void operator+=(LPCWSTR str)
    {
        Append(str);
    }

    operator LPCWSTR() const
    {
        return reinterpret_cast<LPCWSTR>(ProtectedGetBuffer());
    }

    DEFINE_COMPARE_OPERATOR_W(==)
    DEFINE_COMPARE_OPERATOR_W(!=)
    DEFINE_COMPARE_OPERATOR_W(>)
    DEFINE_COMPARE_OPERATOR_W(<)
    DEFINE_COMPARE_OPERATOR_W(>=)
    DEFINE_COMPARE_OPERATOR_W(<=)

    // append/fill
    void AppendCh(WCHAR ch, int count)
    {
        ProtectedFill(ch, count);
    }

    // change a character at a position
    void SetAt(int pos, WCHAR ch)
    {
        ProtectedSetChar(pos, ch);
    }

    // find character in the buffer
    int Find(WCHAR ch, int pos = 0) const
    {
        return ProtectedFind(ch, pos);
    }

    // find substring in the buffer
    int Find(LPCWSTR str, int pos = 0) const
    {
        return ProtectedFind(str, pos);
    }

    // find first occurrence of one of the characters
    int FindOneOf(LPCWSTR str, int pos = 0) const
    {
        return ProtectedFindOneOf(str, pos);
    }
};
}  // namespace AFLibIO

#undef DEFINE_COMPARE_OPERATOR
#undef DEFINE_COMPARE_OPERATOR_A
#undef DEFINE_COMPARE_OPERATOR_W
