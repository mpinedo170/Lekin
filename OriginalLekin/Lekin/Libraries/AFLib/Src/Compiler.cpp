#include "StdAfx.h"

#include "AFLibCompiler.h"

#include "AFLibCalcData.h"
#include "AFLibExcMessage.h"
#include "AFLibGlobal.h"
#include "AFLibLexAnalyzerCCode.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibCalc;
using namespace AFLibPrivate;

const TSimpleType TCompiler::stpV(etVoid, tmdSimple);
const TSimpleType TCompiler::stpB(etBool, tmdSimple);
const TSimpleType TCompiler::stpI(etInt, tmdSimple);
const TSimpleType TCompiler::stpF(etFloat, tmdSimple);
const TSimpleType TCompiler::stpS(etString, tmdSimple);
const TSimpleType TCompiler::stpVRef(etVoid, tmdRef);
const TSimpleType TCompiler::stpBRef(etBool, tmdRef);
const TSimpleType TCompiler::stpIRef(etInt, tmdRef);
const TSimpleType TCompiler::stpFRef(etFloat, tmdRef);
const TSimpleType TCompiler::stpSRef(etString, tmdRef);
const TSimpleType TCompiler::stpVArray(etVoid, tmdArray);
const TSimpleType TCompiler::stpBArray(etBool, tmdArray);
const TSimpleType TCompiler::stpIArray(etInt, tmdArray);
const TSimpleType TCompiler::stpFArray(etFloat, tmdArray);
const TSimpleType TCompiler::stpSArray(etString, tmdArray);

