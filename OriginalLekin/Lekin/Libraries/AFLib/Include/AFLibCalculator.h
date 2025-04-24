#pragma once

#include "AFLibCalcGlobal.h"

/////////////////////////////////////////////////////////////////////////////
// TCalculator class
// A "virtual machine" for executing a sequence of commands.
// Commands include stack push/pop, conditional and unconditional goto,
//   gosub and ret, and a call to a C++ function.
// Arithmetic, etc. are implemented via the call.
// The data is supplied via TCalcStack.

namespace AFLibCalc {
class TCalcStack;
class TFuncManager;

class AFLIB TCalculator
{
private:
    // command type: clear stack, push constant,
    //   push variable, pop elements, call function,
    //   unconditional goto, conditional goto,
    //   go to subroutine, return from sub,
    //   stack frame adjustment, type conversion, full stop
    enum TTypeCom
    {
        cClear,
        cPushC,
        cPushV,
        cPop,
        cCall,
        cGoto,
        cCond,
        cGosub,
        cRet,
        cStack,
        cConvert,
        cQuit
    };

    // individual command
    struct TCommand
    {
        COleVariant m_var;  // cPushC: constant to push
                            // cPushV: ref of local var to push
                            // cPop: # of elem to pop
                            // cCall: nArg
                            // cGoto, cCond, cGosub: goto offset
                            // cRet: # of elem (arg) to remove from stack
                            // cStack: # of local var to add to stack
                            // cConvert: VT_ to convert into (converts stack top)

        TTypeCom m_type;  // type of command

        union
        {
            TCalcFunction m_f;  // cCall: function to call
            int m_iArg;         // cPushV and cCond: special flags
        };
    };

    CArray<TCommand> m_arrCom;  // commands
    bool m_bThrowOnNan;         // throw exception if Nan occures (o.w. just returns Nan)
    bool m_bExtraSafe;          // extra safe processing: do NOT push reference to string in cPushC

    // initialize the command array
    void PrivateInit();

    // helper for all Add...() functions
    TCommand& PrivateAddCommand(TTypeCom type, const COleVariant& v);

public:
    // flags for cPushV
    enum
    {
        pfRefToVar = 1,
        pfGlobal = 2
    };

    // flags for conditional goto
    enum
    {
        jfJumpIfZero = 1,
        jfPopIfJump = 2,
        jfPopIfNot = 4
    };

    TCalculator();
    TCalculator(const TCalculator& calc);
    TCalculator& operator=(const TCalculator& calc);

    // set the "throw on Nan" flag
    void SetThrowOnNan(bool bThrowOnNan);

    // set the "extra safe" flag
    void SetExtraSafe(bool bExtraSafe);

    // get the "throw on Nan" flag
    bool GetThrowOnNan() const
    {
        return m_bThrowOnNan;
    }

    // get the "extra safe" flag
    bool GetExtraSafe() const
    {
        return m_bExtraSafe;
    }

    // add a "clear stack" command
    void AddClear();

    // add a "push const" command
    void AddPushConst(const COleVariant& v);

    // add a "push var" command
    void AddPushVar(int ref, int flags);

    // add a "pop n elements" command
    void AddPop(int nElem = 1);

    // add a command to calculate function f on nArg arguments on top of stack
    void AddCall(TCalcFunction f, int nArg);

    // add an "unconditional goto" command
    void AddGoto(int label);

    // add a "conditional goto" command
    void AddCondGoto(int label, int flags);

    // add a "go-sub" command
    void AddGosub(int label);

    // add a "return from subroutine" command
    void AddRet(int nPopArg);

    // add a "stack frame" command
    void AddStack(int nLocalArg);

    // add a "convert type" command
    void AddConvert(int vt);

    // add a "quit" command
    void AddQuit();

    // set label in a "goto" or "gosub" command; also # of local arg for cStack command
    void SetLabel(int pos, int label = -1);

    // set labels for all "gosub" commands
    bool FinalLink(const TFuncManager& FuMan);

    // append a set of commands
    void AddCommands(const TCalculator& calc);

    // clear the command stream
    void Clear();

    // get current position (# of commands)
    int GetPos() const
    {
        return m_arrCom.GetSize();
    }

    // is the calculator empty?
    bool IsEmpty() const
    {
        return GetPos() <= 0;
    }

    // does the calculator contain a call to given function?
    bool ContainsCall(TCalcFunction f) const;

    // execute commands on given stack
    void Calculate(TCalcData& cd, TCalcStack& stk) const;

    // execute commands
    void Calculate(TCalcData& cd) const;

    // execute commands, return the top of the stack
    COleVariant CalculateExpression(TCalcData& cd) const;

    // print commands
    CString ToString(const TCalcData* pCd = NULL) const;
};
}  // namespace AFLibCalc
