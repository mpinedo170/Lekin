#include "StdAfx.h"

#include "AFLibCompiler.h"

#include "AFLibCalcData.h"
#include "AFLibCalculator.h"
#include "AFLibExcBreak.h"
#include "AFLibFuncManager.h"
#include "AFLibGlobal.h"
#include "AFLibIDManager.h"
#include "AFLibLexAnalyzerCCode.h"
#include "AFLibStringParserAbstract.h"
#include "AFLibSuperText.h"
#include "Local.h"

using namespace AFLib;
using namespace AFLibIO;
using namespace AFLibCalc;
using namespace AFLibPrivate;
using AFLibGui::TSuperText;
using AFLibThread::CExcBreak;

namespace {
const COleVariant varEmpty;
}

TSimpleType TCompiler::CompileIDNum(TCalculator& calc, const TCompilerState& state) const
{
    switch (state.m_pLex->m_IDLex)
    {
        case lexInt:
            calc.AddPushConst(long(state.m_pLex->m_i));
            state.m_pLex->Next();
            return stpI;

        case lexDouble:
            calc.AddPushConst(state.m_pLex->m_d);
            state.m_pLex->Next();
            return stpF;

        case lexString:
            calc.AddPushConst(state.m_pLex->GetStr());
            state.m_pLex->Next();
            return stpS;

        case lexID:
        {
            CString id = state.m_pLex->GetStr();
            const TIDManager::TIDDef* pIddef1 = state.m_pIDMan->GetVarDef(id);

            state.m_pLex->Next();

            // check if this is a variable
            if (pIddef1 != NULL)
            {
                calc.AddPushVar(pIddef1->m_ref, pIddef1->m_bGlobal ? TCalculator::pfGlobal : 0);
                return pIddef1->m_stp;
            }

            // check if this is a constant
            COleVariant var;
            if (GetConstant(id, var))
            {
                calc.AddPushConst(var);
                return TSimpleType::FromVariant(var);
            }

            TFuncPrototype fprArg;

            // check if this is a local function
            const TFuncManager::TIDDef* pIddef2 = state.m_pFuMan->GetFuncDef(id);

            // read function arguments, if any;
            // convert arguments to right types if local function
            if (state.m_pLex->m_IDLex == '(')
            {
                state.m_pLex->Next();

                while (state.m_pLex->m_IDLex != ')')
                {
                    if (!fprArg.IsNoArg()) state.m_pLex->Test(',');
                    TSimpleType stpAct = CompileExpr(calc, state, GetCommaPriority());

                    if (pIddef2 != NULL && fprArg.GetArgCount() < pIddef2->m_fpr.GetArgCount())
                    {
                        const TSimpleType& stpProt = pIddef2->m_fpr.GetArg(fprArg.GetArgCount());

                        // if prototype is a simple type,
                        //   make sure argument is not a reference
                        if (stpProt.IsSimple() && !stpAct.IsArray())
                        {
                            const COleVariant& v = stpProt.GetZeroConst();
                            calc.AddConvert(v.vt);
                        }
                    }

                    fprArg.AddArg(stpAct);
                }

                state.m_pLex->Next();
            }

            // local function -- check arguments and add the call
            if (pIddef2 != NULL)
            {
                if (!pIddef2->m_fpr.Accepts(fprArg))
                    ThrowMessage(LocalAfxString(IDS_WRONG_ARG, id, fprArg.ArgToString()));
                calc.AddGosub(pIddef2->m_pos);
                return pIddef2->m_fpr.GetReturnType();
            }

            // finally, global (standard) function:
            //   no need to convert references since I write the code
            TFunc func = {0};
            if (!GetFunction(id, func)) ThrowMessage(LocalAfxString(IDS_UNDEFINED_ID, id));

            const TPrototype* pProt = MatchArgs(func, fprArg);
            if (pProt == NULL) ThrowMessage(LocalAfxString(IDS_WRONG_ARG, id, fprArg.ArgToString()));

            if (pProt->m_f != NULL)
            {
                int nArg = fprArg.GetArgCount();

                if (pProt->m_fpr.IsPushSP())
                {
                    calc.AddPushConst(varEmpty);
                    ++nArg;
                }
                calc.AddCall(pProt->m_f, nArg);
            }
            else
            {
                ASSERT(!fprArg.IsNoArg());
                if (fprArg.GetArgCount() > 1) calc.AddPop(fprArg.GetArgCount() - 1);
            }

            return pProt->m_fpr.GetReturnType();
        }

        case lexEOF:
            ThrowMessage(LocalAfxString(IDS_LEX_EOF));
    }

    ThrowMessage(LocalAfxString(IDS_UNEXPECTED_LEXEM, state.m_pLex->GetLexemText(state.m_pLex->m_IDLex)));
    return stpV;
}

