#include "StdAfx.h"

#include "AFLibCalcData.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeStatic.h"

using AFLibCalc::TCalcData;
using namespace AFLibIO;

#ifdef _DEBUG
#define COND_I (nArg == 1 && GetT(pArg[0]) == VT_I4)
#define COND_IS (nArg == 2 && GetT(pArg[0]) == VT_I4 && GetT(pArg[0]) == VT_BSTR)

#define ASSERT_I ASSERT(COND_I)
#define ASSERT_II ASSERT(nArg == 2 && GetT(pArg[0]) == VT_I4 && GetT(pArg[1]) == VT_I4)
#define ASSERT_III TestManyArg(pArg, nArg, VT_I4)
#define ASSERT_I_OR_IS ASSERT(COND_I || COND_IS)
#define ASSERT_ASS TestAssignArg(pArg, nArg, VT_I4)
#define ASSERT_INC ASSERT(nArg == 1 && pArg[0].vt == (VT_I4 | VT_BYREF))
#else
#define ASSERT_I
#define ASSERT_II
#define ASSERT_III
#define ASSERT_I_OR_IS
#define ASSERT_ASS
#define ASSERT_INC
#endif

COleVariant TCalcData::SwEqualI(COleVariant* pArg, int nArg)
{
    ASSERT_I;
    ASSERT(GetT(pArg[-1]) == VT_I4);
    return BYTE(GetI(pArg[0]) == GetI(pArg[-1]));
}

COleVariant TCalcData::NegateI(COleVariant* pArg, int nArg)
{
    ASSERT_I;
    return -GetI(pArg[0]);
}

COleVariant TCalcData::NotI(COleVariant* pArg, int nArg)
{
    ASSERT_I;
    return ~GetI(pArg[0]);
}

COleVariant TCalcData::AndI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res(-1L);

    for (int i = 0; i < nArg; ++i) V_I4(&res) &= GetI(pArg[i]);

    return res;
}

COleVariant TCalcData::OrI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res(0L);

    for (int i = 0; i < nArg; ++i) V_I4(&res) |= GetI(pArg[i]);

    return res;
}

COleVariant TCalcData::XorI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res(0L);

    for (int i = 0; i < nArg; ++i) V_I4(&res) ^= GetI(pArg[i]);

    return res;
}

COleVariant TCalcData::ShlI(COleVariant* pArg, int nArg)
{
    ASSERT_II;
    return GetI(pArg[0]) << GetI(pArg[1]);
}

COleVariant TCalcData::ShrI(COleVariant* pArg, int nArg)
{
    ASSERT_II;
    return GetI(pArg[0]) >> GetI(pArg[1]);
}

COleVariant TCalcData::AddI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res(0L);

    for (int i = 0; i < nArg; ++i) V_I4(&res) += GetI(pArg[i]);

    return res;
}

COleVariant TCalcData::SubtractI(COleVariant* pArg, int nArg)
{
    ASSERT_II;
    return GetI(pArg[0]) - GetI(pArg[1]);
}

COleVariant TCalcData::MultiplyI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res(1L);

    for (int i = 0; i < nArg; ++i) V_I4(&res) *= GetI(pArg[i]);

    return res;
}

COleVariant TCalcData::DivideI(COleVariant* pArg, int nArg)
{
    ASSERT_II;
    return GetI(pArg[0]) / GetI(pArg[1]);
}

COleVariant TCalcData::ModuloI(COleVariant* pArg, int nArg)
{
    ASSERT_II;
    return GetI(pArg[0]) % GetI(pArg[1]);
}

COleVariant TCalcData::AbsI(COleVariant* pArg, int nArg)
{
    ASSERT_I;
    long l = GetI(pArg[0]);
    return l < 0 ? -l : l;
}

COleVariant TCalcData::MaxI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res = pArg[0];

    for (int i = 1; i < nArg; ++i) V_I4(&res) = max(V_I4(&res), GetI(pArg[i]));

    return res;
}

COleVariant TCalcData::MinI(COleVariant* pArg, int nArg)
{
    ASSERT_III;
    COleVariant res = pArg[0];

    for (int i = 1; i < nArg; ++i) V_I4(&res) = min(V_I4(&res), GetI(pArg[i]));

    return res;
}

COleVariant TCalcData::StringI(COleVariant* pArg, int nArg)
{
    ASSERT_I_OR_IS;

    if (nArg == 2)
    {
        CString format(GetS(pArg[1]));
        CString s;
        s.Format(format, GetI(pArg[0]));
        return s;
    }
    else
    {
        SAFE_STATIC_SB(buffer);
        buffer.AppendInt(GetI(pArg[0]));
        return LPCTSTR(buffer);
    }
}

COleVariant TCalcData::AssignI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) = GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignAndI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) &= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignOrI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) |= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignXorI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) ^= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignShlI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) <<= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignShrI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) >>= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignAddI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) += GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignSubtractI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) -= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignMultiplyI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) *= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignDivideI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) /= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignModuloI(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_I4REF(pArg) %= GetI(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::IncrementI(COleVariant* pArg, int nArg)
{
    ASSERT_INC;
    COleVariant res = GetI(pArg[0]);
    ++*V_I4REF(pArg);
    return res;
}

COleVariant TCalcData::DecrementI(COleVariant* pArg, int nArg)
{
    ASSERT_INC;
    COleVariant res = GetI(pArg[0]);
    --*V_I4REF(pArg);
    return res;
}
