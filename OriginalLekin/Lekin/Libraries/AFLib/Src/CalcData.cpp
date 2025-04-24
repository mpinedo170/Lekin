#include "StdAfx.h"

#include "AFLibCalcData.h"

#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "AFLibWildCardMSAccess.h"
#include "Local.h"

using namespace AFLibCalc;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibMath;
using namespace AFLibPrivate;

const TCalcData::TNamerElement TCalcData::m_neEnd = {NULL, _T("")};

#ifdef _DEBUG
#define ASSERT_INIT ASSERT(nArg == 2 && pArg[0].vt == (VT_BYREF | VT_VARIANT))
#else
#define ASSERT_INIT
#endif

TCalcData::TCalcData()
{}

TCalcData::~TCalcData()
{
    m_mapWc.DestroyAll();
}

const TWildCardMSAccess* TCalcData::GetPattern(BSTR s)
{
    TWildCardMSAccess* pWc = m_mapWc.GetAt(s);
    if (pWc != NULL) return pWc;

    pWc = new TWildCardMSAccess(s);
    m_mapWc.SetAt(s, pWc);
    return pWc;
}

void TCalcData::TestManyArg(COleVariant* pArg, int nArg, int type) const
{
    ASSERT(nArg > 0);

    for (int i = 0; i < nArg; ++i) ASSERT(GetT(pArg[i]) == type);
}

void TCalcData::TestAssignArg(COleVariant* pArg, int nArg, int type) const
{
    ASSERT(nArg == 2 && pArg[0].vt == (VT_BYREF | type));

    if (type == VT_R8)
        ASSERT(IsArgF(pArg[1]));
    else
        ASSERT(GetT(pArg[1]) == type);
}

bool TCalcData::AreArgsCompat(COleVariant& var1, COleVariant& var2)
{
    return GetT(var1) == GetT(var2) || (IsArgF(var1) && IsArgF(var2));
}

void TCalcData::TestFFF(COleVariant* pArg, int nArg) const
{
    ASSERT(nArg > 0);

    for (int i = 0; i < nArg; ++i) ASSERT(IsArgF(pArg[i]));
}

double TCalcData::GetF(const COleVariant& v)
{
    switch (v.vt)
    {
        case VT_I4:
        case VT_I4 | VT_BYREF:
            return GetI(v);
        case VT_R8:
            return V_R8(&v);
        case VT_R8 | VT_BYREF:
            return *V_R8REF(&v);
    }

    ASSERT(false);
    return 0;
}

const CString& TCalcData::B2S(bool b)
{
    SAFE_CONST_STR(strT, _T("true"));
    SAFE_CONST_STR(strF, _T("false"));
    return b ? strT : strF;
}

int TCalcData::Compare(COleVariant* pArg, int nArg) const
{
    ASSERT(nArg == 2 && AreArgsCompat(pArg[0], pArg[1]));

    // both numeric, maybe different, one is float
    if (GetT(pArg[0]) == VT_R8 || GetT(pArg[1]) == VT_R8)
    {
        double a = GetF(pArg[0]) - GetF(pArg[1]);
        if (IsNan(a)) ThrowMessage(LocalAfxString(IDS_COMPARE_NAN));
        return sgn(a);
    }

    switch (GetT(pArg[0]))
    {
        case VT_UI1:
            return int(GetB(pArg[0])) - int(GetB(pArg[1]));
        case VT_I4:
            return GetI(pArg[0]) - GetI(pArg[1]);
        case VT_BSTR:
        {
            int a = VarBstrCmp(GetS(pArg[0]), GetS(pArg[1]), LOCALE_USER_DEFAULT, 0);
            return a == VARCMP_LT ? -1 : a == VARCMP_GT ? 1 : 0;
        }
    }

    ASSERT(false);
    return 0;
}

COleVariant TCalcData::CreateRefToVar(VARIANT& var)
{
    if (var.vt == (VT_BYREF | VT_VARIANT)) return var;
    ASSERT(!(var.vt & VT_BYREF));

    COleVariant varRef;
    varRef.vt = VT_VARIANT | VT_BYREF;
    V_VARIANTREF(&varRef) = &var;
    return varRef;
}

COleVariant TCalcData::CreateRef(VARIANT& var)
{
    if (var.vt & VT_BYREF) return var;

    COleVariant varRef;
    varRef.vt = var.vt | VT_BYREF;

    if ((var.vt & VT_ARRAY) != 0)
        V_ARRAYREF(&varRef) = &V_ARRAY(&var);
    else
    {
        switch (var.vt)
        {
            case VT_UI1:
                V_UI1REF(&varRef) = &V_UI1(&var);
                break;

            case VT_I4:
                V_I4REF(&varRef) = &V_I4(&var);
                break;

            case VT_R8:
                V_R8REF(&varRef) = &V_R8(&var);
                break;

            case VT_BSTR:
                V_BSTRREF(&varRef) = &V_BSTR(&var);
                break;

            default:
                ASSERT(false);
                break;
        }
    }

    return varRef;
}