TSimpleType TCompiler::CompilePrefix(
    TCalculator& calc, const TCompilerState& state, int oper, const TFunc& op) const
{  // if a number follows a minus, treat minus as a part of the number
    if (oper == '-' && state.m_pLex->IsNumber())
    {
        state.m_pLex->m_d = -state.m_pLex->m_d;
        state.m_pLex->m_i = -state.m_pLex->m_i;
        return CompileTerm(calc, state, op.m_priority);
    }

    TFuncPrototype fprArg;
    TSimpleType sptArg = CompileTerm(calc, state, op.m_priority);
    fprArg.AddArg(sptArg);
    const TPrototype* pProt = MatchArgs(op, fprArg);

    if (pProt == NULL)
        ThrowMessage(
            LocalAfxString(IDS_BAD_PREFIX_OPER, state.m_pLex->GetLexemText(oper), fprArg.ArgToString()));

    switch (oper)
    {
        case lexPlusPlus:
        case lexMinusMinus:
            calc.AddPushConst(1L);
            calc.AddCall(pProt->m_f, 2);
            break;

        case lexKeyInt:
            // special processing for some type convertions
            //   int(b)
            if (sptArg.IsBool())
            {
                calc.AddConvert(VT_I4);
                break;
            }
            goto DEFAULT;

        case lexKeyFloat:
            //   float(b)
            if (sptArg.IsBool())
            {
                calc.AddConvert(VT_R8);
                break;
            }
            goto DEFAULT;

        case lexKeyString:
            //   string(i)
            if (sptArg.IsInt())
            {
                calc.AddConvert(VT_BSTR);
                break;
            }
            // otherwise fall-through to default

        default:
        DEFAULT:
            if (pProt->m_f != NULL) calc.AddCall(pProt->m_f, 1);
    }

    return pProt->m_fpr.GetReturnType();
}

