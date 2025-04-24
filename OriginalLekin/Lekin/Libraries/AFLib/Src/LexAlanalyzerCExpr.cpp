#include "StdAfx.h"

#include "AFLibLexAnalyzerCExpr.h"

#include "AFLibThreadSafeConst.h"

using AFLibCalc::TLexAnalyzerCExpr;

TLexAnalyzerCExpr::TLexAnalyzerCExpr()
{
    SAFE_CONST_STR(strOper, _T("[]()*+-~!/%<>=&^|?:,"));

    for (int i = 0; i < strOper.GetLength(); ++i) AddLexem(CString(strOper[i], 1), int(strOper[i]));

    AddLexem(_T("->"), lexArrow);
    AddLexem(_T("++"), lexPlusPlus);
    AddLexem(_T("--"), lexMinusMinus);
    AddLexem(_T("<<"), lexLessLess);
    AddLexem(_T(">>"), lexMoreMore);
    AddLexem(_T("<="), lexLessEqual);
    AddLexem(_T(">="), lexMoreEqual);
    AddLexem(_T("!="), lexNotEqual);
    AddLexem(_T("=="), lexEqualEqual);
    AddLexem(_T("&&"), lexAndAnd);
    AddLexem(_T("||"), lexOrOr);
    AddLexem(_T("+="), lexPlusEqual);
    AddLexem(_T("-="), lexMinusEqual);
    AddLexem(_T("*="), lexStarEqual);
    AddLexem(_T("/="), lexSlashEqual);
    AddLexem(_T("%="), lexPercentEqual);
    AddLexem(_T("<<="), lexLessLessEqual);
    AddLexem(_T(">>="), lexMoreMoreEqual);
    AddLexem(_T("&="), lexAndEqual);
    AddLexem(_T("|="), lexOrEqual);
    AddLexem(_T("^="), lexXorEqual);
}