TCompiler::TCompiler()
{
    m_bIgnoreFuncCase = true;
    m_bLoopLabels = true;
    m_maxPriority = m_commaPriority = 0;

    //*** postfix operators (priority 1)

    AddPostfixOper('.', 1, ooLeft);
    AddPostfixOper(lexArrow, 1, ooLeft);

    AddPostfixOper('[', 1, ooLeft);
    AddPostfixOperProc('[', TFuncPrototype(stpBRef, stpBArray, stpI), &TCalcData::SubscriptA);
    AddPostfixOperProc('[', TFuncPrototype(stpIRef, stpIArray, stpI), &TCalcData::SubscriptA);
    AddPostfixOperProc('[', TFuncPrototype(stpFRef, stpFArray, stpI), &TCalcData::SubscriptA);
    AddPostfixOperProc('[', TFuncPrototype(stpSRef, stpSArray, stpI), &TCalcData::SubscriptA);

    AddPostfixOper(lexPlusPlus, 1, ooSingle);
    AddPostfixOperProc(lexPlusPlus, TFuncPrototype(stpI, _T("I")), &TCalcData::IncrementI);

    AddPostfixOper(lexMinusMinus, 1, ooSingle);
    AddPostfixOperProc(lexMinusMinus, TFuncPrototype(stpI, _T("I")), &TCalcData::DecrementI);

    //*** prefix operators (priority 2)

    AddPrefixOper(lexPlusPlus, 2);
    AddPrefixOperProc(lexPlusPlus, TFuncPrototype(stpIRef, _T("I")), &TCalcData::AssignAddI);

    AddPrefixOper(lexMinusMinus, 2);
    AddPrefixOperProc(lexMinusMinus, TFuncPrototype(stpIRef, _T("I")), &TCalcData::AssignSubtractI);

    AddPrefixOper('&', 2);
    AddPrefixOper('*', 2);

    AddPrefixOper('+', 2);
    AddPrefixOperProc('+', TFuncPrototype(stpF, _T("f")), NULL);
    AddPrefixOperProc('+', TFuncPrototype(stpI, _T("i")), NULL);  // last added operator has priority

    AddPrefixOper('-', 2);
    AddPrefixOperProc('-', TFuncPrototype(stpF, _T("f")), &TCalcData::NegateF);
    AddPrefixOperProc('-', TFuncPrototype(stpI, _T("i")), &TCalcData::NegateI);

    AddPrefixOper('~', 2);
    AddPrefixOperProc('~', TFuncPrototype(stpI, _T("i")), &TCalcData::NotI);

    AddPrefixOper('!', 2);
    AddPrefixOperProc('!', TFuncPrototype(stpB, _T("b")), &TCalcData::NotB);

    // type conversion operators

    AddPrefixOper(lexKeyInt, 2);
    AddPrefixOperProc(lexKeyInt, TFuncPrototype(stpI, _T("b")), &TCalcData::IntB);
    AddPrefixOperProc(lexKeyInt, TFuncPrototype(stpI, _T("f")), &TCalcData::IntF);
    AddPrefixOperProc(lexKeyInt, TFuncPrototype(stpI, _T("i")), NULL);
    AddPrefixOperProc(lexKeyInt, TFuncPrototype(stpI, _T("s")), &TCalcData::IntS);

    AddPrefixOper(lexKeyFloat, 2);
    AddPrefixOperProc(lexKeyFloat, TFuncPrototype(stpF, _T("b")), &TCalcData::DoubleB);
    AddPrefixOperProc(lexKeyFloat, TFuncPrototype(stpF, _T("f")), NULL);
    AddPrefixOperProc(lexKeyFloat, TFuncPrototype(stpF, _T("s")), &TCalcData::DoubleS);

    AddPrefixOper(lexKeyString, 2);
    AddPrefixOperProc(lexKeyString, TFuncPrototype(stpS, _T("b")), &TCalcData::StringB);
    AddPrefixOperProc(lexKeyString, TFuncPrototype(stpS, _T("f")), &TCalcData::StringF);
    AddPrefixOperProc(lexKeyString, TFuncPrototype(stpS, _T("i")), &TCalcData::StringI);
    AddPrefixOperProc(lexKeyString, TFuncPrototype(stpS, _T("s")), NULL);

    //*** multiplicative operators (priority 3)

    AddPostfixOper('*', 3, ooChain);
    AddPostfixOperProc('*', TFuncPrototype(stpF, _T("f"), TFuncPrototype::fChain), &TCalcData::MultiplyF);
    AddPostfixOperProc('*', TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::MultiplyI);

    AddPostfixOper('/', 3, ooLeft);
    AddPostfixOperProc('/', TFuncPrototype(stpF, _T("ff")), &TCalcData::DivideF);
    AddPostfixOperProc('/', TFuncPrototype(stpI, _T("ii")), &TCalcData::DivideI);

    AddPostfixOper('%', 3, ooLeft);
    AddPostfixOperProc('%', TFuncPrototype(stpF, _T("ff")), &TCalcData::ModuloF);
    AddPostfixOperProc('%', TFuncPrototype(stpI, _T("ii")), &TCalcData::ModuloI);

    //*** additive operators (priority 4)

    AddPostfixOper('+', 4, ooChain);
    AddPostfixOperProc('+', TFuncPrototype(stpF, _T("f"), TFuncPrototype::fChain), &TCalcData::AddF);
    AddPostfixOperProc('+', TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::AddI);
    AddPostfixOperProc('+', TFuncPrototype(stpS, _T("s"), TFuncPrototype::fChain), &TCalcData::AddS);

    AddPostfixOper('-', 4, ooLeft);
    AddPostfixOperProc('-', TFuncPrototype(stpF, _T("ff")), &TCalcData::SubtractF);
    AddPostfixOperProc('-', TFuncPrototype(stpI, _T("ii")), &TCalcData::SubtractI);

    //*** shift operators (priority 5)

    AddPostfixOper(lexLessLess, 5, ooLeft);
    AddPostfixOperProc(lexLessLess, TFuncPrototype(stpI, _T("ii")), &TCalcData::ShlI);

    AddPostfixOper(lexMoreMore, 5, ooLeft);
    AddPostfixOperProc(lexMoreMore, TFuncPrototype(stpI, _T("ii")), &TCalcData::ShlI);

    //*** more-less operators (priority 6)

    AddPostfixOper('<', 6, ooLeft);
    AddPostfixOperProc('<', TFuncPrototype(stpB, _T("bb")), &TCalcData::Less);
    AddPostfixOperProc('<', TFuncPrototype(stpB, _T("ff")), &TCalcData::Less);
    AddPostfixOperProc('<', TFuncPrototype(stpB, _T("ii")), &TCalcData::Less);
    AddPostfixOperProc('<', TFuncPrototype(stpB, _T("ss")), &TCalcData::Less);

    AddPostfixOper('>', 6, ooLeft);
    AddPostfixOperProc('>', TFuncPrototype(stpB, _T("bb")), &TCalcData::More);
    AddPostfixOperProc('>', TFuncPrototype(stpB, _T("ff")), &TCalcData::More);
    AddPostfixOperProc('>', TFuncPrototype(stpB, _T("ii")), &TCalcData::More);
    AddPostfixOperProc('>', TFuncPrototype(stpB, _T("ss")), &TCalcData::More);

    AddPostfixOper(lexLessEqual, 6, ooLeft);
    AddPostfixOperProc(lexLessEqual, TFuncPrototype(stpB, _T("bb")), &TCalcData::NotMore);
    AddPostfixOperProc(lexLessEqual, TFuncPrototype(stpB, _T("ff")), &TCalcData::NotMore);
    AddPostfixOperProc(lexLessEqual, TFuncPrototype(stpB, _T("ii")), &TCalcData::NotMore);
    AddPostfixOperProc(lexLessEqual, TFuncPrototype(stpB, _T("ss")), &TCalcData::NotMore);

    AddPostfixOper(lexMoreEqual, 6, ooLeft);
    AddPostfixOperProc(lexMoreEqual, TFuncPrototype(stpB, _T("bb")), &TCalcData::NotLess);
    AddPostfixOperProc(lexMoreEqual, TFuncPrototype(stpB, _T("ff")), &TCalcData::NotLess);
    AddPostfixOperProc(lexMoreEqual, TFuncPrototype(stpB, _T("ii")), &TCalcData::NotLess);
    AddPostfixOperProc(lexMoreEqual, TFuncPrototype(stpB, _T("ss")), &TCalcData::NotLess);

    //*** equality operators (priority 7)

    AddPostfixOper(lexEqualEqual, 7, ooLeft);
    AddPostfixOperProc(lexEqualEqual, TFuncPrototype(stpB, _T("bb")), &TCalcData::Equal);
    AddPostfixOperProc(lexEqualEqual, TFuncPrototype(stpB, _T("ff")), &TCalcData::Equal);
    AddPostfixOperProc(lexEqualEqual, TFuncPrototype(stpB, _T("ii")), &TCalcData::Equal);
    AddPostfixOperProc(lexEqualEqual, TFuncPrototype(stpB, _T("ss")), &TCalcData::Equal);

    AddPostfixOper(lexNotEqual, 7, ooLeft);
    AddPostfixOperProc(lexNotEqual, TFuncPrototype(stpB, _T("bb")), &TCalcData::NotEqual);
    AddPostfixOperProc(lexNotEqual, TFuncPrototype(stpB, _T("ff")), &TCalcData::NotEqual);
    AddPostfixOperProc(lexNotEqual, TFuncPrototype(stpB, _T("ii")), &TCalcData::NotEqual);
    AddPostfixOperProc(lexNotEqual, TFuncPrototype(stpB, _T("ss")), &TCalcData::NotEqual);

    //*** logical or bitwise "and" (priority 8)

    AddPostfixOper('&', 8, ooChain);
    AddPostfixOperProc('&', TFuncPrototype(stpB, _T("b"), TFuncPrototype::fChain), &TCalcData::AndB);
    AddPostfixOperProc('&', TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::AndI);

    //*** logical or bitwise "xor" (priority 9)

    AddPostfixOper('^', 9, ooChain);
    AddPostfixOperProc('^', TFuncPrototype(stpB, _T("b"), TFuncPrototype::fChain), &TCalcData::XorB);
    AddPostfixOperProc('^', TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::XorI);

    //*** logical or bitwise "or" (priority 10)

    AddPostfixOper('|', 10, ooChain);
    AddPostfixOperProc('|', TFuncPrototype(stpB, _T("b"), TFuncPrototype::fChain), &TCalcData::OrB);
    AddPostfixOperProc('|', TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::OrI);

    //*** logical exclusive "and" (priority 11)

    AddPostfixOper(lexAndAnd, 11, ooSingle);
    AddPostfixOperProc(lexAndAnd, TFuncPrototype(stpB, _T("b")), NULL);

    //*** logical exclusive "or" (priority 12)

    AddPostfixOper(lexOrOr, 12, ooSingle);
    AddPostfixOperProc(lexOrOr, TFuncPrototype(stpB, _T("b")), NULL);

    //*** conditional operator "?:" (priority 13)

    AddPostfixOper('?', 13, ooSingle);
    AddPostfixOperProc('?', TFuncPrototype(stpB, _T("b")), NULL);

    //*** assignments (priority 14)

    AddPostfixOper('=', 14, ooRight);
    AddPostfixOperProc('=', TFuncPrototype(stpBRef, _T("Bb")), &TCalcData::AssignB);
    AddPostfixOperProc('=', TFuncPrototype(stpFRef, _T("Ff")), &TCalcData::AssignF);
    AddPostfixOperProc('=', TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignI);
    AddPostfixOperProc('=', TFuncPrototype(stpSRef, _T("Ss")), &TCalcData::AssignS);

    AddPostfixOper(lexPlusEqual, 14, ooRight);
    AddPostfixOperProc(lexPlusEqual, TFuncPrototype(stpFRef, _T("Ff")), &TCalcData::AssignAddF);
    AddPostfixOperProc(lexPlusEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignAddI);
    AddPostfixOperProc(lexPlusEqual, TFuncPrototype(stpSRef, _T("Ss")), &TCalcData::AssignAddS);

    AddPostfixOper(lexMinusEqual, 14, ooRight);
    AddPostfixOperProc(lexMinusEqual, TFuncPrototype(stpFRef, _T("Ff")), &TCalcData::AssignSubtractF);
    AddPostfixOperProc(lexMinusEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignSubtractI);

    AddPostfixOper(lexStarEqual, 14, ooRight);
    AddPostfixOperProc(lexStarEqual, TFuncPrototype(stpFRef, _T("Ff")), &TCalcData::AssignMultiplyF);
    AddPostfixOperProc(lexStarEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignMultiplyI);

    AddPostfixOper(lexSlashEqual, 14, ooRight);
    AddPostfixOperProc(lexSlashEqual, TFuncPrototype(stpFRef, _T("Ff")), &TCalcData::AssignDivideF);
    AddPostfixOperProc(lexSlashEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignDivideI);

    AddPostfixOper(lexPercentEqual, 14, ooRight);
    AddPostfixOperProc(lexPercentEqual, TFuncPrototype(stpFRef, _T("Ff")), &TCalcData::AssignModuloF);
    AddPostfixOperProc(lexPercentEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignModuloI);

    AddPostfixOper(lexLessLessEqual, 14, ooRight);
    AddPostfixOperProc(lexLessLessEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignShlI);

    AddPostfixOper(lexMoreMoreEqual, 14, ooRight);
    AddPostfixOperProc(lexMoreMoreEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignShrI);

    AddPostfixOper(lexAndEqual, 14, ooRight);
    AddPostfixOperProc(lexAndEqual, TFuncPrototype(stpBRef, _T("Bb")), &TCalcData::AssignAndB);
    AddPostfixOperProc(lexAndEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignAndI);

    AddPostfixOper(lexOrEqual, 14, ooRight);
    AddPostfixOperProc(lexOrEqual, TFuncPrototype(stpBRef, _T("Bb")), &TCalcData::AssignOrB);
    AddPostfixOperProc(lexOrEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignOrI);

    AddPostfixOper(lexXorEqual, 14, ooRight);
    AddPostfixOperProc(lexXorEqual, TFuncPrototype(stpBRef, _T("Bb")), &TCalcData::AssignXorB);
    AddPostfixOperProc(lexXorEqual, TFuncPrototype(stpIRef, _T("Ii")), &TCalcData::AssignXorI);

    //*** comma operator (priority 15)

    AddPostfixOper(',', 15, ooSingle);
    AddPostfixOperProc(',', TFuncPrototype(stpB, _T("v")), NULL);

    //*** constants

    AddConstant(_T("true"), BYTE(1));
    AddConstant(_T("false"), BYTE(0));
    AddConstant(_T("pi"), M_PI);

    //*** standard functions

    AddFunctionAndProc(_T("abs"), TFuncPrototype(stpF, _T("f")), &TCalcData::AbsF);
    AddFunctionProc(_T("abs"), TFuncPrototype(stpI, _T("i")), &TCalcData::AbsI);

    AddFunctionAndProc(_T("isnan"), TFuncPrototype(stpB, _T("f")), &TCalcData::IsNanF);

    AddFunctionAndProc(_T("floor"), TFuncPrototype(stpF, _T("f")), &TCalcData::FloorF);
    AddFunctionProc(_T("floor"), TFuncPrototype(stpF, _T("fi")), &TCalcData::FloorF);

    AddFunctionAndProc(_T("ceil"), TFuncPrototype(stpF, _T("f")), &TCalcData::CeilF);
    AddFunctionProc(_T("ceil"), TFuncPrototype(stpF, _T("fi")), &TCalcData::CeilF);

    AddFunctionAndProc(_T("round"), TFuncPrototype(stpF, _T("f")), &TCalcData::RoundF);
    AddFunctionProc(_T("round"), TFuncPrototype(stpF, _T("fi")), &TCalcData::RoundF);

    AddFunctionAndProc(_T("sqrt"), TFuncPrototype(stpF, _T("f")), &TCalcData::SqrtF);

    AddFunctionAndProc(_T("exp"), TFuncPrototype(stpF, _T("f")), &TCalcData::ExpF);

    AddFunctionAndProc(_T("log"), TFuncPrototype(stpF, _T("f")), &TCalcData::LogF);
    AddFunctionProc(_T("log"), TFuncPrototype(stpF, _T("ff")), &TCalcData::LogF);

    AddFunctionAndProc(_T("random"), TFuncPrototype(stpF, strEmpty), &TCalcData::RandomF);

    AddFunctionAndProc(_T("length"), TFuncPrototype(stpI, _T("s")), &TCalcData::LengthS);
    AddFunctionProc(_T("length"), TFuncPrototype(stpI, stpVArray), &TCalcData::LengthA);

    AddFunctionAndProc(
        _T("redim"), TFuncPrototype(stpV, stpVArray, stpI, TFuncPrototype::fPushSP), &TCalcData::RedimA);

    AddFunctionAndProc(_T("left"), TFuncPrototype(stpS, _T("si")), &TCalcData::LeftS);

    AddFunctionAndProc(_T("right"), TFuncPrototype(stpS, _T("si")), &TCalcData::RightS);

    AddFunctionAndProc(_T("mid"), TFuncPrototype(stpS, _T("si")), &TCalcData::MidS);
    AddFunctionProc(_T("mid"), TFuncPrototype(stpS, _T("sii")), &TCalcData::MidS);

    AddFunctionAndProc(_T("find"), TFuncPrototype(stpI, _T("ss")), &TCalcData::FindS);
    AddFunctionProc(_T("find"), TFuncPrototype(stpI, _T("ssi")), &TCalcData::FindS);

    AddFunctionAndProc(_T("extract"), TFuncPrototype(stpS, _T("Ss")), &TCalcData::ExtractS);

    AddFunctionAndProc(_T("match"), TFuncPrototype(stpB, _T("ss")), &TCalcData::MatchS);

    AddFunctionAndProc(_T("upper"), TFuncPrototype(stpS, _T("s")), &TCalcData::UpperS);

    AddFunctionAndProc(_T("lower"), TFuncPrototype(stpS, _T("s")), &TCalcData::LowerS);

    AddFunctionAndProc(_T("max"), TFuncPrototype(stpF, _T("f"), TFuncPrototype::fChain), &TCalcData::MaxF);
    AddFunctionProc(_T("max"), TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::MaxI);
    AddFunctionProc(_T("max"), TFuncPrototype(stpF, stpFArray), &TCalcData::MaxAF);
    AddFunctionProc(_T("max"), TFuncPrototype(stpI, stpIArray), &TCalcData::MaxAI);

    AddFunctionAndProc(_T("min"), TFuncPrototype(stpF, _T("f"), TFuncPrototype::fChain), &TCalcData::MinF);
    AddFunctionProc(_T("min"), TFuncPrototype(stpI, _T("i"), TFuncPrototype::fChain), &TCalcData::MinI);
    AddFunctionProc(_T("min"), TFuncPrototype(stpF, stpFArray), &TCalcData::MinAF);
    AddFunctionProc(_T("min"), TFuncPrototype(stpI, stpIArray), &TCalcData::MinAI);

    AddFunctionAndProc(_T("print"), TFuncPrototype(stpI, _T("v"), TFuncPrototype::fChain), &TCalcData::Print);
    AddFunctionProc(_T("print"), TFuncPrototype(stpI, stpBArray), &TCalcData::PrintAB);
    AddFunctionProc(_T("print"), TFuncPrototype(stpI, stpFArray), &TCalcData::PrintAF);
    AddFunctionProc(_T("print"), TFuncPrototype(stpI, stpIArray), &TCalcData::PrintAI);
    AddFunctionProc(_T("print"), TFuncPrototype(stpI, stpSArray), &TCalcData::PrintAS);
}

