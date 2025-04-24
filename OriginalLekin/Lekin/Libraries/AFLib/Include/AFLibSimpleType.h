#pragma once

/////////////////////////////////////////////////////////////////////////////
// TSimpleType class
// Simple type (void, bool, int, float, string) and reference flag

namespace AFLibCalc {  // "original" type -- one of 5; "*" in prototype translates into void
enum TElementaryType
{
    etVoid,
    etBool,
    etInt,
    etFloat,
    etString
};

// type modifyer -- simple type, reference, or array
enum TTypeModifyer
{
    tmdSimple,
    tmdRef,
    tmdArray
};

class AFLIB TSimpleType
{
private:
    TElementaryType m_et;  // elementary type
    TTypeModifyer m_tmd;   // is this a reference or array?

public:
    TSimpleType() : m_et(etVoid), m_tmd(tmdSimple)
    {}

    TSimpleType(TElementaryType et, TTypeModifyer tmd) : m_et(et), m_tmd(tmd)
    {}

    bool operator==(const TSimpleType& stp) const
    {
        return m_et == stp.m_et && m_tmd == stp.m_tmd;
    }

    bool operator!=(const TSimpleType& stp) const
    {
        return !(*this == stp);
    }

    // convert vt type into a type
    static TSimpleType FromVT(VARTYPE vt);

    // get type of the argument
    static TSimpleType FromVariant(const COleVariant& v)
    {
        return FromVT(v.vt);
    }

    // "this" is in function prototype; does it accept stp?
    bool Accepts(const TSimpleType& stp) const;

    // return "int&" and such
    CString ToString() const;

    // get constant (zero) of this type
    const COleVariant& GetZeroConst() const;

    // set type modifyer
    void SetModifyer(TTypeModifyer tmd)
    {
        m_tmd = tmd;
    }

    // is this a simple type?
    bool IsSimple() const
    {
        return m_tmd == tmdSimple;
    }

    // is this a reference?
    bool IsReference() const
    {
        return m_tmd == tmdRef;
    }

    // is this an array?
    bool IsArray() const
    {
        return m_tmd == tmdArray;
    }

    // is this a "void" type
    bool IsVoid() const
    {
        return m_et == etVoid;
    }

    // is this a "boolean" type
    bool IsBool() const
    {
        return !IsArray() && m_et == etBool;
    }

    // is this a "boolean" type
    bool IsInt() const
    {
        return !IsArray() && m_et == etInt;
    }

    // does this type represent a number (f or i)?
    bool IsFType() const
    {
        return !IsArray() && (m_et == etInt || m_et == etFloat);
    }

    // get the elementary type
    TElementaryType GetET() const
    {
        return m_et;
    }
};
}  // namespace AFLibCalc
