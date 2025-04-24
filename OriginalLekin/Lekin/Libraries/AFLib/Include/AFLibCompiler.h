#pragma once

#include "AFLibCalcGlobal.h"
#include "AFLibDefine.h"
#include "AFLibFuncPrototype.h"
#include "AFLibMaps.h"

namespace AFLib {
class CExcMessage;
}

namespace AFLibIO {
class TStringParserAbstract;
}

namespace AFLibGui {
class TSuperText;
}

/////////////////////////////////////////////////////////////////////////////
// TCompiler
// Translates a C-like code into a series of stack-based commands.
// Also has a function for syntax-highlighting.

namespace AFLibCalc {
class TLexAnalyzer;
class TCalculator;
class TIDManager;
class TFuncManager;

class AFLIB TCompiler
{
private:
    DEFINE_COPY_AND_ASSIGN(TCompiler);

    bool m_bIgnoreFuncCase;  // ignore lettercase in functions and constants?
    int m_maxPriority;       // maximum priority
    int m_commaPriority;     // priority of the comma minus 1

    // initialize m_commaPriority
    void InitCommaPriority();

protected:
    // function prototype
    struct TPrototype
    {
        TFuncPrototype m_fpr;  // actual prototype
        TCalcFunction m_f;     // calculator function
        TPrototype* m_pNext;   // next prototype in the chain
    };

    // association types -- left, right, and chained
    enum TTypeOper
    {
        ooSingle,
        ooLeft,
        ooRight,
        ooChain
    };

    // operation or function information
    struct TFunc
    {
        int m_priority;       // opeartion priority (or function ID)
        TTypeOper m_oo;       // association type
        TPrototype* m_pProt;  // function prototype
    };

    // operation map: lexem to TFunc
    typedef AFLib::CSmartMap<int, int, TFunc, const TFunc&> TOperMap;

    // function map: string to TFunc
    typedef AFLib::CSmartMap<CString, LPCTSTR, TFunc, const TFunc&> TFuncMap;

    // constant map: string to COleVariant value
    typedef AFLib::CSmartMap<CString, LPCTSTR, COleVariant, const COleVariant&> TConstMap;

    // state of the compiler
    struct TCompilerState
    {
        TLexAnalyzer* m_pLex;    // lexical analyzer
        TFuncManager* m_pFuMan;  // container of functions
        TIDManager* m_pIDMan;    // container of identifiers
    };

private:
    TOperMap m_mapPostOper;  // map of postfix (normal) operations
    TOperMap m_mapPreOper;   // map of prefix operations
    TFuncMap m_mapFunc;      // map of standard functions
    TConstMap m_mapConst;    // map of constants

    // helper for CompileFuncDef(): check for []
    void PrivateDimArray(const TCompilerState& state, TSimpleType& stp) const;

    // helper for AddPrefixOper() and AddPostfixOper()
    void PrivateAddOper(TOperMap& map, int IDLex, int priority, TTypeOper oo, bool bThrowOnDup);

    // helper for DelPrefixOper() and DelPostfixOper()
    void PrivateDelOper(TOperMap& map, int IDLex, bool bThrowOnNotFound);

    // helper for AddPrefixOperProc() and AddPostfixOperProc()
    void PrivateAddOperProc(TOperMap& map, int IDLex, const TFuncPrototype& fpr, TCalcFunction f);

    // delete all prototypes for the function
    static void PrivateAddProc(TFunc& func, const TFuncPrototype& fpr, TCalcFunction f);

    // delete all prototypes for the function
    static void PrivateDelFunc(TFunc& func);

    // compute max priority after deleting an operation
    void PrivateComputeMaxPriority(TOperMap& map);

    // destroy one of the operator maps
    void PrivateDestroyMap(TOperMap& map);

protected:
    bool m_bLoopLabels;  // use loop labels: for A (;;) for B (;;) { ... break A; }

    // set whether to ignore case in functions and constants
    void SetFunctionIC(bool bIgnoreFuncCase);