TCompiler::~TCompiler()
{
    ClearOpers();
    ClearFunctions();
}

TLexAnalyzer* TCompiler::CreateLexAnalyzer() const
{
    return new TLexAnalyzerCCode;
}

void TCompiler::PrivateDestroyMap(TOperMap& map)
{
    POSITION pos = map.GetStartPosition();

    while (pos != NULL)
    {
        int key;
        TFunc func;
        map.GetNextAssoc(pos, key, func);
        PrivateDelFunc(func);
    }

    map.RemoveAll();
}

void TCompiler::PrivateAddOper(TOperMap& map, int IDLex, int priority, TTypeOper oo, bool bThrowOnDup)
{
    if (map.Exists(IDLex))
    {
        if (bThrowOnDup)
        {
            CAutoPtr<TLexAnalyzer> pLex(CreateLexAnalyzer());
            ThrowMessage(LocalAfxString(IDS_DUP_OPER, pLex->GetLexemText(IDLex)));
        }
        PrivateDelOper(map, IDLex, false);
    }

    TFunc op = {priority, oo, NULL};
    map.SetAt(IDLex, op);
    m_maxPriority = max(m_maxPriority, op.m_priority);
    InitCommaPriority();
}

void TCompiler::PrivateDelOper(TOperMap& map, int IDLex, bool bThrowOnNotFound)
{
    TFunc op = {0};

    if (!map.Lookup(IDLex, op))
    {
        if (!bThrowOnNotFound) return;
        CAutoPtr<TLexAnalyzer> pLex(CreateLexAnalyzer());
        ThrowMessage(LocalAfxString(IDS_NO_OPER, pLex->GetLexemText(IDLex)));
    }

    PrivateDelFunc(op);
    map.RemoveKey(IDLex);
    if (op.m_priority < m_maxPriority) return;

    m_maxPriority = 0;
    PrivateComputeMaxPriority(m_mapPreOper);
    PrivateComputeMaxPriority(m_mapPostOper);
    InitCommaPriority();
}

