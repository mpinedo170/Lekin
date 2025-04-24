#pragma once

#include "AFLibCalcGlobal.h"
#include "AFLibMaps.h"

namespace AFLib {
class TWildCardMSAccess;
}

/////////////////////////////////////////////////////////////////////////////
// TCalcData
// Local data for calculating expressions.
// Member functions operate with the stack of COleVariant's;
// pArg[nArg - 1] is the last argument and the top of the stack.
// In comments, we use:
//   b - bool or ref to bool;
//   B - ref to bool;
//   i - int or ref to int;
//   I - ref to int;
//   f - double or int or ref to double or ref to int;
//   F - ref to double;
//   s - string or ref to string;
//   S - ref to string.

namespace AFLibCalc {
class AFLIB TCalcData
{
private:
    // map of wild cards -- to avoid repeated creation in MatchS()
    AFLib::CS2PMapW<AFLib::TWildCardMSAccess> m_mapWc;

    // helper for ToUpperS() and ToLowreS()
    static BSTR PrivateToUL(COleVariant* pArg, int nArg, wint_t (*f)(wint_t c));

protected:
    // helper for ASSERT_III and ASSERT_BBB
    void TestManyArg(COleVariant* pArg, int nArg, int type) const;

    // helper for ASSERT_FFF
    void TestFFF(COleVariant* pArg, int nArg) const;

    // helper for ASSERT_ASSIGN
    void TestAssignArg(COleVariant* pArg, int nArg, int type) const;

    // helper for ASSERT_INIT in CalcDataArray
    void TestArrInitArg(COleVariant* pArg, int nArg) const;

    // helper for Less, Equal, etc.
    int Compare(COleVariant* pArg, int nArg) const;

    // helper structure for FuncToName()
    struct TNamerElement
    {
        TCalcFunction m_f;
        CString m_name;
    };

    // helper for FuncToName()
    const CString& ProtectedFuncToName(TCalcFunction f, const TNamerElement* arrNe) const;

    // overridable print
    virtual void ProtectedPrint(LPCTSTR str);

    // arrNe array in ProtectedFuncToName() should end with m_neEnd
    static const TNamerElement m_neEnd;

public:
    TCalcData();
    virtual ~TCalcData();

    //*** Namer function (for TCalculator::ToString) ***
    virtual const CString& FuncToName(TCalcFunction f) const;

    //*** Functions useful for implementation ***

    // get wildcard pattern from map; if not found, create
    const AFLib::TWildCardMSAccess* GetPattern(BSTR s);

    // type of v, sans reference
    static int GetT(const COleVariant& v)
    {
        return v.vt & ~VT_BYREF;
    }

    // get byte value; take reference into account
    static BYTE GetB(const COleVariant& v)
    {
        return v.vt & VT_BYREF ? *V_UI1REF(&v) : V_UI1(&v);
    }

    // get integer value; take reference into account
    static long GetI(const COleVariant& v)
    {
        return v.vt & VT_BYREF ? *V_I4REF(&v) : V_I4(&v);
    }

    // get double value; convert from int; take reference into account
    static double GetF(const COleVariant& v);

    // get string value; take reference into account
    static BSTR GetS(const COleVariant& v)
    {
        return v.vt & VT_BYREF ? *V_BSTRREF(&v) : V_BSTR(&v);
    }

    // convert bool into string ("true" or "false")
    static const CString& B2S(bool b);

    // check if parameter is a number
    static bool IsArgF(COleVariant& var)
    {
        return GetT(var) == VT_I4 || GetT(var) == VT_R8;
    }

    // are arguments compatible? (same type, or both numerical)
    static bool AreArgsCompat(COleVariant& var1, COleVariant& var2);

    // create a reference to a variable, preserving type if possible
    static COleVariant CreateRef(VARIANT& var);

    // create a reference to a variant
    static COleVariant CreateRefToVar(VARIANT& var);

    //*** Universal functions ***

    // 1 if arg1 < arg2 (bb or ff or ss)
    COleVariant Less(COleVariant* pArg, int nArg);

    // 1 if arg1 >= arg2 (bb or ff or ss)
    COleVariant NotLess(COleVariant* pArg, int nArg);

    // 1 if arg1 > arg2 (bb or ff or ss)
    COleVariant More(COleVariant* pArg, int nArg);

    // 1 if arg1 <= arg2 (bb or ff or ss)
    COleVariant NotMore(COleVariant* pArg, int nArg);

    // 1 if arg1 == arg2 (bb or dd or ss)
    COleVariant Equal(COleVariant* pArg, int nArg);

    // 1 if arg1 != arg2 (bb or ff or ss)
    COleVariant NotEqual(COleVariant* pArg, int nArg);

    // print all arguments (any arg)
    COleVariant Print(COleVariant* pArg, int nArg);

    // initial assign: 1st arg is ref to Variant; take type from 2nd argument
    COleVariant Initialize(COleVariant* pArg, int nArg);

    //*** Boolean functions ***

    // true <-> false (b)
    COleVariant NotB(COleVariant* pArg, int nArg);

