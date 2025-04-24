#include "StdAfx.h"

#include "AFLibLexAnalyzerCCode.h"

#include "AFLibLexParser.h"
#include "AFLibThreadSafeConst.h"

using AFLibCalc::TLexAnalyzerCCode;

TLexAnalyzerCCode::TLexAnalyzerCCode()
{
    SAFE_CONST_STR(strSym, _T("{};"));

    for (int i = 0; i < strSym.GetLength(); ++i) AddLexem(CString(strSym[i], 1), int(strSym[i]));

    AddKeyword(_T("void"), lexKeyVoid);
    AddKeyword(_T("bool"), lexKeyBool);
    AddKeyword(_T("int"), lexKeyInt);
    AddKeyword(_T("float"), lexKeyFloat);
    AddKeyword(_T("string"), lexKeyString);
    AddKeyword(_T("return"), lexKeyReturn);
    AddKeyword(_T("if"), lexKeyIf);
    AddKeyword(_T("else"), lexKeyElse);
    AddKeyword(_T("for"), lexKeyFor);
    AddKeyword(_T("while"), lexKeyWhile);
    AddKeyword(_T("do"), lexKeyDo);
    AddKeyword(_T("switch"), lexKeySwitch);
    AddKeyword(_T("case"), lexKeyCase);
    AddKeyword(_T("default"), lexKeyDefault);
    AddKeyword(_T("break"), lexKeyBreak);
    AddKeyword(_T("continue"), lexKeyContinue);
}

bool TLexAnalyzerCCode::SkipComments()
{
    static const LPCTSTR arrComm[] = {_T("//"), _T("/*"), NULL};

    switch (m_pSp->TestSymbolArray(arrComm))
    {
        case 0:
            if (m_pSp->GetLineNum() == m_iLine) m_pSp->SkipToEol();
            return true;

        case 1:
            m_pSp->SkipTo(_T("*/"));
            return true;
    }

    return false;
}