void TCompiler::PrivateAddOperProc(TOperMap& map, int IDLex, const TFuncPrototype& fpr, TCalcFunction f)
{
    TFunc op = {0};

    if (!map.Lookup(IDLex, op))
    {
        CAutoPtr<TLexAnalyzer> pLex(CreateLexAnalyzer());
        ThrowMessage(LocalAfxString(IDS_NO_OPER, pLex->GetLexemText(IDLex)));
    }

    PrivateAddProc(op, fpr, f);
    map.SetAt(IDLex, op);
}

void TCompiler::PrivateAddProc(TFunc& func, const TFuncPrototype& fpr, TCalcFunction f)
{
    TPrototype* pProt = new TPrototype;
    pProt->m_fpr = fpr;
    pProt->m_f = f;

    pProt->m_pNext = func.m_pProt;
    func.m_pProt = pProt;
}

void TCompiler::PrivateDelFunc(TFunc& func)
{
    for (TPrototype* pProt = func.m_pProt; pProt != NULL;)
    {
        TPrototype* pProt2 = pProt->m_pNext;
        delete pProt;
        pProt = pProt2;
    }
    func.m_pProt = NULL;
}

void TCompiler::InitCommaPriority()
{
    TFunc op;
    m_commaPriority = GetPostfixOper(',', op) ? op.m_priority - 1 : GetMaxPriority();
}

