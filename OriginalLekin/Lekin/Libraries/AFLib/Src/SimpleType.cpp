#include "StdAfx.h"

#include "AFLibSimpleType.h"

#include "AFLibStrings.h"

using namespace AFLib;
using AFLibCalc::TSimpleType;

bool TSimpleType::Accepts(const TSimpleType& stp) const
{  // Void can't be accepted...
    if (stp.IsVoid()) return false;

    // ...but it accepts any elementary type
    if (m_et == stp.m_et || m_et == etVoid)
    {  // if types are completely equal, they match
        if (m_tmd == stp.m_tmd) return true;

        // if "this" is simple, stp may be a reference
        if (IsSimple() && stp.IsReference()) return true;

        // otherwise no match
        return false;
    }

    // simple float accepts int (simple or reference)
    if (IsSimple() && m_et == etFloat && stp.IsFType()) return true;

    // nothing else matches
    return false;
}

namespace {
const CString arrType[] = {_T("void"), _T("bool"), _T("int"), _T("float"), _T("string")};

const CString arrMod[] = {_T(""), _T("&"), _T("[]")};

const COleVariant arrZeroConst[] = {
    COleVariant(0L), COleVariant(BYTE(0)), COleVariant(0L), COleVariant(0.), COleVariant(_T(""))};
}  // namespace

CString TSimpleType::ToString() const
{
    return arrType[m_et] + arrMod[m_tmd];
}

TSimpleType TSimpleType::FromVT(VARTYPE vt)
{
    TElementaryType et = etVoid;
    TTypeModifyer tmd = tmdArray;

    switch (vt & ~(VT_BYREF | VT_ARRAY))
    {
        case VT_UI1:
            et = etBool;
            break;
        case VT_I4:
            et = etInt;
            break;
        case VT_R8:
            et = etFloat;
            break;
        case VT_BSTR:
            et = etString;
            break;
        default:
            ASSERT(false);
    }

    switch (vt & (VT_BYREF | VT_ARRAY))
    {
        case 0:
            tmd = tmdSimple;
            break;
        case VT_BYREF:
            tmd = tmdRef;
            break;
    }

    return TSimpleType(et, tmd);
}

const COleVariant& TSimpleType::GetZeroConst() const
{
    return arrZeroConst[m_et];
}