TSimpleType TCompiler::CompilePostfix(TCalculator& calc,
    const TCompilerState& state,
    int oper,
    const TFunc& op,
    const TSimpleType& stpFirstArg) const
{
    TFuncPrototype fprArg;
    fprArg.AddArg(stpFirstArg);

    switch (oper)
    {
        case '?':
        {
            if (!stpFirstArg.IsBool()) ThrowMessage(LocalAfxString(IDS_BAD_CONDITION));

            int pos1 = calc.GetPos();
            calc.AddCondGoto(
                0, TCalculator::jfJumpIfZero | TCalculator::jfPopIfJump | TCalculator::jfPopIfNot);
            TSimpleType stp1 = CompileTerm(calc, state, GetMaxPriority());

            int pos2 = calc.GetPos();
            calc.AddGoto(0);
            calc.SetLabel(pos1);
            state.m_pLex->Test(':');

            TSimpleType stp2 = CompileTerm(calc, state, op.m_priority);
            TSimpleType stpResult;

            // if both are references (arrays), result is a reference (array)
            if (stp1 == stp2)
                stpResult = stp1;
            else if (!stp1.IsArray() && !stp2.IsArray())
            {
                if (stp1.GetET() == stp2.GetET())
                    stpResult = TSimpleType(stp1.GetET(), tmdSimple);
                else if (stp1.IsFType() && stp2.IsFType())
                    stpResult = stpF;
            }

            // if no "common ground" was found, throw an exception
            if (stpResult.IsVoid())
                ThrowMessage(LocalAfxString(IDS_QUEST_MISMATCH, stp1.ToString(), stp2.ToString()));

            calc.SetLabel(pos2);
            return stpResult;
        }

        case lexAndAnd:
        case lexOrOr:
        {
            if (!stpFirstArg.IsBool())
                ThrowMessage(LocalAfxString(IDS_BAD_POSTFIX_OPER, state.m_pLex->GetLexemText(oper)));

            CIntArray arrPos;
            bool bDone = false;
            BYTE defVal = oper == lexAndAnd ? 0 : 1;
            int flags = TCalculator::jfPopIfJump | TCalculator::jfPopIfNot;

            if (oper == lexAndAnd) flags |= TCalculator::jfJumpIfZero;

            while (true)
            {
                arrPos.Add(calc.GetPos());
                calc.AddCondGoto(0, flags);

                if (bDone) break;
                TSimpleType stp = CompileTerm(calc, state, op.m_priority - 1);

                if (!stp.IsBool())
                    ThrowMessage(LocalAfxString(IDS_BAD_POSTFIX_OPER, state.m_pLex->GetLexemText(oper)));

                if (state.m_pLex->m_IDLex == oper)
                    state.m_pLex->Next();
                else
                    bDone = true;
            }

            calc.AddPushConst(BYTE(1 - defVal));
            int pos = calc.GetPos();
            calc.AddGoto(0);

            for (int i = 0; i < arrPos.GetSize(); ++i) calc.SetLabel(arrPos[i]);

            calc.AddPushConst(defVal);
            calc.SetLabel(pos);
            return stpB;
        }

        case '[':
        {
            fprArg.AddArg(CompileTerm(calc, state, GetMaxPriority()));
            state.m_pLex->Test(']');
            goto Right;
        }

        case ',':
            calc.AddPop();
            return CompileTerm(calc, state, op.m_priority);
    }

    switch (op.m_oo)
    {
        case ooSingle:
        {
            const TPrototype* pProt = MatchArgs(op, fprArg);
            if (pProt == NULL)
                ThrowMessage(LocalAfxString(
                    IDS_BAD_POSTFIX_OPER, state.m_pLex->GetLexemText(oper), fprArg.ArgToString()));

            calc.AddCall(pProt->m_f, 1);
            return pProt->m_fpr.GetReturnType();
        }

        case ooLeft:
            fprArg.AddArg(CompileTerm(calc, state, op.m_priority - 1));
            goto Right;

        case ooRight:
            fprArg.AddArg(CompileTerm(calc, state, op.m_priority));
        Right:
        {
            const TPrototype* pProt = MatchArgs(op, fprArg);

            if (pProt == NULL)
                ThrowMessage(LocalAfxString(
                    IDS_BAD_POSTFIX_OPER, state.m_pLex->GetLexemText(oper), fprArg.ArgToString()));

            if (pProt->m_f == NULL)
                ThrowMessage(LocalAfxString(IDS_OPER_NOT_IMPLEMENTED, state.m_pLex->GetLexemText(oper)));

            calc.AddCall(pProt->m_f, 2);
            return pProt->m_fpr.GetReturnType();
        }

        case ooChain:
        {
            while (true)
            {
                fprArg.AddArg(CompileTerm(calc, state, op.m_priority - 1));
                if (state.m_pLex->m_IDLex != oper) break;
                state.m_pLex->Next();
            }

            const TPrototype* pProt = MatchArgs(op, fprArg);
            if (pProt == NULL)
                ThrowMessage(LocalAfxString(
                    IDS_BAD_POSTFIX_OPER, state.m_pLex->GetLexemText(oper), fprArg.ArgToString()));

            calc.AddCall(pProt->m_f, fprArg.GetArgCount());
            return pProt->m_fpr.GetReturnType();
        }
    }

    ASSERT(false);
    return stpV;
}

TSimpleType TCompiler::CompileTerm(TCalculator& calc, const TCompilerState& state, int priority) const
{
    int oper = state.m_pLex->m_IDLex;
    TFunc op;

    // check for prefix operators
    if (GetPrefixOper(oper, op) && op.m_priority == priority)
    {
        state.m_pLex->Next();
        return CompilePrefix(calc, state, oper, op);
    }

    // if no prefix operator with matching priority, compile as the expression...
    if (priority > 0) return CompileExpr(calc, state, priority);

    // ...unless the priority is zero.
    // Parenthesis and identifiers have priority of 0.
    // If not parenthesis, must be ID or number or something like it.

    if (oper != '(') return CompileIDNum(calc, state);

    // Parenthesis: compile as the expression, starting with max priority.
    state.m_pLex->Next();
    TSimpleType stp = CompileExpr(calc, state, GetMaxPriority());
    state.m_pLex->Test(')');
    return stp;
}

TSimpleType TCompiler::CompileExpr(TCalculator& calc, const TCompilerState& state, int priority) const
{
    TSimpleType stp = CompileTerm(calc, state, priority - 1);

    while (true)
    {
        int oper = state.m_pLex->m_IDLex;
        TFunc op;

        if (!GetPostfixOper(oper, op)) break;
        if (op.m_priority > priority) break;

        state.m_pLex->Next();
        stp = CompilePostfix(calc, state, oper, op, stp);
    }

    return stp;
}

void TCompiler::CompileBlock(TCalculator& calc, const TCompilerState& state) const
{
    state.m_pLex->Test('{');
    while (state.m_pLex->m_IDLex != '}') CompileStatement(calc, state);
    state.m_pLex->Next();
}