void TCompiler::SetFunctionIC(bool bIgnoreFuncCase)
{
    ASSERT(m_mapFunc.GetCount() == 0);
    m_bIgnoreFuncCase = bIgnoreFuncCase;
}

void TCompiler::AddFunction(LPCTSTR sFunc, int funcID, bool bThrowOnDup)
{
    CString sFunc2 = sFunc;
    if (m_bIgnoreFuncCase) sFunc2.MakeLower();

    if (m_mapFunc.Exists(sFunc2))
    {
        if (bThrowOnDup) ThrowMessage(LocalAfxString(IDS_DUP_FUNCTION, sFunc));
        DelFunction(sFunc2);
    }

    TFunc func = {funcID, ooSingle, NULL};
    m_mapFunc.SetAt(sFunc2, func);
}

void TCompiler::AddFunctionProc(LPCTSTR sFunc, const TFuncPrototype& fpr, TCalcFunction f)
{
    CString sFunc2 = sFunc;
    if (m_bIgnoreFuncCase) sFunc2.MakeLower();

    TFunc func = {0};

    if (!m_mapFunc.Lookup(sFunc2, func)) ThrowMessage(LocalAfxString(IDS_NO_FUNCTION, sFunc));

    PrivateAddProc(func, fpr, f);
    m_mapFunc.SetAt(sFunc2, func);
}