    // logical "and" (b...)
    COleVariant AndB(COleVariant* pArg, int nArg);

    // logical "or" (b...)
    COleVariant OrB(COleVariant* pArg, int nArg);

    // logical "or" (b...)
    COleVariant XorB(COleVariant* pArg, int nArg);

    // convert into int (b)
    COleVariant IntB(COleVariant* pArg, int nArg);

    // convert into double (b)
    COleVariant DoubleB(COleVariant* pArg, int nArg);

    // convert into string (b)
    COleVariant StringB(COleVariant* pArg, int nArg);

    // pArg[0] := pArg[1] (Bb)
    COleVariant AssignB(COleVariant* pArg, int nArg);

    // perform "and" and assign (Bb)
    COleVariant AssignAndB(COleVariant* pArg, int nArg);

    // perform "or" and assign (Bb)
    COleVariant AssignOrB(COleVariant* pArg, int nArg);

    // perform "xor" and assign (Bb)
    COleVariant AssignXorB(COleVariant* pArg, int nArg);

    //*** Integer functions ***

    // check if pArg[0] equals pArg[-1] (nArg = 1)
    COleVariant SwEqualI(COleVariant* pArg, int nArg);

    // negate the argument (i)
    COleVariant NegateI(COleVariant* pArg, int nArg);

    // inverse all bits (i)
    COleVariant NotI(COleVariant* pArg, int nArg);

    // bitwise "and" (i...)
    COleVariant AndI(COleVariant* pArg, int nArg);

    // bitwise "or" (i...)
    COleVariant OrI(COleVariant* pArg, int nArg);

    // bitwise "or" (i...)
    COleVariant XorI(COleVariant* pArg, int nArg);

    // bitwise shift left (ii)
    COleVariant ShlI(COleVariant* pArg, int nArg);

    // bitwise shift right (ii)
    COleVariant ShrI(COleVariant* pArg, int nArg);

    // add (i...)
    COleVariant AddI(COleVariant* pArg, int nArg);

    // subtract (ii)
    COleVariant SubtractI(COleVariant* pArg, int nArg);

    // multiply (i...)
    COleVariant MultiplyI(COleVariant* pArg, int nArg);

    // integer divide (ii)
    COleVariant DivideI(COleVariant* pArg, int nArg);

    // modulo (ii)
    COleVariant ModuloI(COleVariant* pArg, int nArg);

    // absolute value (i)
    COleVariant AbsI(COleVariant* pArg, int nArg);

    // max of nArg members (i...)
    COleVariant MaxI(COleVariant* pArg, int nArg);

    // min of nArg members (i...)
    COleVariant MinI(COleVariant* pArg, int nArg);

    // convert into string (i or is; optional 2nd argument is format)
    COleVariant StringI(COleVariant* pArg, int nArg);

    // pArg[0] := pArg[1] (Ii)
    COleVariant AssignI(COleVariant* pArg, int nArg);

    // bitwise "and" and assign (Ii)
    COleVariant AssignAndI(COleVariant* pArg, int nArg);

    // bitwise "or" and assign (Ii)
    COleVariant AssignOrI(COleVariant* pArg, int nArg);

    // bitwise "xor" and assign (Ii)
    COleVariant AssignXorI(COleVariant* pArg, int nArg);

    // bitwise shift left and assign (Ii)
    COleVariant AssignShlI(COleVariant* pArg, int nArg);

    // bitwise shift right and assign (Ii)
    COleVariant AssignShrI(COleVariant* pArg, int nArg);

    // add and assign (Ii)
    COleVariant AssignAddI(COleVariant* pArg, int nArg);

    // subtract and assign (Ii)
    COleVariant AssignSubtractI(COleVariant* pArg, int nArg);

    // integer multiply and assign (Ii)
    COleVariant AssignMultiplyI(COleVariant* pArg, int nArg);

    // divide and assign (Ii)
    COleVariant AssignDivideI(COleVariant* pArg, int nArg);

    // modulo and assign (Ii)
    COleVariant AssignModuloI(COleVariant* pArg, int nArg);

    // implements postfix ++ (I)
    COleVariant IncrementI(COleVariant* pArg, int nArg);

    // implements postfix -- (I)
    COleVariant DecrementI(COleVariant* pArg, int nArg);

    //*** Floating functions ***

    // negate the argument (i)
    COleVariant NegateF(COleVariant* pArg, int nArg);

    // add (f...)
    COleVariant AddF(COleVariant* pArg, int nArg);

    // subtract (ff)
    COleVariant SubtractF(COleVariant* pArg, int nArg);

    // multiply (f...)
    COleVariant MultiplyF(COleVariant* pArg, int nArg);

    // divide (ff)
    COleVariant DivideF(COleVariant* pArg, int nArg);

    // modulo (ff)
    COleVariant ModuloF(COleVariant* pArg, int nArg);

    // raise to power (ff)
    COleVariant PowerF(COleVariant* pArg, int nArg);

    // absolute value (f)
    COleVariant AbsF(COleVariant* pArg, int nArg);

