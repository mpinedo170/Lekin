#include "StdAfx.h"

#include "AFLibCalcData.h"

#include "AFLibGlobal.h"
#include "AFLibStringBuffer.h"
#include "Local.h"

using AFLibCalc::TCalcData;
using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibPrivate;

#ifdef _DEBUG
#define COND_ARRAY (pArg[0].vt & (VT_ARRAY | VT_BYREF)) == (VT_ARRAY | VT_BYREF)
#define COND_A (nArg == 1 && COND_ARRAY)

#define ASSERT_A ASSERT(COND_A)
#define ASSERT_AI ASSERT(nArg == 2 && COND_ARRAY && GetT(pArg[1]) == VT_I4)
#define ASSERT_AII ASSERT(nArg == 3 && COND_ARRAY && GetT(pArg[1]) == VT_I4 && GetT(pArg[2]) == VT_I4)
#define ASSERT_Abool ASSERT(nArg == 1 && pArg[0].vt == (VT_UI1 | VT_ARRAY | VT_BYREF))
#define ASSERT_Aint ASSERT(nArg == 1 && pArg[0].vt == (VT_I4 | VT_ARRAY | VT_BYREF))
#define ASSERT_Adbl ASSERT(nArg == 1 && pArg[0].vt == (VT_R8 | VT_ARRAY | VT_BYREF))
#define ASSERT_Astr ASSERT(nArg == 1 && pArg[0].vt == (VT_BSTR | VT_ARRAY | VT_BYREF))
#define ASSERT_INIT TestArrInitArg(pArg, nArg)
#else
#define ASSERT_A
#define ASSERT_AI
#define ASSERT_AII
#define ASSERT_Abool
#define ASSERT_Aint
#define ASSERT_Adbl
#define ASSERT_Astr
#define ASSERT_INIT
#endif

void TCalcData::TestArrInitArg(COleVariant* pArg, int nArg) const
{
    ASSERT(nArg >= 3 && pArg[0].vt == (VT_BYREF | VT_VARIANT) && GetT(pArg[1]) == VT_I4);

    VARTYPE vt = GetT(pArg[2]);
    ASSERT((vt & VT_ARRAY) == 0);

    for (int i = 3; i < nArg; ++i) ASSERT(GetT(pArg[i]) == vt || (GetT(pArg[i]) == VT_I4 && vt == VT_R8));
}

COleVariant TCalcData::InitializeA(COleVariant* pArg, int nArg)
{
    ASSERT_INIT;

    COleSafeArray& arr = *static_cast<COleSafeArray*>(V_VARIANTREF(pArg));
    VARTYPE vt = GetT(pArg[2]);
    int length = max(int(GetI(pArg[1])), nArg - 2);

    arr.Clear();
    arr.CreateOneDim(vt, length);

    switch (vt)
    {
        case VT_UI1:
        {
            LPBYTE pData = reinterpret_cast<LPBYTE>(arr.parray->pvData);
            for (int i = 0; i < nArg - 2; ++i) pData[i] = GetB(pArg[i + 2]);
            break;
        }

        case VT_I4:
        {
            LPLONG pData = reinterpret_cast<LPLONG>(arr.parray->pvData);
            for (int i = 0; i < nArg - 2; ++i) pData[i] = GetI(pArg[i + 2]);
            break;
        }

        case VT_R8:
        {
            double* pData = reinterpret_cast<double*>(arr.parray->pvData);
            for (int i = 0; i < nArg - 2; ++i) pData[i] = GetF(pArg[i + 2]);
            break;
        }

        case VT_BSTR:
        {
            BSTR* pData = reinterpret_cast<BSTR*>(arr.parray->pvData);
            int i = 0;

            for (; i < nArg - 2; ++i) AFSysReAllocStringBstr(pData[i], GetS(pArg[i + 2]));

            for (; i < length; ++i) AFSysReAllocString(pData[i], strEmptyW);

            break;
        }

        default:
            ASSERT(false);
    }

    return CreateRef(arr);
}

COleVariant TCalcData::SubscriptA(COleVariant* pArg, int nArg)
{
    ASSERT_AI;
    SAFEARRAY* pArray = *pArg[0].pparray;
    int index = GetI(pArg[1]);

    if (index < 0 || ULONG(index) >= pArray->rgsabound->cElements)
        ThrowMessage(LocalAfxString(IDS_OUT_OF_BOUND));

    COleVariant res;
    res.vt = (pArg[0].vt & ~VT_ARRAY) | VT_BYREF;

    switch (GetT(res))
    {
        case VT_UI1:
            V_UI1REF(&res) = reinterpret_cast<LPBYTE>(pArray->pvData) + index;
            break;

        case VT_I4:
            V_I4REF(&res) = reinterpret_cast<LPLONG>(pArray->pvData) + index;
            break;

        case VT_R8:
            V_R8REF(&res) = reinterpret_cast<double*>(pArray->pvData) + index;
            break;

        case VT_BSTR:
            V_BSTRREF(&res) = reinterpret_cast<BSTR*>(pArray->pvData) + index;
            break;

        default:
            ASSERT(false);
            break;
    }

    return res;
}

COleVariant TCalcData::LengthA(COleVariant* pArg, int nArg)
{
    ASSERT_A;
    SAFEARRAY* pArray = *pArg[0].pparray;
    return long(pArray->rgsabound->cElements);
}