void TCompiler::AddFunctionAndProc(LPCTSTR sFunc, const TFuncPrototype& fpr, TCalcFunction f)
{
    AddFunction(sFunc);
    AddFunctionProc(sFunc, fpr, f);
}

void TCompiler::DelFunction(LPCTSTR sFunc, bool bThrowOnNotFound)
{
    CString sFunc2 = sFunc;
    if (m_bIgnoreFuncCase) sFunc2.MakeLower();

    TFunc func = {0};

    if (!m_mapFunc.Lookup(sFunc2, func))
    {
        if (bThrowOnNotFound) ThrowMessage(LocalAfxString(IDS_NO_FUNCTION, sFunc));
        return;
    }

    PrivateDelFunc(func);
    m_mapFunc.RemoveKey(sFunc2);
}

void TCompiler::ClearFunctions()
{
    POSITION pos = m_mapFunc.GetStartPosition();

    while (pos != NULL)
    {
        CString key;
        TFunc func;
        m_mapFunc.GetNextAssoc(pos, key, func);
        PrivateDelFunc(func);
    }

    m_mapFunc.RemoveAll();
}

void TCompiler::AddConstant(LPCTSTR sConst, const COleVariant& value, bool bThrowOnDup)
{
    CString sConst2 = sConst;
    if (m_bIgnoreFuncCase) sConst2.MakeLower();

    if (m_mapConst.Exists(sConst2))
    {
        if (bThrowOnDup) ThrowMessage(LocalAfxString(IDS_DUP_CONSTANT, sConst));
        DelConstant(sConst2);
    }

    m_mapConst.SetAt(sConst2, value);
}