void TCompiler::CompileVarDef(TCalculator& calc, const TCompilerState& state) const
{
    ASSERT(state.m_pLex->m_IDLex >= lexKeyVoid && state.m_pLex->m_IDLex <= lexKeyString);
    TSimpleType stp(TElementaryType(state.m_pLex->m_IDLex - lexKeyVoid), tmdSimple);

    state.m_pLex->Next();

    while (true)
    {
        CString id = state.m_pLex->GetStr();
        int ref = state.m_pIDMan->GetNRef();
        bool bGlobal = state.m_pIDMan->IsGlobal();

        state.m_pLex->Test(lexID);

        // check if there is a variable, constant, or global function by this name
        COleVariant var;
        TFunc func = {0};
        if (GetConstant(id, var) || GetFunction(id, func) || state.m_pIDMan->GetVarDef(id) != NULL)
            ThrowMessage(LocalAfxString(IDS_DUP_ID, id));

        if (state.m_pIDMan->IsTopLevel() && state.m_pLex->m_IDLex == '(')
        {
            CompileFuncDef(calc, state, id, stp);
            return;
        }

        // this is a variable! if there is a function by the same name, quit
        if (state.m_pFuMan->GetFuncDef(id) != NULL) ThrowMessage(LocalAfxString(IDS_DUP_ID, id));

        if (stp.IsVoid()) ThrowMessage(LocalAfxString(IDS_VOID_VAR));

        calc.AddPushVar(ref, TCalculator::pfRefToVar | (bGlobal ? TCalculator::pfGlobal : 0));

        if (state.m_pLex->m_IDLex == '[')
        {
            state.m_pLex->Next();

            bool bDimPresent = state.m_pLex->m_IDLex != ']';
            int nArg = 2;

            if (bDimPresent)
            {
                TSimpleType stpNelem = CompileExpr(calc, state, GetMaxPriority());
                if (!stpNelem.IsInt()) ThrowMessage(LocalAfxString(IDS_ARRAY_DIM_NOT_INT));
                state.m_pLex->Test(']');
            }
            else
            {
                state.m_pLex->Next();
                calc.AddPushConst(0L);
            }

            if (state.m_pLex->m_IDLex == '=')
            {
                state.m_pLex->Next();
                state.m_pLex->Test('{');

                while (true)
                {
                    TSimpleType stpInit = CompileExpr(calc, state, GetCommaPriority());

                    if (!stp.Accepts(stpInit)) ThrowMessage(LocalAfxString(IDS_INIT_MISMATCH));

                    // convert the first element to the right type
                    if (stp.GetET() != stpInit.GetET() && nArg == 2) calc.AddConvert(stp.GetZeroConst().vt);

                    ++nArg;
                    if (state.m_pLex->m_IDLex != ',') break;
                    state.m_pLex->Next();
                }

                state.m_pLex->Test('}');
            }
            else
            {
                if (!bDimPresent) ThrowMessage(LocalAfxString(IDS_ARRAY_NOT_INIT));
                calc.AddPushConst(stp.GetZeroConst());
                ++nArg;
            }

            calc.AddCall(&TCalcData::InitializeA, nArg);
            calc.AddPop();

            TSimpleType stp2(stp.GetET(), tmdArray);
            VERIFY(state.m_pIDMan->AddVar(id, stp2) == ref);
        }
        else
        {
            if (state.m_pLex->m_IDLex == '=')
            {
                state.m_pLex->Next();
                TSimpleType stpInit = CompileExpr(calc, state, GetCommaPriority());

                if (!stp.Accepts(stpInit)) ThrowMessage(LocalAfxString(IDS_INIT_MISMATCH));
                if (stp.GetET() != stpInit.GetET()) calc.AddConvert(stp.GetZeroConst().vt);
            }
            else
                calc.AddPushConst(stp.GetZeroConst());

            calc.AddCall(&TCalcData::Initialize, 2);
            calc.AddPop();
            VERIFY(state.m_pIDMan->AddVar(id, stp) == ref);
        }

        state.m_pFuMan->AddLocalVar(ref);
        if (state.m_pLex->m_IDLex != ',') break;
        state.m_pLex->Next();
    }

    state.m_pLex->Test(';');
}

void TCompiler::PrivateDimArray(const TCompilerState& state, TSimpleType& stp) const
{
    if (state.m_pLex->m_IDLex != '[') return;

    if (!stp.IsSimple()) ThrowMessage(LocalAfxString(IDS_BAD_TYPE_MOD));

    stp.SetModifyer(tmdArray);
    state.m_pLex->Next();
    state.m_pLex->Test(']');
}

