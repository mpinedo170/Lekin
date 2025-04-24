#include "StdAfx.h"

#include "AFLibCalcData.h"

using AFLibCalc::TCalcData;

#ifdef _DEBUG
#define ASSERT_B ASSERT(nArg == 1 && GetT(pArg[0]) == VT_UI1)
#define ASSERT_BBB TestManyArg(pArg, nArg, VT_UI1)
#define ASSERT_ASS TestAssignArg(pArg, nArg, VT_UI1)
#else
#define ASSERT_B
#define ASSERT_BBB
#define ASSERT_ASS
#endif

COleVariant TCalcData::NotB(COleVariant* pArg, int nArg)
{
    ASSERT_B;
    return BYTE(!GetB(pArg[0]));
}

COleVariant TCalcData::AndB(COleVariant* pArg, int nArg)
{
    ASSERT_BBB;
    COleVariant res(BYTE(0));

    for (int i = 0; i < nArg; ++i)
        if (GetB(pArg[i]) == 0) return res;

    V_UI1(&res) = 1;
    return res;
}

COleVariant TCalcData::OrB(COleVariant* pArg, int nArg)
{
    ASSERT_BBB;
    COleVariant res(BYTE(1));

    for (int i = 0; i < nArg; ++i)
        if (GetB(pArg[i]) == 1) return res;

    V_UI1(&res) = 0;
    return res;
}

COleVariant TCalcData::XorB(COleVariant* pArg, int nArg)
{
    ASSERT_BBB;
    COleVariant res(BYTE(0));

    for (int i = 0; i < nArg; ++i) V_UI1(&res) ^= GetB(pArg[i]);

    return res;
}

COleVariant TCalcData::IntB(COleVariant* pArg, int nArg)
{
    ASSERT_B;
    return GetB(pArg[0]) ? 1L : 0L;
}

COleVariant TCalcData::DoubleB(COleVariant* pArg, int nArg)
{
    ASSERT_B;
    return GetB(pArg[0]) ? 1. : 0.;
}

COleVariant TCalcData::StringB(COleVariant* pArg, int nArg)
{
    ASSERT_B;
    return COleVariant(B2S(GetB(pArg[0]) != 0));
}

COleVariant TCalcData::AssignB(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_UI1REF(pArg) = GetB(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignAndB(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_UI1REF(pArg) &= GetB(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignOrB(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_UI1REF(pArg) |= GetB(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignXorB(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_UI1REF(pArg) ^= GetB(pArg[1]);
    return pArg[0];
}
