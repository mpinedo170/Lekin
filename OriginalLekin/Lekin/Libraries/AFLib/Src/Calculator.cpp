#include "StdAfx.h"

#include "AFLibCalculator.h"

#include "AFLibCalcData.h"
#include "AFLibCalcStack.h"
#include "AFLibFuncManager.h"
#include "AFLibGlobal.h"
#include "AFLibMathGlobal.h"
#include "AFLibStringBuffer.h"
#include "AFLibThread.h"
#include "AFLibThreadSafeConst.h"
#include "Local.h"

using namespace AFLibCalc;
using namespace AFLib;
using namespace AFLibThread;
using namespace AFLibPrivate;
using AFLibMath::IsNan;

TCalculator::TCalculator()
{
    PrivateInit();
}

TCalculator::TCalculator(const TCalculator& calc)
{
    PrivateInit();
    *this = calc;
}

TCalculator& TCalculator::operator=(const TCalculator& calc)
{
    if (&calc == this) return *this;

    m_arrCom.Copy(calc.m_arrCom);
    m_bThrowOnNan = calc.m_bThrowOnNan;
    return *this;
}

void TCalculator::PrivateInit()
{
    m_arrCom.SetSize(0, 1 << 10);
    m_bThrowOnNan = m_bExtraSafe = false;
}

void TCalculator::SetThrowOnNan(bool bThrowOnNan)
{
    m_bThrowOnNan = bThrowOnNan;
}

void TCalculator::SetExtraSafe(bool bExtraSafe)
{
    m_bExtraSafe = bExtraSafe;
}

void TCalculator::Clear()
{
    m_arrCom.RemoveAll();
}

bool TCalculator::ContainsCall(TCalcFunction f) const
{
    for (int i = 0; i < GetPos(); ++i)
    {
        const TCommand& com = m_arrCom[i];
        if (com.m_type == cCall && com.m_f == f) return true;
    }
    return false;
}

TCalculator::TCommand& TCalculator::PrivateAddCommand(TTypeCom type, const COleVariant& v)
{
    int n = GetPos();
    m_arrCom.SetSize(n + 1);

    TCommand& com = m_arrCom[n];
    memset(&com, 0, sizeof(com));

    com.m_type = type;
    com.m_var = v;
    return com;
}

void TCalculator::AddClear()
{
    PrivateAddCommand(cClear, 0L);
}

void TCalculator::AddPushConst(const COleVariant& v)
{
    PrivateAddCommand(cPushC, v);
}

void TCalculator::AddPushVar(int ref, int flags)
{
    TCommand& com = PrivateAddCommand(cPushV, long(ref));
    com.m_iArg = flags;
}

void TCalculator::AddPop(int nElem)
{
    PrivateAddCommand(cPop, long(nElem));
}

void TCalculator::AddCall(TCalcFunction f, int nArg)
{
    TCommand& com = PrivateAddCommand(cCall, long(nArg));
    com.m_f = f;
}

void TCalculator::AddGoto(int label)
{
    PrivateAddCommand(cGoto, long(label - GetPos()));
}

void TCalculator::AddCondGoto(int label, int flags)
{
    TCommand& com = PrivateAddCommand(cCond, long(label - GetPos()));
    com.m_iArg = flags;
}

void TCalculator::AddGosub(int label)
{
    PrivateAddCommand(cGosub, long(label));
}

void TCalculator::AddRet(int nPopArg)
{
    PrivateAddCommand(cRet, long(nPopArg));
}

void TCalculator::AddStack(int nLocalArg)
{
    PrivateAddCommand(cStack, long(nLocalArg));
}

void TCalculator::AddConvert(int vt)
{
    PrivateAddCommand(cConvert, short(vt));
}

void TCalculator::AddQuit()
{
    PrivateAddCommand(cQuit, 0L);
}