void TCompiler::CompileFuncDef(
    TCalculator& calc, const TCompilerState& state, LPCTSTR id, const TSimpleType& stpRet) const
{
    TIDManager IDMan(state.m_pIDMan, state.m_pIDMan->GetIC());
    TFuncPrototype fpr(stpRet, strEmpty);

    state.m_pLex->Next();
    IDMan.SetLocal();

    if (state.m_pLex->m_IDLex != ')')
    {
        while (true)
        {
            if (state.m_pLex->m_IDLex < lexKeyVoid || state.m_pLex->m_IDLex > lexKeyString)
                ThrowMessage(LocalAfxString(IDS_TYPE_EXPECTED));

            TSimpleType stp(TElementaryType(state.m_pLex->m_IDLex - lexKeyVoid), tmdSimple);
            if (stp.IsVoid()) ThrowMessage(LocalAfxString(IDS_VOID_VAR));

            state.m_pLex->Next();

            if (state.m_pLex->m_IDLex == '&')
            {
                stp.SetModifyer(tmdRef);
                state.m_pLex->Next();
            }

            PrivateDimArray(state, stp);

            CString idVar;
            if (state.m_pLex->m_IDLex == lexID)
            {
                idVar = state.m_pLex->GetStr();
                state.m_pLex->Next();
            }

            PrivateDimArray(state, stp);
            IDMan.AddVar(idVar, stp);
            fpr.AddArg(stp);

            if (state.m_pLex->m_IDLex == ')') break;
            state.m_pLex->Test(',');
        }
    }

    state.m_pLex->Next();
    if (state.m_pLex->m_IDLex == ';')
    {
        state.m_pFuMan->DeclareFunc(id, fpr);
        return;
    }

    int posJumpOver = calc.GetPos();
    calc.AddGoto(0);

    IDMan.ConvertFuncArgs();
    state.m_pFuMan->StartFunc(id, fpr, calc);

    TCompilerState state2 = state;
    state2.m_pIDMan = &IDMan;
    CompileBlock(calc, state2);

    // add "Return 0" in the end just in case
    calc.AddPushConst(state.m_pFuMan->GetCurFuncDef()->m_fpr.GetReturnType().GetZeroConst());
    calc.AddRet(state.m_pFuMan->GetCurFuncDef()->m_fpr.GetArgCount());

    state.m_pFuMan->EndFunc(calc);
    calc.SetLabel(posJumpOver);
}

void TCompiler::CompileCondition(TCalculator& calc, const TCompilerState& state) const
{
    TSimpleType stp = CompileExpr(calc, state, GetMaxPriority());
    if (!stp.IsBool()) ThrowMessage(LocalAfxString(IDS_BAD_CONDITION));
}

void TCompiler::CompileIf(TCalculator& calc, const TCompilerState& state) const
{
    state.m_pLex->Test(lexKeyIf);
    state.m_pLex->Test('(');
    CompileCondition(calc, state);
    state.m_pLex->Test(')');

    int pos = calc.GetPos();
    calc.AddCondGoto(0, TCalculator::jfJumpIfZero | TCalculator::jfPopIfJump | TCalculator::jfPopIfNot);
    CompileStatement(calc, state);

    if (state.m_pLex->m_IDLex == lexKeyElse)
    {
        int pos2 = calc.GetPos();
        calc.AddGoto(0);
        calc.SetLabel(pos);
        pos = pos2;
        state.m_pLex->Next();
        CompileStatement(calc, state);
    }

    calc.SetLabel(pos);
}

void TCompiler::CompileFor(TCalculator& calc, const TCompilerState& state) const
{
    CString label;
    state.m_pLex->Test(lexKeyFor);

    if (m_bLoopLabels && state.m_pLex->m_IDLex == lexID)
    {
        label = state.m_pLex->GetStr();
        state.m_pLex->Next();
    }

    TIDManager IDMan(state.m_pIDMan, state.m_pIDMan->GetIC(), label);
    TCompilerState state2 = state;

    state2.m_pIDMan = &IDMan;
    state.m_pLex->Test('(');

    // initialization

    switch (state.m_pLex->m_IDLex)
    {
        case lexKeyVoid:
        case lexKeyBool:
        case lexKeyInt:
        case lexKeyFloat:
        case lexKeyString:
            CompileVarDef(calc, state2);
            break;

        case ';':
            state.m_pLex->Next();
            break;

        default:
            CompileExpr(calc, state2, GetMaxPriority());
            calc.AddPop();
            state.m_pLex->Test(';');
            break;
    }

    // condition

    int posReloop = calc.GetPos();

    if (state.m_pLex->m_IDLex != ';')
    {
        CompileCondition(calc, state2);
        IDMan.AddBreak(calc.GetPos());
        calc.AddCondGoto(0, TCalculator::jfJumpIfZero | TCalculator::jfPopIfJump | TCalculator::jfPopIfNot);
    }

    state.m_pLex->Test(';');

    // increment

    TCalculator calcInc;
    if (state.m_pLex->m_IDLex != ')')
    {
        CompileExpr(calcInc, state2, GetMaxPriority());
        calcInc.AddPop();
    }

    state.m_pLex->Test(')');

    // operator
    CompileLoopBody(calc, state2);

    // finalize
    int posContinue = calc.GetPos();
    calc.AddCommands(calcInc);
    calc.AddGoto(posReloop);
    IDMan.FinalizeLoop(calc, calc.GetPos(), posContinue);
}

