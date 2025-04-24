#pragma once

#include "AFLibSimpleType.h"

/////////////////////////////////////////////////////////////////////////////
// TFuncPrototype class
// Function prototype: types only, no argument names

namespace AFLibCalc {
class AFLIB TFuncPrototype
{
public:
    // flag values for m_flags:
    //   last argument extended to infinity; push SP before call
    enum
    {
        fChain = 1,
        fPushSP = 2
    };

private:
    TSimpleType m_stpRet;             // return type
    CArray<TSimpleType> m_arrStpArg;  // argument types
    int m_flags;                      // flags

public:
    TFuncPrototype();
    TFuncPrototype(const TFuncPrototype& fpr);
    TFuncPrototype(const TSimpleType& stpRet, const TSimpleType& stpArg, int flags = 0);
    TFuncPrototype(
        const TSimpleType& stpRet, const TSimpleType& stpArg1, const TSimpleType& stpArg2, int flags = 0);
    TFuncPrototype(const TSimpleType& stpRet,
        const TSimpleType& stpArg1,
        const TSimpleType& stpArg2,
        const TSimpleType& stpArg3,
        int flags = 0);
    TFuncPrototype(const TSimpleType& stpRet, LPCTSTR arg, int flags = 0);
    TFuncPrototype& operator=(const TFuncPrototype& fpr);

    bool operator==(const TFuncPrototype& fpr) const;

    bool operator!=(const TFuncPrototype& fpr) const
    {
        return !(*this == fpr);
    }

    // add an argument
    void AddArg(const TSimpleType& stp);

    // accepts argument types listed in fpr
    bool Accepts(const TFuncPrototype& fpr) const;

    // print arguments into a string
    CString ArgToString() const;

    // get return type
    const TSimpleType& GetReturnType() const
    {
        return m_stpRet;
    }

    // no arguments?
    bool IsNoArg() const
    {
        return m_arrStpArg.IsEmpty() != 0;
    }

    // get number of function arguments
    int GetArgCount() const
    {
        return m_arrStpArg.GetSize();
    }

    // get argument with given index
    const TSimpleType& GetArg(int index) const
    {
        return m_arrStpArg[index];
    }

    // need to push SP before calling?
    bool IsPushSP() const
    {
        return (m_flags & fPushSP) != 0;
    }
};
}  // namespace AFLibCalc