void TCalculator::SetLabel(int pos, int label)
{
    TCommand& com = m_arrCom[pos];
    if (label < 0) label = GetPos();

    switch (com.m_type)
    {
        case cGoto:
        case cCond:
            V_I4(&com.m_var) = label - pos;
            break;

        case cGosub:
        case cStack:
            V_I4(&com.m_var) = label;
            break;

        default:
            ASSERT(false);
    }
}

bool TCalculator::FinalLink(const TFuncManager& FuMan)
{
    bool bOk = true;

    for (int i = 0; i < GetPos(); ++i)
    {
        TCommand& com = m_arrCom[i];
        if (com.m_type != cGosub || V_I4(&com.m_var) >= 0) continue;

        int pos = FuMan.GetNewFuncPos(-V_I4(&com.m_var));

        if (pos < 0)
            bOk = false;
        else
            V_I4(&com.m_var) = pos;
    }

    return bOk;
}

void TCalculator::AddCommands(const TCalculator& calc)
{
    m_arrCom.Append(calc.m_arrCom);
}

namespace {
bool EvalCond(const COleVariant& var)
{
    switch (var.vt)
    {
        case VT_UI1:
            return V_UI1(&var) != 0;
        case VT_UI1 | VT_BYREF:
            return *V_UI1REF(&var) != 0;
        case VT_I4:
            return V_I4(&var) != 0;
        case VT_I4 | VT_BYREF:
            return *V_I4REF(&var) != 0;
        case VT_R8:
            return V_R8(&var) != 0;
        case VT_R8 | VT_BYREF:
            return *V_R8REF(&var) != 0;
        case VT_BSTR:
            return ::SysStringLen(V_BSTR(&var)) > 0;
        case VT_BSTR | VT_BYREF:
            return ::SysStringLen(*V_BSTRREF(&var)) > 0;
    }

    ASSERT(false);
    return 0;
}

bool EvalNan(const COleVariant& var)
{
    switch (var.vt)
    {
        case VT_R8:
            return IsNan(V_R8(&var));
        case VT_R8 | VT_BYREF:
            return IsNan(*V_R8REF(&var));
        case VT_UI1:
        case VT_UI1 | VT_BYREF:
        case VT_I4:
        case VT_I4 | VT_BYREF:
        case VT_BSTR:
        case VT_BSTR | VT_BYREF:
            return false;
    }
    return true;
}
}  // namespace