void TCompiler::CompileWhile(TCalculator& calc, const TCompilerState& state) const
{
    CString label;
    state.m_pLex->Test(lexKeyWhile);

    if (m_bLoopLabels && state.m_pLex->m_IDLex == lexID)
    {
        label = state.m_pLex->GetStr();
        state.m_pLex->Next();
    }

    TIDManager IDMan(state.m_pIDMan, state.m_pIDMan->GetIC(), label);
    TCompilerState state2 = state;

    state2.m_pIDMan = &IDMan;
    state.m_pLex->Test('(');

    // condition
    int posContinue = calc.GetPos();
    CompileCondition(calc, state2);
    IDMan.AddBreak(calc.GetPos());
    calc.AddCondGoto(0, TCalculator::jfJumpIfZero | TCalculator::jfPopIfJump | TCalculator::jfPopIfNot);
    state.m_pLex->Test(')');

    // operator
    CompileLoopBody(calc, state2);

    // finalize
    calc.AddGoto(posContinue);
    IDMan.FinalizeLoop(calc, calc.GetPos(), posContinue);
}

void TCompiler::CompileDo(TCalculator& calc, const TCompilerState& state) const
{
    TIDManager IDMan(state.m_pIDMan, state.m_pIDMan->GetIC(), strEmpty);
    TCompilerState state2 = state;

    state2.m_pIDMan = &IDMan;
    state.m_pLex->Test(lexKeyDo);

    int posStartLoop = calc.GetPos();

    // operator
    CompileLoopBody(calc, state2);

    // condition
    int posContinue = calc.GetPos();

    state.m_pLex->Test(lexKeyWhile);
    state.m_pLex->Test('(');
    CompileCondition(calc, state2);
    state.m_pLex->Test(')');
    state.m_pLex->Test(';');

    calc.AddCondGoto(posStartLoop, TCalculator::jfPopIfJump | TCalculator::jfPopIfNot);

    // finalize
    IDMan.FinalizeLoop(calc, calc.GetPos(), posContinue);
}