void TCompiler::DelConstant(LPCTSTR sConst, bool bThrowOnNotFound)
{
    CString sConst2 = sConst;
    if (m_bIgnoreFuncCase) sConst2.MakeLower();

    if (!m_mapConst.Exists(sConst2))
    {
        if (bThrowOnNotFound) ThrowMessage(LocalAfxString(IDS_NO_CONSTANT, sConst));
        return;
    }

    m_mapFunc.RemoveKey(sConst2);
}

void TCompiler::ClearConstants()
{
    m_mapConst.RemoveAll();
}

void TCompiler::PrivateComputeMaxPriority(TOperMap& map)
{
    int IDLex = 0;
    TFunc op = {0};

    for (POSITION pos = map.GetStartPosition(); pos != NULL; map.GetNextAssoc(pos, IDLex, op))
        m_maxPriority = max(m_maxPriority, op.m_priority);
}

void TCompiler::ClearOpers()
{
    PrivateDestroyMap(m_mapPreOper);
    PrivateDestroyMap(m_mapPostOper);
}

bool TCompiler::GetPrefixOper(int IDLex, TFunc& op) const
{
    return m_mapPreOper.Lookup(IDLex, op) != 0;
}

bool TCompiler::GetPostfixOper(int IDLex, TFunc& op) const
{
    return m_mapPostOper.Lookup(IDLex, op) != 0;
}