void TCalculator::Calculate(TCalcData& cd, TCalcStack& stk) const
{
    int BP = 1;
    int PC = 0;
    int nSteps = 0;

    while (PC < GetPos())
    {
        if (PC < 0) ThrowMessage(LocalAfxString(IDS_NEGATIVE_PC, IntToStr(PC)));

        if (--nSteps <= 0)
        {  // check for the break condition every 50 steps
            CThread::Break();
            nSteps = 50;
        }

        const TCommand& com = m_arrCom[PC];

        switch (com.m_type)
        {
            case cClear:
                stk.ClearStack();
                break;

            case cPushC:
            case cPushV:
            {
                int n = stk.GetHeight();
                stk.m_arrStk.SetSize(n + 1);
                COleVariant& var = stk.m_arrStk[n];

                if (com.m_type == cPushC)
                {
                    switch (com.m_var.vt)
                    {
                        case VT_EMPTY:
                            // special case: if com.m_var is empty, push SP
                            var.vt = VT_I4;
                            var.lVal = n + 1;
                            break;

                        case VT_BSTR:
                            if (!m_bExtraSafe)
                            {  // For a string, push reference to it.
                                // This is a little unsafe, but built-in functions do not change arguments,
                                //   whereas local functions call cConvert for all by-val parameters.
                                var = TCalcData::CreateRef(*const_cast<COleVariant*>(&com.m_var));
                                break;
                            }

                        default:
                            var = com.m_var;
                            break;
                    }

                    break;
                }

                COleVariant* pVarSrc = NULL;
                int ref = V_I4(&com.m_var);

                if ((com.m_iArg & pfGlobal) != 0)
                {
                    stk.ExtendVar(ref);
                    pVarSrc = &stk.m_arrGlobal[ref];
                }
                else
                    pVarSrc = &stk.m_arrStk[BP + ref];

                var = com.m_iArg & pfRefToVar ? TCalcData::CreateRefToVar(*pVarSrc)
                                              : TCalcData::CreateRef(*pVarSrc);
                break;
            }

            case cPop:
                if (stk.GetHeight() < V_I4(&com.m_var))
                    ThrowMessage(LocalAfxString(IDS_STACK_EMPTY_ON_POP, IntToStr(PC)));
                stk.Pop(V_I4(&com.m_var));
                break;

            case cCond:
            {
                if (stk.IsEmpty()) ThrowMessage(LocalAfxString(IDS_STACK_EMPTY_ON_COND, IntToStr(PC)));

                int flags = com.m_iArg;
                bool bJmp = EvalCond(stk.GetTop());

                if (flags & jfJumpIfZero) bJmp = !bJmp;

                bool bPop = bJmp ? (flags & jfPopIfJump) != 0 : (flags & jfPopIfNot) != 0;

                if (bPop) stk.Pop();
                if (!bJmp) break;
                // now fall through to "goto"
            }

            case cGoto:
                PC += V_I4(&com.m_var);
                continue;

            case cCall:
            {
                int nArg = V_I4(&com.m_var);
                int indexArg = stk.GetHeight() - nArg;

                if (indexArg < 0) ThrowMessage(LocalAfxString(IDS_STACK_EMPTY_ON_CALL, IntToStr(PC)));

                COleVariant result = (cd.*(com.m_f))(stk.m_arrStk.GetData() + indexArg, nArg);

                if (m_bThrowOnNan && EvalNan(result))
                    ThrowMessage(LocalAfxString(IDS_STACK_NAN, IntToStr(PC)));

                stk.m_arrStk.SetSize(indexArg + 1);
                stk.m_arrStk[indexArg] = result;
                break;
            }

            case cGosub:
                stk.m_arrStk.Add(MAKEUINT64(BP, PC + 1));
                PC = V_I4(&com.m_var);
                continue;

            case cRet:
            {  // stack before command: bottom ... arg1 arg2 ... argN retPC/BP locVar1 ... locVarK retVal
                //                                                             ^-BP
                //        after command: bottom ... retVal
                int nArg = V_I4(&com.m_var);

                if (BP <= nArg) ThrowMessage(LocalAfxString(IDS_STACK_EMPTY_ON_SUB_RET, IntToStr(PC)));

                const COleVariant& varRetPC = stk.m_arrStk[BP - 1];

                if (varRetPC.vt != VT_UI8) ThrowMessage(LocalAfxString(IDS_RET_PC_NOT_INT64, IntToStr(PC)));

                PC = LOW32(V_I8(&varRetPC));
                int newBP = HIGH32(V_I8(&varRetPC));

                stk.m_arrStk.RemoveAt(BP - nArg - 1, nArg + stk.GetHeight() - BP);

                COleVariant& var = stk.GetTop();
                var.ChangeType(var.vt & ~VT_BYREF);
                BP = newBP;
                continue;
            }

            case cStack:
                BP = stk.m_arrStk.GetSize();
                stk.m_arrStk.SetSize(BP + V_I4(&com.m_var));
                break;

            case cConvert:
                if (stk.IsEmpty()) ThrowMessage(LocalAfxString(IDS_STACK_EMPTY_ON_CONVERT, IntToStr(PC)));
                stk.GetTop().ChangeType(V_I2(&com.m_var));
                break;

            case cQuit:
                PC = GetPos();
                continue;
        }

        ++PC;
    }
}

void TCalculator::Calculate(TCalcData& cd) const
{
    TCalcStack stk;
    Calculate(cd, stk);
}