void TCompiler::CompileSwitch(TCalculator& calc, const TCompilerState& state) const
{
    CString label;
    state.m_pLex->Test(lexKeySwitch);

    if (m_bLoopLabels && state.m_pLex->m_IDLex == lexID)
    {
        label = state.m_pLex->GetStr();
        state.m_pLex->Next();
    }

    TIDManager IDMan(state.m_pIDMan, state.m_pIDMan->GetIC(), label);
    TCompilerState state2 = state;

    state2.m_pIDMan = &IDMan;
    IDMan.RejectContinue();
    state.m_pLex->Test('(');

    // switching variable
    TSimpleType stpSw = CompileExpr(calc, state2, GetMaxPriority());

    if (stpSw.IsArray() || stpSw.IsBool() || stpSw.GetET() == etFloat)
        ThrowMessage(LocalAfxString(IDS_BAD_SWITCH));

    stpSw.SetModifyer(tmdSimple);
    state.m_pLex->Test(')');

    int nRef = IDMan.GetNRef();
    int posJumpNextCase = -1;
    int posDefault = -1;

    // body of the switch
    state.m_pLex->Test('{');

    while (state.m_pLex->m_IDLex != '}')
    {
        switch (state.m_pLex->m_IDLex)
        {
            case lexKeyCase:
            {
                int posFallThrough = -1;
                state.m_pLex->Next();

                // add Goto for the fallthough; will be filled by the next "real" operator
                if (posJumpNextCase >= 0)
                {
                    posFallThrough = calc.GetPos();
                    calc.AddGoto(0);
                }

                // fill the label in the previous "jump if not equal" command
                if (posJumpNextCase >= 0)
                {
                    calc.SetLabel(posJumpNextCase);
                    posJumpNextCase = -1;
                }

                // compute label expression (does not have to be a constant); verify its type
                TSimpleType stp = CompileExpr(calc, state2, GetMaxPriority());

                if (stp.IsArray()) ThrowMessage(LocalAfxString(IDS_SWITCH_MISMATCH));

                stp.SetModifyer(tmdSimple);
                if (stp != stpSw) ThrowMessage(LocalAfxString(IDS_SWITCH_MISMATCH));

                state.m_pLex->Test(':');

                // compare to stack, do not pop switch-var from stack
                calc.AddCall(stpSw.GetET() == etInt ? &TCalcData::SwEqualI : &TCalcData::SwEqualS, 1);

                // "jump if not equal" to next case label
                posJumpNextCase = calc.GetPos();
                calc.AddCondGoto(
                    0, TCalculator::jfJumpIfZero | TCalculator::jfPopIfJump | TCalculator::jfPopIfNot);

                // found matching value! pop from stack!
                calc.AddPop();

                // fall through to here; prepare for a real operator.
                // if next operator is "case", will do another "fall-through jump"
                if (posFallThrough >= 0) calc.SetLabel(posFallThrough);

                break;
            }

            case lexKeyDefault:
                state.m_pLex->Next();
                state.m_pLex->Test(':');

                if (posDefault >= 0) ThrowMessage(LocalAfxString(IDS_DUPLICATE_DEFAULT));

                // "default" is the first label; jump over it
                if (posJumpNextCase < 0)
                {
                    posJumpNextCase = calc.GetPos();
                    calc.AddGoto(0);
                }

                // remember the position of the "default" label
                posDefault = calc.GetPos();
                break;

            default:
                CompileStatement(calc, state2);

                // make sure no new variables were defined!
                if (IDMan.GetNRef() > nRef) ThrowMessage(LocalAfxString(IDS_VAR_IN_SWITCH));
        }
    }

    state.m_pLex->Next();

    // no labels!
    if (posJumpNextCase < 0) ThrowMessage(LocalAfxString(IDS_SWITCH_NO_LAB));

    // jump over pop
    IDMan.AddBreak(calc.GetPos());
    calc.AddGoto(0);

    // stopped looking for a matching value!
    // jump here from the last value-check and pop from stack!
    calc.SetLabel(posJumpNextCase);
    calc.AddPop();

    // jump to Default, if exists
    if (posDefault >= 0) calc.AddGoto(posDefault);

    // end of the operator: we come here from fall-through and all breaks
    IDMan.FinalizeLoop(calc, calc.GetPos(), 0);
}

void TCompiler::CompileLoopBody(TCalculator& calc, const TCompilerState& state) const
{
    if (state.m_pLex->m_IDLex == '{')
        CompileBlock(calc, state);
    else
        CompileStatement(calc, state);
}

void TCompiler::CompileBreakContinue(TCalculator& calc, const TCompilerState& state) const
{
    bool bContinue = false;
    CString label;

    switch (state.m_pLex->m_IDLex)
    {
        case lexKeyContinue:
            bContinue = true;
        case lexKeyBreak:
            break;
        default:
            ASSERT(false);
    }

    state.m_pLex->Next();
    if (m_bLoopLabels && state.m_pLex->m_IDLex == lexID)
    {
        label = state.m_pLex->GetStr();
        state.m_pLex->Next();
    }

    TIDManager* pIDM = state.m_pIDMan->FindLoopedIDM(label, bContinue);
    int pos = calc.GetPos();

    if (bContinue)
        pIDM->AddContinue(pos);
    else
        pIDM->AddBreak(pos);

    state.m_pLex->Test(';');
    calc.AddGoto(0);
}

void TCompiler::CompileReturn(TCalculator& calc, const TCompilerState& state) const
{
    const TFuncManager::TIDDef* pIDDef = state.m_pFuMan->GetCurFuncDef();
    state.m_pLex->Test(lexKeyReturn);

    if (pIDDef == NULL)
    {
        state.m_pLex->Test(';');
        calc.AddQuit();
        return;
    }

    const TSimpleType& stpRet = pIDDef->m_fpr.GetReturnType();
    if (stpRet.IsVoid())
        calc.AddPushConst(stpRet.GetZeroConst());
    else
    {
        TSimpleType stp = CompileExpr(calc, state, GetMaxPriority());
        if (!stpRet.Accepts(stp)) ThrowMessage(LocalAfxString(IDS_RET_MISMATCH));
    }

    state.m_pLex->Test(';');
    calc.AddRet(pIDDef->m_fpr.GetArgCount());
}

