#pragma once

#include "AFLibStrings.h"

/////////////////////////////////////////////////////////////////////////////
// Global functions and constants for the expression calculator

namespace AFLibCalc {
class __single_inheritance TCalcData;

// function that can be called by TCalculator::Calculate()
typedef COleVariant (TCalcData::*TCalcFunction)(COleVariant* pArg, int nArg);

// generate a Print() call with 9 parameters (expressions)
AFLIB CString GeneratePrint(UINT ids, AFLIB_NINE_STRINGS);
AFLIB CString GeneratePrint(LPCTSTR format, AFLIB_NINE_STRINGS);
}  // namespace AFLibCalc
