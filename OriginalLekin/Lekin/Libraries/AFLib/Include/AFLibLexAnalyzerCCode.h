#pragma once

#include "AFLibLexAnalyzerCExpr.h"

/////////////////////////////////////////////////////////////////////////////
// TLexAnalyzerCCode class
// Generic lexical analyzer of C code

namespace AFLibCalc {  // C-code lexems (keywords)
enum
{
    lexKeyVoid = lexLastCExpr,
    lexKeyBool,
    lexKeyInt,
    lexKeyFloat,
    lexKeyString,
    lexKeyReturn,
    lexKeyIf,
    lexKeyElse,
    lexKeyFor,
    lexKeyWhile,
    lexKeyDo,
    lexKeySwitch,
    lexKeyCase,
    lexKeyDefault,
    lexKeyBreak,
    lexKeyContinue
};

class TLexAnalyzerCCode : public TLexAnalyzerCExpr
{
private:
    typedef TLexAnalyzerCExpr super;
    DEFINE_COPY_AND_ASSIGN(TLexAnalyzerCCode);

    virtual bool SkipComments();

public:
    TLexAnalyzerCCode();
};
}  // namespace AFLibCalc