bool TCompiler::GetFunction(LPCTSTR sFunc, TFunc& func) const
{
    bool bOk = false;

    if (m_bIgnoreFuncCase)
    {
        CString sFunc2 = sFunc;
        sFunc2.MakeLower();
        bOk = m_mapFunc.Lookup(sFunc2, func) != 0;
    }
    else
        bOk = m_mapFunc.Lookup(sFunc, func) != 0;

    return bOk;
}

bool TCompiler::GetConstant(LPCTSTR sConst, COleVariant& var) const
{
    bool bOk = false;

    if (m_bIgnoreFuncCase)
    {
        CString sConst2 = sConst;
        sConst2.MakeLower();
        bOk = m_mapConst.Lookup(sConst2, var) != 0;
    }
    else
        bOk = m_mapConst.Lookup(sConst, var) != 0;

    return bOk;
}

const TCompiler::TPrototype* TCompiler::MatchArgs(const TFunc& func, const TFuncPrototype& fprArg) const
{
    for (const TPrototype* pProt = func.m_pProt; pProt != NULL; pProt = pProt->m_pNext)
        if (pProt->m_fpr.Accepts(fprArg)) return pProt;
    return NULL;
}

void TCompiler::PrefixCompileException(CExcMessage* pExc, const TLexAnalyzer* pLex, bool bLineCol) const
{
    pExc->Prefix(bLineCol
                     ? LocalAfxString(IDS_COMPILE_LINE_COL, IntToStr(pLex->m_iLine), IntToStr(pLex->m_iCol))
                     : LocalAfxString(IDS_COMPILE_POSITION, IntToStr(pLex->m_pos)));
}