    // check if lettercase in functions and constants is ignored
    bool GetFunctionIC() const
    {
        return m_bIgnoreFuncCase;
    }

    // catch and "prefix" an exception during compilation
    void PrefixCompileException(AFLib::CExcMessage* pExc, const TLexAnalyzer* pLex, bool bLineCol) const;

    // add function
    void AddFunction(LPCTSTR sFunc, int funcID = -1, bool bThrowOnDup = true);

    // add function processor
    void AddFunctionProc(LPCTSTR sFunc, const TFuncPrototype& fpr, TCalcFunction f);

    // add function and processor; throw if duplicate
    void AddFunctionAndProc(LPCTSTR sFunc, const TFuncPrototype& fpr, TCalcFunction f);

    // remove function
    void DelFunction(LPCTSTR sFunc, bool bThrowOnNotFound = true);

    // remove all functions
    void ClearFunctions();

    // add prefix operation
    void AddPrefixOper(int IDLex, int priority, bool bThrowOnDup = true)
    {
        PrivateAddOper(m_mapPreOper, IDLex, priority, ooSingle, bThrowOnDup);
    }

    // add prefix operator processor function
    void AddPrefixOperProc(int IDLex, const TFuncPrototype& fpr, TCalcFunction f)
    {
        PrivateAddOperProc(m_mapPreOper, IDLex, fpr, f);
    }

    // remove prefix operation
    void DelPrefixOper(int IDLex, bool bThrowOnNotFound = true)
    {
        PrivateDelOper(m_mapPreOper, IDLex, bThrowOnNotFound);
    }

    // add postfix operation
    void AddPostfixOper(int IDLex, int priority, TTypeOper oo, bool bThrowOnDup = true)
    {
        PrivateAddOper(m_mapPostOper, IDLex, priority, oo, bThrowOnDup);
    }

    // add prefix operator processor function
    void AddPostfixOperProc(int IDLex, const TFuncPrototype& fpr, TCalcFunction f)
    {
        PrivateAddOperProc(m_mapPostOper, IDLex, fpr, f);
    }

    // remove postfix operation
    void DelPostfixOper(int IDLex, bool bThrowOnNotFound = true)
    {
        PrivateDelOper(m_mapPostOper, IDLex, bThrowOnNotFound);
    }

    // remove all operations
    void ClearOpers();

    // add a constant
    void AddConstant(LPCTSTR sConst, const COleVariant& value, bool bThrowOnDup = true);

    // delete a constant
    void DelConstant(LPCTSTR sConst, bool bThrowOnNotFound = true);

    // remove all constants
    void ClearConstants();

    // get max priority
    int GetMaxPriority() const
    {
        return m_maxPriority;
    }

    // get comma priority (for compiling function arguments)
    int GetCommaPriority() const
    {
        return m_commaPriority;
    }

    // convert a lexem into a prefix operation definition
    virtual bool GetPrefixOper(int IDLex, TFunc& op) const;

    // convert a lexem into a postfix operation definition
    virtual bool GetPostfixOper(int IDLex, TFunc& op) const;

    // convert an ID into a function definition
    virtual bool GetFunction(LPCTSTR sFunc, TFunc& func) const;

    // convert an ID into a constant
    virtual bool GetConstant(LPCTSTR sConst, COleVariant& var) const;

    // find the right prototype from the function or operator definition
    virtual const TPrototype* MatchArgs(const TFunc& func, const TFuncPrototype& fprArg) const;

    // *** Magical Compilation functions start here ***

    // compile operation; return resulting type
    virtual TSimpleType CompileIDNum(TCalculator& calc, const TCompilerState& state) const;

    // compile expression, check if return type is boolean
    void CompileCondition(TCalculator& calc, const TCompilerState& state) const;

    // compile a block from { to }
    void CompileBlock(TCalculator& calc, const TCompilerState& state) const;

    // compile a variable definition
    void CompileVarDef(TCalculator& calc, const TCompilerState& state) const;

    // compile a local function definition
    void CompileFuncDef(
        TCalculator& calc, const TCompilerState& state, LPCTSTR id, const TSimpleType& stpRet) const;