COleVariant TCalculator::CalculateExpression(TCalcData& cd) const
{
    TCalcStack stk;
    Calculate(cd, stk);

    if (stk.IsEmpty()) ThrowMessage(LocalAfxString(IDS_STACK_EMPTY_ON_RET));

    if ((stk.GetTop().vt & VT_BYREF) == 0) return stk.GetTop();

    COleVariant var = stk.GetTop();
    var.ChangeType(var.vt & ~VT_BYREF);
    return var;
}

CString TCalculator::ToString(const TCalcData* pCd) const
{
    SAFE_CONST_STR(strUnknown, LocalAfxString(IDS_UNKNOWN_FUNC));
    TCalcData cdTemp;
    AFLibIO::TStringBuffer buffer;

    int nDig = GetPos() <= 1 ? 1 : int(log10(double(GetPos() - 1))) + 1;

    if (pCd == NULL) pCd = &cdTemp;

    for (int i = 0; i < GetPos(); ++i)
    {
        const TCommand& com = m_arrCom[i];
        CString s;
        s.Format(_T("%*d "), nDig, i);
        buffer += s;

        switch (com.m_type)
        {
            case cClear:
                buffer += LocalAfxString(IDS_CLEAR_STACK);
                break;

            case cPushC:
                s = CString(com.m_var);

                switch (com.m_var.vt)
                {
                    case VT_EMPTY:
                        s = _T("SP");
                        break;
                    case VT_UI1:
                        s += _T("B");
                        break;
                    case VT_I4:
                        s += _T("I");
                        break;
                    case VT_BSTR:
                        s = QuoteString(s);
                        break;
                }

                buffer += LocalAfxString(IDS_PUSH, s);
                break;

            case cPushV:
                s = (com.m_iArg & pfGlobal) != 0 ? _T("G") : _T("L");
                s += _T("[") + IntToStr(V_I4(&com.m_var)) + _T("]");

                if ((com.m_iArg & pfRefToVar) != 0) s += LocalAfxString(IDS_VARIANT);

                buffer += LocalAfxString(IDS_PUSH, s);
                break;

            case cPop:
                buffer += LocalAfxString(IDS_POP, IntToStr(V_I4(&com.m_var)));
                break;

            case cGoto:
                buffer += LocalAfxString(IDS_GOTO, IntToStr(V_I4(&com.m_var) + i));
                break;

            case cCond:
            {
                int flags = com.m_iArg;

                buffer += LocalAfxString(
                    IDS_COND, IntToStr(V_I4(&com.m_var) + i), flags & jfJumpIfZero ? _T("0") : _T("1"));

                if (flags & jfPopIfJump) buffer += LocalAfxString(IDS_POP_IF_JUMP);
                if (flags & jfPopIfNot) buffer += LocalAfxString(IDS_POP_IF_NOT);
                break;
            }

            case cCall:
                s = pCd->FuncToName(com.m_f);
                buffer += LocalAfxString(IDS_CALL, s.IsEmpty() ? strUnknown : s, IntToStr(V_I4(&com.m_var)));
                break;

            case cGosub:
                buffer += LocalAfxString(IDS_GOSUB, IntToStr(V_I4(&com.m_var)));
                break;

            case cRet:
                buffer += LocalAfxString(IDS_RET, IntToStr(V_I4(&com.m_var)));
                break;

            case cStack:
                buffer += LocalAfxString(IDS_STACK, IntToStr(V_I4(&com.m_var)));
                break;

            case cConvert:
            {
                TSimpleType stp = TSimpleType::FromVT(V_I2(&com.m_var));
                buffer += LocalAfxString(IDS_CONVERT, stp.ToString());
                break;
            }

            case cQuit:
                buffer += LocalAfxString(IDS_QUIT);
                break;
        }

        buffer += strEol;
    }

    return LPCTSTR(buffer);
}
