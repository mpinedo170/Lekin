#pragma once

#include "AFLibLexAnalyzer.h"

/////////////////////////////////////////////////////////////////////////////
// TLexAnalyzerCExpr class
// Lexical analyzer of a C expression

namespace AFLibCalc {  // C-expression lexems (operators, brackets, etc.)
enum
{
    lexArrow = 128,
    lexPlusPlus,
    lexMinusMinus,
    lexLessLess,
    lexMoreMore,
    lexLessEqual,
    lexMoreEqual,
    lexNotEqual,
    lexEqualEqual,
    lexAndAnd,
    lexOrOr,
    lexPlusEqual,
    lexMinusEqual,
    lexStarEqual,
    lexSlashEqual,
    lexPercentEqual,
    lexLessLessEqual,
    lexMoreMoreEqual,
    lexAndEqual,
    lexOrEqual,
    lexXorEqual,
    lexLastCExpr
};

class TLexAnalyzerCExpr : public TLexAnalyzer
{
private:
    typedef TLexAnalyzer super;
    DEFINE_COPY_AND_ASSIGN(TLexAnalyzerCExpr);

public:
    TLexAnalyzerCExpr();
};
}  // namespace AFLibCalc