    // compile "if"
    void CompileIf(TCalculator& calc, const TCompilerState& state) const;

    // operator -- but do not create an extra ID Manager on "{"
    void CompileLoopBody(TCalculator& calc, const TCompilerState& state) const;

    // compile "for"
    void CompileFor(TCalculator& calc, const TCompilerState& state) const;

    // compile "while"
    void CompileWhile(TCalculator& calc, const TCompilerState& state) const;

    // compile "do-while"
    void CompileDo(TCalculator& calc, const TCompilerState& state) const;

    // compile "switch-case-default"
    void CompileSwitch(TCalculator& calc, const TCompilerState& state) const;

    // compile "break" or "continue"
    void CompileBreakContinue(TCalculator& calc, const TCompilerState& state) const;

    // compile "return"
    void CompileReturn(TCalculator& calc, const TCompilerState& state) const;

    // compile an operand; return resulting type
    virtual TSimpleType CompileTerm(TCalculator& calc, const TCompilerState& state, int priority) const;

    // compile a prefix operation; return resulting type
    virtual TSimpleType CompilePrefix(
        TCalculator& calc, const TCompilerState& state, int oper, const TFunc& op) const;

    // compile operation; return resulting type
    virtual TSimpleType CompilePostfix(TCalculator& calc,
        const TCompilerState& state,
        int oper,
        const TFunc& op,
        const TSimpleType& stpFirstArg) const;

    // compile an expression of given priority; return resulting type
    virtual TSimpleType CompileExpr(TCalculator& calc, const TCompilerState& state, int priority) const;

    // compile any statement: "if", "for", expression, etc.
    virtual void CompileStatement(TCalculator& calc, const TCompilerState& state) const;

    // compile an expression embedded in a string
    virtual TSimpleType CompileEmbeddedExpr(LPCTSTR buffer, int& pos, TCalculator& calc, bool bTestEof) const;

    // finalize code compilation
    virtual void FinalizeCompile(TCalculator& calc, const TCompilerState& state) const;

    // helper for CreateSuperText(): add a lexem to superText
    virtual void AppendSuperText(
        LPCTSTR buffer, int pos1, int pos2, int IDLex, LPCTSTR str, AFLibGui::TSuperText& superText) const;

    // creates the right lexical analyzer for this compiler
    virtual TLexAnalyzer* CreateLexAnalyzer() const;

public:
    // argument types for defining operators and functions
    static const TSimpleType stpV;       // void (any type)
    static const TSimpleType stpB;       // bool
    static const TSimpleType stpI;       // int
    static const TSimpleType stpF;       // float (int also OK)
    static const TSimpleType stpS;       // string
    static const TSimpleType stpVRef;    // void& (any reference)
    static const TSimpleType stpBRef;    // bool&
    static const TSimpleType stpIRef;    // int&
    static const TSimpleType stpFRef;    // float&
    static const TSimpleType stpSRef;    // string&
    static const TSimpleType stpVArray;  // void[] (array of any type)
    static const TSimpleType stpBArray;  // bool[]
    static const TSimpleType stpIArray;  // int[]
    static const TSimpleType stpFArray;  // float[]
    static const TSimpleType stpSArray;  // string[]

    TCompiler();
    virtual ~TCompiler();

    // compile an expression into a set of calculator commands
    TSimpleType CompileExpression(LPCTSTR buffer, TCalculator& calc) const;

    // compile an expression into a set of calculator commands (move pos in TStringParser, fill expr)
    TSimpleType CompileExpression(AFLibIO::TStringParserAbstract& sp, TCalculator& calc, CString& expr) const;

    // compile code into a set of calculator commands
    virtual void CompileCode(LPCTSTR buffer, TCalculator& calc, TIDManager* pIDMan = NULL) const;

    // convert into super-text with formatting
    void CreateSuperText(LPCTSTR buffer, AFLibGui::TSuperText& superText) const;
};
}  // namespace AFLibCalc