void TCompiler::CompileStatement(TCalculator& calc, const TCompilerState& state) const
{
    switch (state.m_pLex->m_IDLex)
    {
        case ';':
            state.m_pLex->Next();
            break;

        case '{':
        {
            TIDManager IDMan(state.m_pIDMan, state.m_pIDMan->GetIC());
            TCompilerState state2 = state;

            state2.m_pIDMan = &IDMan;
            CompileBlock(calc, state2);
            break;
        }

        case lexKeyVoid:
        case lexKeyBool:
        case lexKeyInt:
        case lexKeyFloat:
        case lexKeyString:
            CompileVarDef(calc, state);
            break;

        case lexKeyIf:
            CompileIf(calc, state);
            break;

        case lexKeyFor:
            CompileFor(calc, state);
            break;

        case lexKeyWhile:
            CompileWhile(calc, state);
            break;

        case lexKeyDo:
            CompileDo(calc, state);
            break;

        case lexKeySwitch:
            CompileSwitch(calc, state);
            break;

        case lexKeyBreak:
        case lexKeyContinue:
            CompileBreakContinue(calc, state);
            break;

        case lexKeyReturn:
            CompileReturn(calc, state);
            break;

        default:
            CompileExpr(calc, state, GetMaxPriority());
            calc.AddPop();
            state.m_pLex->Test(';');
            break;
    }
}

TSimpleType TCompiler::CompileEmbeddedExpr(LPCTSTR buffer, int& pos, TCalculator& calc, bool bTestEof) const
{
    CAutoPtr<TLexAnalyzer> pLex(CreateLexAnalyzer());
    pLex->Start(buffer, pos);

    TFuncManager FuMan(true);
    TIDManager IDMan(NULL, true);
    TCompilerState state = {pLex, &FuMan, &IDMan};

    try
    {
        state.m_pLex->Next();
        TSimpleType stp = CompileExpr(calc, state, GetMaxPriority());
        pos = state.m_pLex->m_pos;
        if (bTestEof) state.m_pLex->Test(lexEOF);
        return stp;
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CExcMessage* pExc)
    {
        PrefixCompileException(pExc, pLex, false);
        throw;
    }
}

void TCompiler::FinalizeCompile(TCalculator& calc, const TCompilerState& state) const
{
    if (!calc.FinalLink(*state.m_pFuMan)) ThrowMessage(LocalAfxString(IDS_UNDEF_FUNC));
}

TSimpleType TCompiler::CompileExpression(LPCTSTR buffer, TCalculator& calc) const
{
    int pos = 0;
    return CompileEmbeddedExpr(buffer, pos, calc, true);
}

TSimpleType TCompiler::CompileExpression(TStringParserAbstract& sp, TCalculator& calc, CString& expr) const
{
    int pos = sp.GetPos();
    int posStart = pos;
    TSimpleType stp = CompileEmbeddedExpr(sp.GetInitPtr(), pos, calc, false);

    expr = CString(sp.GetInitPtr() + posStart, pos - posStart);
    sp.SetPos(pos);
    return stp;
}

void TCompiler::CompileCode(LPCTSTR buffer, TCalculator& calc, TIDManager* pIDMan) const
{
    CAutoPtr<TLexAnalyzer> pLex(CreateLexAnalyzer());
    pLex->Start(buffer, 0);

    TFuncManager FuMan(true);
    TCompilerState state = {pLex, &FuMan, pIDMan};

    try
    {
        state.m_pLex->Next();

        if (pIDMan == NULL)
        {
            TIDManager IDMan(NULL, true);
            state.m_pIDMan = &IDMan;
            while (state.m_pLex->m_IDLex != lexEOF) CompileStatement(calc, state);
        }
        else
        {
            while (state.m_pLex->m_IDLex != lexEOF) CompileStatement(calc, state);
        }

        FinalizeCompile(calc, state);
    }
    catch (CExcBreak*)
    {
        throw;
    }
    catch (CExcMessage* pExc)
    {
        PrefixCompileException(pExc, pLex, true);
        throw;
    }
}

void TCompiler::AppendSuperText(
    LPCTSTR buffer, int pos1, int pos2, int IDLex, LPCTSTR str, TSuperText& superText) const
{
    for (int i = pos1; i < pos2; ++i) superText.Add(buffer[i]);
}

void TCompiler::CreateSuperText(LPCTSTR buffer, TSuperText& superText) const
{
    TStringBuffer str;
    CAutoPtr<TLexAnalyzer> pLex(CreateLexAnalyzer());

    pLex->Start(buffer, 0);
    pLex->SlackyNext();

    // handle leading whitespace
    if (pLex->m_pos > 0) AppendSuperText(buffer, 0, pLex->m_pos, lexEOF, str, superText);

    while (pLex->m_IDLex != lexEOF)
    {
        int pos = pLex->m_pos;
        int IDLex = pLex->m_IDLex;

        str.Empty();
        str += pLex->GetStr();

        pLex->SlackyNext();
        AppendSuperText(buffer, pos, pLex->m_pos, IDLex, str, superText);
    }
}
