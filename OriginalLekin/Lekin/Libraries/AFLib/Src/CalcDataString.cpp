#include "StdAfx.h"

#include "AFLibCalcData.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibWildCardMSAccess.h"
#include "Local.h"

using AFLibCalc::TCalcData;
using namespace AFLib;
using namespace AFLibPrivate;

#ifdef _DEBUG
#define COND_SS (nArg == 2 && GetT(pArg[0]) == VT_BSTR && GetT(pArg[1]) == VT_BSTR)
#define COND_SI (nArg == 2 && GetT(pArg[0]) == VT_BSTR && GetT(pArg[1]) == VT_I4)
#define COND_SSI (nArg == 3 && GetT(pArg[0]) == VT_BSTR && GetT(pArg[1]) == VT_BSTR && GetT(pArg[2]) == VT_I4)
#define COND_SII (nArg == 3 && GetT(pArg[0]) == VT_BSTR && GetT(pArg[1]) == VT_I4 && GetT(pArg[2]) == VT_I4)

#define ASSERT_S ASSERT(nArg == 1 && GetT(pArg[0]) == VT_BSTR)
#define ASSERT_SS ASSERT(COND_SS)
#define ASSERT_SSS TestManyArg(pArg, nArg, VT_BSTR)
#define ASSERT_SI ASSERT(COND_SI)
#define ASSERT_SI_OR_SII ASSERT(COND_SI || COND_SII)
#define ASSERT_SS_OR_SSI ASSERT(COND_SI || COND_SII)
#define ASSERT_ASS TestAssignArg(pArg, nArg, VT_BSTR)
#else
#define ASSERT_S
#define ASSERT_SS
#define ASSERT_SSS
#define ASSERT_SI
#define ASSERT_SI_OR_SII
#define ASSERT_SS_OR_SSI
#define ASSERT_ASS
#endif

COleVariant TCalcData::SwEqualS(COleVariant* pArg, int nArg)
{
    ASSERT_S;
    ASSERT(GetT(pArg[-1]) == VT_BSTR);
    return BYTE(Compare(pArg - 1, 2) == 0);
}

COleVariant TCalcData::AddS(COleVariant* pArg, int nArg)
{
    ASSERT_SSS;
    int len = 0;

    for (int i = 0; i < nArg; ++i) len += ::SysStringLen(GetS(pArg[i]));

    COleVariant res;
    res.vt = VT_BSTR;
    V_BSTR(&res) = AFSysAllocString(len);

    len = 0;
    for (int i = 0; i < nArg; ++i)
    {
        BSTR s = GetS(pArg[i]);
        int sz = ::SysStringLen(s);

        memcpy(V_BSTR(&res) + len, s, sz * sizeof(OLECHAR));
        len += sz;
    }

    return res;
}

COleVariant TCalcData::LengthS(COleVariant* pArg, int nArg)
{
    ASSERT_S;
    return long(::SysStringLen(GetS(pArg[0])));
}

COleVariant TCalcData::LeftS(COleVariant* pArg, int nArg)
{
    ASSERT_SI;
    BSTR s = GetS(pArg[0]);
    int len = max(min(int(GetI(pArg[1])), int(::SysStringLen(s))), 0);

    COleVariant res;
    res.vt = VT_BSTR;
    V_BSTR(&res) = AFSysAllocStringLen(s, len);
    return res;
}

COleVariant TCalcData::RightS(COleVariant* pArg, int nArg)
{
    ASSERT_SI;
    BSTR s = GetS(pArg[0]);
    int total = ::SysStringLen(s);
    int len = max(min(int(GetI(pArg[1])), total), 0);

    COleVariant res;
    res.vt = VT_BSTR;
    V_BSTR(&res) = AFSysAllocStringLen(s + total - len, len);
    return res;
}

COleVariant TCalcData::MidS(COleVariant* pArg, int nArg)
{
    ASSERT_SI_OR_SII;
    BSTR s = GetS(pArg[0]);
    int offset = GetI(pArg[1]);
    int len = ::SysStringLen(s) - offset;

    COleVariant res;
    res.vt = VT_BSTR;

    if (len < 0)
    {
        V_BSTR(&res) = AFSysAllocString(strEmptyW);
        return res;
    }

    if (nArg > 2) len = min(len, int(GetI(pArg[2])));

    V_BSTR(&res) = AFSysAllocStringLen(s + offset, len);
    return res;
}