COleVariant TCalcData::RedimA(COleVariant* pArg, int nArg)
{
    ASSERT_AII;
    SAFEARRAY* pArray = *pArg[0].pparray;
    int lenOld = pArray->rgsabound->cElements;
    int lenNew = GetI(pArg[1]);

    if (lenNew == lenOld || lenNew <= 0) return BYTE(0);

    LPBYTE pStart = reinterpret_cast<LPBYTE>(pArray->pvData);
    LPBYTE pEnd = pStart + lenOld * pArray->cbElements;
    int vt = pArg[0].vt & ~VT_ARRAY;
    const COleVariant* pStack = pArg + nArg - GetI(pArg[2]);

    for (; pStack < pArg; ++pStack)
    {
        if (pStack->vt != vt) continue;

        LPBYTE pVar = V_UI1REF(pStack);
        if (pVar >= pStart && pVar < pEnd) ThrowMessage(LocalAfxString(IDS_BAD_REDIM));
    }

    SAFEARRAYBOUND rgsabound = {static_cast<ULONG>(lenNew), 0};
    AfxCheckError(::SafeArrayRedim(pArray, &rgsabound));

    if ((pArg[0].vt & ~(VT_ARRAY | VT_BYREF)) == VT_BSTR)
    {
        BSTR* pData = reinterpret_cast<BSTR*>(pArray->pvData);
        for (int i = lenOld; i < lenNew; ++i) AFSysReAllocString(pData[i], strEmptyW);
    }

    return BYTE(1);
}

COleVariant TCalcData::MaxAF(COleVariant* pArg, int nArg)
{
    ASSERT_Adbl;
    SAFEARRAY* pArray = *pArg[0].pparray;
    const double* pData = reinterpret_cast<const double*>(pArray->pvData);
    double mx = pData[0];

    for (int i = pArray->rgsabound->cElements; --i >= 1;) mx = max(mx, pData[i]);

    return mx;
}

COleVariant TCalcData::MaxAI(COleVariant* pArg, int nArg)
{
    ASSERT_Aint;
    SAFEARRAY* pArray = *pArg[0].pparray;
    LPCLONG pData = reinterpret_cast<LPCLONG>(pArray->pvData);
    long mx = pData[0];

    for (int i = pArray->rgsabound->cElements; --i >= 1;) mx = max(mx, pData[i]);

    return mx;
}

COleVariant TCalcData::MinAF(COleVariant* pArg, int nArg)
{
    ASSERT_Adbl;
    SAFEARRAY* pArray = *pArg[0].pparray;
    const double* pData = reinterpret_cast<const double*>(pArray->pvData);
    double mn = pData[0];

    for (int i = pArray->rgsabound->cElements; --i >= 1;) mn = min(mn, pData[i]);

    return mn;
}

COleVariant TCalcData::MinAI(COleVariant* pArg, int nArg)
{
    ASSERT_Aint;
    SAFEARRAY* pArray = *pArg[0].pparray;
    LPCLONG pData = reinterpret_cast<LPCLONG>(pArray->pvData);
    long mn = pData[0];

    for (int i = pArray->rgsabound->cElements; --i >= 1;) mn = min(mn, pData[i]);

    return mn;
}

COleVariant TCalcData::PrintAB(COleVariant* pArg, int nArg)
{
    ASSERT_Abool;
    SAFEARRAY* pArray = *pArg[0].pparray;
    LPCBYTE pData = reinterpret_cast<LPCBYTE>(pArray->pvData);
    TStringBuffer buffer;

    for (ULONG i = 0; i < pArray->rgsabound->cElements; ++i) buffer.AppendInt(pData[i]);

    ProtectedPrint(buffer);
    return long(pArray->rgsabound->cElements);
}

COleVariant TCalcData::PrintAI(COleVariant* pArg, int nArg)
{
    ASSERT_Aint;
    SAFEARRAY* pArray = *pArg[0].pparray;
    LPCLONG pData = reinterpret_cast<LPCLONG>(pArray->pvData);
    TStringBuffer buffer;

    for (ULONG i = 0; i < pArray->rgsabound->cElements; ++i)
    {
        if (i > 0) buffer += chrSpace;
        buffer.AppendInt(pData[i]);
    }

    ProtectedPrint(buffer);
    return long(pArray->rgsabound->cElements);
}

COleVariant TCalcData::PrintAF(COleVariant* pArg, int nArg)
{
    ASSERT_Adbl;
    SAFEARRAY* pArray = *pArg[0].pparray;
    const double* pData = reinterpret_cast<const double*>(pArray->pvData);
    TStringBuffer buffer;

    for (ULONG i = 0; i < pArray->rgsabound->cElements; ++i)
    {
        if (i > 0) buffer += chrSpace;
        buffer.AppendDouble(pData[i]);
    }

    ProtectedPrint(buffer);
    return long(pArray->rgsabound->cElements);
}

COleVariant TCalcData::PrintAS(COleVariant* pArg, int nArg)
{
    ASSERT_Astr;
    SAFEARRAY* pArray = *pArg[0].pparray;
    const BSTR* pData = reinterpret_cast<const BSTR*>(pArray->pvData);
    TStringBuffer buffer;

    for (ULONG i = 0; i < pArray->rgsabound->cElements; ++i)
    {
        if (i > 0) buffer += chrSpace;
        buffer.Append(pData[i]);
    }

    ProtectedPrint(buffer);
    return long(pArray->rgsabound->cElements);
}
