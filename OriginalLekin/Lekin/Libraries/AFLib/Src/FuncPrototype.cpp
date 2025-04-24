#include "StdAfx.h"

#include "AFLibFuncPrototype.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeConst.h"

using namespace AFLibCalc;
using namespace AFLib;
using namespace AFLibIO;

TFuncPrototype::TFuncPrototype()
{
    m_flags = 0;
}

TFuncPrototype::TFuncPrototype(const TFuncPrototype& fpr)
{
    m_arrStpArg.SetSize(0, 1 << 4);
    *this = fpr;
}

TFuncPrototype::TFuncPrototype(const TSimpleType& stpRet, const TSimpleType& stpArg, int flags) :
    m_stpRet(stpRet),
    m_flags(flags)
{
    AddArg(stpArg);
}

TFuncPrototype::TFuncPrototype(
    const TSimpleType& stpRet, const TSimpleType& stpArg1, const TSimpleType& stpArg2, int flags) :
    m_stpRet(stpRet),
    m_flags(flags)
{
    AddArg(stpArg1);
    AddArg(stpArg2);
}

TFuncPrototype::TFuncPrototype(const TSimpleType& stpRet,
    const TSimpleType& stpArg1,
    const TSimpleType& stpArg2,
    const TSimpleType& stpArg3,
    int flags) :
    m_stpRet(stpRet),
    m_flags(flags)
{
    AddArg(stpArg1);
    AddArg(stpArg2);
    AddArg(stpArg3);
}

TFuncPrototype::TFuncPrototype(const TSimpleType& stpRet, LPCTSTR arg, int flags) :
    m_stpRet(stpRet),
    m_flags(flags)
{
    SAFE_CONST_STR(strType, _T("vbifs"));

    m_arrStpArg.SetSize(_tcslen(arg));

    for (int i = 0;; ++i)
    {
        TCHAR ch = arg[i];
        if (ch == 0) break;

        int k = strType.Find(ToLower(ch));
        ASSERT(k >= 0);
        m_arrStpArg[i] = TSimpleType(TElementaryType(k), IsUpper(ch) ? tmdRef : tmdSimple);
    }
}

TFuncPrototype& TFuncPrototype::operator=(const TFuncPrototype& fpr)
{
    if (this == &fpr) return *this;
    m_stpRet = fpr.m_stpRet;
    m_flags = fpr.m_flags;
    m_arrStpArg.Copy(fpr.m_arrStpArg);
    return *this;
}

bool TFuncPrototype::operator==(const TFuncPrototype& fpr) const
{
    if (m_stpRet != fpr.m_stpRet || m_flags != fpr.m_flags || GetArgCount() != fpr.GetArgCount())
        return false;

    for (int i = 0; i < GetArgCount(); ++i)
        if (GetArg(i) != fpr.GetArg(i)) return false;

    return true;
}

void TFuncPrototype::AddArg(const TSimpleType& stp)
{
    m_arrStpArg.Add(stp);
}

bool TFuncPrototype::Accepts(const TFuncPrototype& fpr) const
{
    int lenP = GetArgCount();
    int lenA = fpr.GetArgCount();

    if (lenA != lenP)
    {  // if this is not a "chained operation", the number of arguments must be the same
        if ((m_flags & fChain) == 0) return false;

        // if this IS a "chained operation", need to have AT LEAST the arguments in the prototype
        if (lenA < lenP) return false;
    }

    // if came here and no arguments in the prototype, the check is complete
    if (lenP == 0) return true;

    for (int i = 0; i < lenA; ++i)
        if (!GetArg(min(i, lenP - 1)).Accepts(fpr.GetArg(i))) return false;

    return true;
}

CString TFuncPrototype::ArgToString() const
{
    TStringBuffer buffer;
    buffer += _T('(');

    for (int i = 0; i < GetArgCount(); ++i)
    {
        if (i > 0) buffer += chrComma;
        buffer += GetArg(i).ToString();
    }

    buffer += _T(')');
    return LPCTSTR(buffer);
}