COleVariant TCalcData::FindS(COleVariant* pArg, int nArg)
{
    ASSERT_SS_OR_SSI;
    BSTR s = GetS(pArg[0]);
    BSTR pattern = GetS(pArg[1]);
    int offset = nArg == 3 ? GetI(pArg[2]) : 0;
    int lenS = ::SysStringLen(s);
    int lenPat = ::SysStringLen(pattern);
    int offMax = lenS - lenPat;

    if (lenPat <= 0) return long(offset);

    while (offset <= offMax)
    {
        LPWSTR ss = wmemchr(s + offset, *pattern, offMax - offset + 1);
        if (ss == NULL) return long(-1);

        offset = ss - s;

        if (offset <= offMax && wmemcmp(s + offset + 1, pattern + 1, lenPat - 1) == 0) return long(offset);

        ++offset;
    }

    return long(-1);
}

COleVariant TCalcData::ExtractS(COleVariant* pArg, int nArg)
{
    ASSERT(nArg == 2 && pArg[0].vt == (VT_BSTR | VT_BYREF) && GetT(pArg[1]) == VT_BSTR);
    BSTR& s = *V_BSTRREF(&pArg[0]);
    BSTR pattern = GetS(pArg[1]);
    int lenS = ::SysStringLen(s);
    int lenPat = ::SysStringLen(pattern);

    COleVariant res;
    res.vt = VT_BSTR;

    if (lenPat <= 0)
    {
        V_BSTR(&res) = AFSysAllocString(strEmptyW);
        return res;
    }

    if (lenPat <= lenS)
    {
        LPCWSTR sEnd = s + lenS - lenPat;
        for (LPCWSTR ss = s; ss <= sEnd; ++ss)
            if (memcmp(ss, pattern, lenPat * sizeof(OLECHAR)) == 0)
            {
                int lenPrefix = ss - s;

                V_BSTR(&res) = AFSysAllocStringLen(s, lenPrefix);
                AFSysReAllocStringLen(s, ss + lenPat, lenS - lenPat - lenPrefix);
                return res;
            }
    }

    V_BSTR(&res) = s;
    s = AFSysAllocString(strEmptyW);
    return res;
}

COleVariant TCalcData::MatchS(COleVariant* pArg, int nArg)
{
    ASSERT_SS;
    const TWildCardMSAccess* pWc = GetPattern(GetS(pArg[1]));
    return BYTE(pWc->Matches(GetS(pArg[0])) ? 1 : 0);
}

BSTR TCalcData::PrivateToUL(COleVariant* pArg, int nArg, wint_t (*f)(wint_t c))
{
    ASSERT_S;
    BSTR s = AFSysAllocStringBstr(GetS(pArg[0]));
    int len = ::SysStringLen(s);

    for (int i = 0; i < len; ++i) s[i] = f(s[i]);
    return s;
}

COleVariant TCalcData::UpperS(COleVariant* pArg, int nArg)
{
    COleVariant res;
    res.vt = VT_BSTR;
    V_BSTR(&res) = PrivateToUL(pArg, nArg, towupper);
    return res;
}

COleVariant TCalcData::LowerS(COleVariant* pArg, int nArg)
{
    COleVariant res;
    res.vt = VT_BSTR;
    V_BSTR(&res) = PrivateToUL(pArg, nArg, towlower);
    return res;
}

COleVariant TCalcData::IntS(COleVariant* pArg, int nArg)
{
    ASSERT_S;
    try
    {
        COleVariant d;
        d.ChangeType(VT_I4, pArg);
        return d;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        return -1L;
    }
}

COleVariant TCalcData::DoubleS(COleVariant* pArg, int nArg)
{
    ASSERT_S;
    try
    {
        COleVariant d;
        d.ChangeType(VT_R8, pArg);
        return d;
    }
    catch (CException* pExc)
    {
        pExc->Delete();
        return AFLibMath::Nan;
    }
}

COleVariant TCalcData::AssignS(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    AFSysReAllocStringBstr(*V_BSTRREF(pArg), GetS(pArg[1]));
    return pArg[0];
}

COleVariant TCalcData::AssignAddS(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    COleVariant res = AddS(pArg, nArg);

    ::SysFreeString(*V_BSTRREF(pArg));
    *V_BSTRREF(pArg) = V_BSTR(&res);
    res.vt = VT_EMPTY;
    V_BSTR(&res) = NULL;
    return pArg[0];
}