COleVariant TCalcData::Less(COleVariant* pArg, int nArg)
{
    return BYTE(Compare(pArg, nArg) < 0);
}

COleVariant TCalcData::NotLess(COleVariant* pArg, int nArg)
{
    return BYTE(Compare(pArg, nArg) >= 0);
}

COleVariant TCalcData::More(COleVariant* pArg, int nArg)
{
    return BYTE(Compare(pArg, nArg) > 0);
}

COleVariant TCalcData::NotMore(COleVariant* pArg, int nArg)
{
    return BYTE(Compare(pArg, nArg) <= 0);
}

COleVariant TCalcData::Equal(COleVariant* pArg, int nArg)
{
    return BYTE(Compare(pArg, nArg) == 0);
}

COleVariant TCalcData::NotEqual(COleVariant* pArg, int nArg)
{
    return BYTE(Compare(pArg, nArg) != 0);
}

COleVariant TCalcData::Print(COleVariant* pArg, int nArg)
{
    SAFE_STATIC_SB(buffer);

    for (int i = 0; i < nArg; ++i) buffer.AppendVariant(pArg[i]);

    ProtectedPrint(buffer);
    return long(nArg);
}

COleVariant TCalcData::Initialize(COleVariant* pArg, int nArg)
{
    ASSERT_INIT;
    COleVariant& var = *static_cast<COleVariant*>(V_VARIANTREF(pArg));

    var = pArg[1];
    if (var.vt & VT_BYREF) var.ChangeType(var.vt & ~VT_BYREF, &pArg[1]);

    return CreateRef(var);
}

const CString& TCalcData::ProtectedFuncToName(TCalcFunction f, const TNamerElement* arrNe) const
{
    for (int i = 0; arrNe[i].m_f != NULL; ++i)
        if (arrNe[i].m_f == f) return arrNe[i].m_name;

    return strEmpty;
}

void TCalcData::ProtectedPrint(LPCTSTR str)
{
    StandardOut(str);
    StandardOut(strEol);
}

#define FUNC(name)                  \
    {                               \
        &TCalcData::name, _T(#name) \
    }

const CString& TCalcData::FuncToName(TCalcFunction f) const
{
    static const TNamerElement arrNe[] = {FUNC(Less), FUNC(NotLess), FUNC(More), FUNC(NotMore), FUNC(Equal),
        FUNC(NotEqual), FUNC(Print), FUNC(Initialize),

        FUNC(NotB), FUNC(AndB), FUNC(OrB), FUNC(XorB), FUNC(IntB), FUNC(DoubleB), FUNC(StringB),
        FUNC(AssignB), FUNC(AssignAndB), FUNC(AssignOrB), FUNC(AssignXorB),

        FUNC(SwEqualI), FUNC(NegateI), FUNC(NotI), FUNC(AndI), FUNC(OrI), FUNC(XorI), FUNC(AddI),
        FUNC(SubtractI), FUNC(MultiplyI), FUNC(DivideI), FUNC(ModuloI), FUNC(AbsI), FUNC(MaxI), FUNC(MinI),
        FUNC(StringI), FUNC(AssignI), FUNC(AssignAndI), FUNC(AssignOrI), FUNC(AssignXorI), FUNC(AssignAddI),
        FUNC(AssignSubtractI), FUNC(AssignMultiplyI), FUNC(AssignDivideI), FUNC(AssignModuloI),
        FUNC(IncrementI), FUNC(DecrementI),

        FUNC(NegateF), FUNC(AddF), FUNC(SubtractF), FUNC(MultiplyF), FUNC(DivideF), FUNC(ModuloF),
        FUNC(PowerF), FUNC(AbsF), FUNC(MaxF), FUNC(MinF), FUNC(IsNanF), FUNC(FloorF), FUNC(CeilF),
        FUNC(RoundF), FUNC(SqrtF), FUNC(ExpF), FUNC(LogF), FUNC(IntF), FUNC(StringF), FUNC(AssignF),
        FUNC(AssignAddF), FUNC(AssignSubtractF), FUNC(AssignMultiplyF), FUNC(AssignDivideF),
        FUNC(AssignModuloF), FUNC(AssignPowerF),

        FUNC(SwEqualS), FUNC(AddS), FUNC(LengthS), FUNC(LeftS), FUNC(RightS), FUNC(MidS), FUNC(FindS),
        FUNC(ExtractS), FUNC(MatchS), FUNC(UpperS), FUNC(LowerS), FUNC(IntS), FUNC(DoubleS), FUNC(AssignS),
        FUNC(AssignAddS),

        FUNC(InitializeA), FUNC(SubscriptA), FUNC(LengthA), FUNC(RedimA), FUNC(MaxAF), FUNC(MaxAI),
        FUNC(MinAF), FUNC(MinAI), FUNC(PrintAB), FUNC(PrintAI), FUNC(PrintAF), FUNC(PrintAS),

        m_neEnd};

    return ProtectedFuncToName(f, arrNe);
}
