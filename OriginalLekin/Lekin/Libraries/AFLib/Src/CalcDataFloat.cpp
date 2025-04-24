#include "StdAfx.h"

#include "AFLibCalcData.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibRandom.h"
#include "AFLibStringBuffer.h"
#include "AFLibThreadSafeStatic.h"
#include "Local.h"

using AFLibCalc::TCalcData;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibMath;
using namespace AFLibThread;
using namespace AFLibPrivate;

#ifdef _DEBUG
#define COND_F (nArg == 1 && IsArgF(pArg[0]))
#define COND_FF (nArg == 2 && IsArgF(pArg[0]) && IsArgF(pArg[1]))
#define COND_FI (nArg == 2 && IsArgF(pArg[0]) && pArg[1].vt == VT_I4)

#define ASSERT_F ASSERT(COND_F)
#define ASSERT_FF ASSERT(COND_FF)
#define ASSERT_FFF TestFFF(pArg, nArg)
#define ASSERT_F_OR_FI ASSERT(COND_F || COND_FI)
#define ASSERT_F_OR_FF ASSERT(COND_F || COND_FF)
#define ASSERT_ASS TestAssignArg(pArg, nArg, VT_R8)
#else
#define ASSERT_F
#define ASSERT_FF
#define ASSERT_FFF
#define ASSERT_F_OR_FI
#define ASSERT_F_OR_FF
#define ASSERT_ASS
#endif

COleVariant TCalcData::RandomF(COleVariant* pArg, int nArg)
{
    return TRandom::Get().Uniform();
}

COleVariant TCalcData::NegateF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    return -GetF(pArg[0]);
}

COleVariant TCalcData::AddF(COleVariant* pArg, int nArg)
{
    ASSERT_FFF;
    COleVariant res(0.);

    for (int i = 0; i < nArg; ++i) V_R8(&res) += GetF(pArg[i]);

    return res;
}

COleVariant TCalcData::SubtractF(COleVariant* pArg, int nArg)
{
    ASSERT_FF;
    return GetF(pArg[0]) - GetF(pArg[1]);
}

COleVariant TCalcData::MultiplyF(COleVariant* pArg, int nArg)
{
    ASSERT_FFF;
    COleVariant res(1.);

    for (int i = 0; i < nArg; ++i) V_R8(&res) *= GetF(pArg[i]);

    return res;
}

COleVariant TCalcData::DivideF(COleVariant* pArg, int nArg)
{
    ASSERT_FF;
    return GetF(pArg[0]) / GetF(pArg[1]);
}

COleVariant TCalcData::ModuloF(COleVariant* pArg, int nArg)
{
    ASSERT_FF;
    double n = floor(GetF(*pArg[0]) / GetF(*pArg[1]));
    return GetF(pArg[0]) - GetF(pArg[1]) * n;
}

COleVariant TCalcData::PowerF(COleVariant* pArg, int nArg)
{
    ASSERT_FF;
    return pow(GetF(pArg[0]), GetF(pArg[1]));
}

COleVariant TCalcData::AbsF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    return fabs(GetF(pArg[0]));
}

COleVariant TCalcData::MaxF(COleVariant* pArg, int nArg)
{
    ASSERT_FFF;
    COleVariant res = pArg[0];

    for (int i = 1; i < nArg; ++i) V_R8(&res) = max(V_R8(&res), GetF(pArg[i]));

    return res;
}

COleVariant TCalcData::MinF(COleVariant* pArg, int nArg)
{
    ASSERT_FFF;
    COleVariant res = pArg[0];

    for (int i = 1; i < nArg; ++i) V_R8(&res) = min(V_R8(&res), GetF(pArg[i]));

    return res;
}

COleVariant TCalcData::IsNanF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    return BYTE(IsNan(GetF(pArg[0])));
}

COleVariant TCalcData::FloorF(COleVariant* pArg, int nArg)
{
    ASSERT_F_OR_FI;
    double coef = nArg == 1 ? 1. : pow(10., -GetF(pArg[1]));
    return coef * floor(GetF(pArg[0]) / coef);
}

COleVariant TCalcData::CeilF(COleVariant* pArg, int nArg)
{
    ASSERT_F_OR_FI;
    double coef = nArg == 1 ? 1. : pow(10., -GetF(pArg[1]));
    return coef * ceil(GetF(pArg[0]) / coef);
}

COleVariant TCalcData::RoundF(COleVariant* pArg, int nArg)
{
    ASSERT_F_OR_FI;
    double coef = nArg == 1 ? 1. : pow(10., -GetF(pArg[1]));
    return coef * floor(GetF(pArg[0]) / coef + 0.5);
}

COleVariant TCalcData::SqrtF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    return sqrt(GetF(pArg[0]));
}

COleVariant TCalcData::ExpF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    return exp(GetF(pArg[0]));
}

COleVariant TCalcData::LogF(COleVariant* pArg, int nArg)
{
    ASSERT_F_OR_FF;
    COleVariant res(log(GetF(pArg[0])));

    if (nArg == 2) V_R8(&res) /= log(GetF(pArg[1]));
    return res;
}

COleVariant TCalcData::IntF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    double a = GetF(pArg[0]);

    if (IsNan(a)) ThrowMessage(LocalAfxString(IDS_CONVERT_NAN_INT));
    return long(Rint(a));
}

COleVariant TCalcData::StringF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    SAFE_STATIC_SB(buffer);

    buffer.AppendDouble(GetF(pArg[0]));
    return LPCTSTR(buffer);
}

COleVariant TCalcData::StringPercentF(COleVariant* pArg, int nArg)
{
    ASSERT_F;
    SAFE_STATIC_SB(buffer);

    buffer.AppendPercent(GetF(pArg[0]));
    return LPCTSTR(buffer);
}

COleVariant TCalcData::AssignF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_R8REF(pArg) = GetF(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignAddF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_R8REF(pArg) += GetF(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignSubtractF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_R8REF(pArg) -= GetF(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignMultiplyF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_R8REF(pArg) *= GetF(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignDivideF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_R8REF(pArg) /= GetF(pArg[1]);
    return pArg[0];
}

COleVariant TCalcData::AssignModuloF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    double n = floor(GetF(*pArg[0]) / GetF(*pArg[1]));
    *V_R8REF(pArg) = GetF(pArg[0]) - GetF(pArg[1]) * n;
    return pArg[0];
}

COleVariant TCalcData::AssignPowerF(COleVariant* pArg, int nArg)
{
    ASSERT_ASS;
    *V_R8REF(pArg) = pow(GetF(pArg[0]), GetF(pArg[1]));
    return pArg[0];
}