    // max of nArg members (f...)
    COleVariant MaxF(COleVariant* pArg, int nArg);

    // min of nArg members (f...)
    COleVariant MinF(COleVariant* pArg, int nArg);

    // 1 if the argument is Nan? (f)
    COleVariant IsNanF(COleVariant* pArg, int nArg);

    // round down to n digits (f or fi)
    COleVariant FloorF(COleVariant* pArg, int nArg);

    // round up to n digits (f or fi)
    COleVariant CeilF(COleVariant* pArg, int nArg);

    // round up or down to n digits (f or fi)
    COleVariant RoundF(COleVariant* pArg, int nArg);

    // square root (f)
    COleVariant SqrtF(COleVariant* pArg, int nArg);

    // antilogarithm (f)
    COleVariant ExpF(COleVariant* pArg, int nArg);

    // logarithm (f or ff)
    COleVariant LogF(COleVariant* pArg, int nArg);

    // uniform (0,1) random number (no arg)
    COleVariant RandomF(COleVariant* pArg, int nArg);

    // convert into int (f)
    COleVariant IntF(COleVariant* pArg, int nArg);

    // convert into string (f)
    COleVariant StringF(COleVariant* pArg, int nArg);

    // *100, convert into string, append % sign (f)
    COleVariant StringPercentF(COleVariant* pArg, int nArg);

    // pArg[0] := pArg[1] (Ff)
    COleVariant AssignF(COleVariant* pArg, int nArg);

    // add and assign (Ff)
    COleVariant AssignAddF(COleVariant* pArg, int nArg);

    // subtract and assign (Ff)
    COleVariant AssignSubtractF(COleVariant* pArg, int nArg);

    // multiply and assign (Ff)
    COleVariant AssignMultiplyF(COleVariant* pArg, int nArg);

    // divide and assign (Ff)
    COleVariant AssignDivideF(COleVariant* pArg, int nArg);

    // modulo and assign (Ff)
    COleVariant AssignModuloF(COleVariant* pArg, int nArg);

    // raise to power and assign (Ff)
    COleVariant AssignPowerF(COleVariant* pArg, int nArg);

    //*** String functions ***

    // check if pArg[0] equals pArg[-1] (nArg = 1)
    COleVariant SwEqualS(COleVariant* pArg, int nArg);

    // concatenate (s...)
    COleVariant AddS(COleVariant* pArg, int nArg);

    // length (s)
    COleVariant LengthS(COleVariant* pArg, int nArg);

    // left substring, 0-based (si)
    COleVariant LeftS(COleVariant* pArg, int nArg);

    // right substring, 0-based (si)
    COleVariant RightS(COleVariant* pArg, int nArg);

    // substring in the middle, 0-based (si or sii)
    COleVariant MidS(COleVariant* pArg, int nArg);

    // find substring (ssi or ss, returns index or -1)
    COleVariant FindS(COleVariant* pArg, int nArg);

    // extract from arg1 up to first occurrrece of arg2 (Ss)
    COleVariant ExtractS(COleVariant* pArg, int nArg);

    // check if string matches a pattern (ss)
    COleVariant MatchS(COleVariant* pArg, int nArg);

    // convert string to uppercase (s)
    COleVariant UpperS(COleVariant* pArg, int nArg);

    // convert string to lowercase (s)
    COleVariant LowerS(COleVariant* pArg, int nArg);

    // convert into int (s)
    COleVariant IntS(COleVariant* pArg, int nArg);

    // convert into double (s)
    COleVariant DoubleS(COleVariant* pArg, int nArg);

    // pArg[0] := pArg[1] (Ss)
    COleVariant AssignS(COleVariant* pArg, int nArg);

    // add and assign (Ss)
    COleVariant AssignAddS(COleVariant* pArg, int nArg);

    //*** Array functions ***

    // init array: array var, #elem, elem1, ... (at least 3 arg)
    COleVariant InitializeA(COleVariant* pArg, int nArg);

    // subscript []
    COleVariant SubscriptA(COleVariant* pArg, int nArg);

    // length of an array
    COleVariant LengthA(COleVariant* pArg, int nArg);

    // set array dimension
    COleVariant RedimA(COleVariant* pArg, int nArg);

    // max of float array
    COleVariant MaxAF(COleVariant* pArg, int nArg);

    // max of int array
    COleVariant MaxAI(COleVariant* pArg, int nArg);

    // min of float array
    COleVariant MinAF(COleVariant* pArg, int nArg);

    // min of int array
    COleVariant MinAI(COleVariant* pArg, int nArg);

    // print all elements of a boolean array
    COleVariant PrintAB(COleVariant* pArg, int nArg);

    // print all elements of an integer array
    COleVariant PrintAI(COleVariant* pArg, int nArg);

    // print all elements of a float array
    COleVariant PrintAF(COleVariant* pArg, int nArg);

    // print all elements of a string array
    COleVariant PrintAS(COleVariant* pArg, int nArg);
};
}  // namespace AFLibCalc
