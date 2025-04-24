/////////////////////////////////////////////////////////////////////////////
// AFLib Main Include File
// No "#pragma once" since it may be included more than once

#if !defined(INCLUDE_AFLIB) && !defined(INCLUDE_AFLIB_INT_SET) && !defined(INCLUDE_AFLIB_GUI) &&    \
    !defined(INCLUDE_AFLIB_IO) && !defined(INCLUDE_AFLIB_MATH) && !defined(INCLUDE_AFLIB_THREAD) && \
    !defined(INCLUDE_AFLIB_DB)
#error Please define one of the INCLUDE_AFLIB constants.
#endif

#ifndef AFLIB
#define AFLIB __declspec(dllimport)
#endif

#if defined(INCLUDE_AFLIB) || defined(INCLUDE_AFLIB_INT_SET)
#undef INCLUDE_AFLIB

#include "AFLib3State.h"
#include "AFLibAppAF.h"
#include "AFLibBitMask.h"
#include "AFLibCollections.h"
#include "AFLibExcMsgBox.h"
#include "AFLibHeap.h"
#include "AFLibMemoryBulk.h"
#include "AFLibTestMaps.h"

#ifdef INCLUDE_AFLIB_INT_SET
#undef INCLUDE_AFLIB_INT_SET
#include "AFLibWildCardMSAccess.h"
#endif

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLib;
#endif
#endif

#if defined(INCLUDE_AFLIB_VARIANT)
#undef INCLUDE_AFLIB_VARIANT
#include "AFLibOleVariantArray.h"
#endif

#ifdef INCLUDE_AFLIB_GUI
#undef INCLUDE_AFLIB_GUI

#include "AFLibBtnImg.h"
#include "AFLibBtnTxt.h"

#include "AFLibCmbFont.h"
#include "AFLibEditCode.h"
#include "AFLibLstMask.h"

#include "AFLibDlgDitemDetails.h"
#include "AFLibDlgSelfDestruct.h"

#include "AFLibCbdDataUnit.h"
#include "AFLibCbiDataUnit.h"
#include "AFLibCbsDataUnit.h"
#include "AFLibChkDataUnit.h"
#include "AFLibDblDataUnit.h"
#include "AFLibDirDataUnit.h"
#include "AFLibMskDataUnit.h"
#include "AFLibQuaDataUnit.h"
#include "AFLibRadDataUnit.h"
#include "AFLibSpnDataUnit.h"
#include "AFLibSpnInfDataUnit.h"
#include "AFLibSpnPrcDataUnit.h"
#include "AFLibStrDataUnit.h"

#include "AFLibFrmAF.h"
#include "AFLibResource.h"
#include "AFLibViewTempEx.h"

#include "AFLibDocAF.h"
#include "AFLibUndo.h"
#include "AFLibUndoOp.h"

#include "AFLibDlgLog.h"
#include "AFLibDlgRun.h"

#include "AFLibDragField.h"
#include "AFLibLstDragCompanion.h"

#include "AFLibAFListDrawData.h"
#include "AFLibBlockMBCounter.h"
#include "AFLibDCStateSaver.h"
#include "AFLibGuiGlobal.h"
#include "AFLibMenuItemInfo.h"
#include "AFLibMetaFileDCAF.h"
#include "AFLibSuperText.h"
#include "AFLibWndAFHeader.h"

#ifdef INCLUDE_AFLIB_GUI_COMMON_CTL
#undef INCLUDE_AFLIB_GUI_COMMON_CTL

#include "AFLibLstDitem.h"
#include "AFLibRichEditCtrlW.h"
#include "AFLibSpinInfinity.h"
#include "AFLibTreeDitem.h"

#include "AFLibDlgPrint.h"
#include "AFLibMargin.h"

#include "AFLibProFont.h"
#include "AFLibProTT.h"

#include "AFLibViewSplitted.h"
#endif

#ifdef INCLUDE_AFLIB_GUI_ATL
#undef INCLUDE_AFLIB_GUI_ATL
#include "AFLibMenuBitmapper.h"
#endif

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLibGui;
#endif
#endif

#ifdef INCLUDE_AFLIB_IO
#undef INCLUDE_AFLIB_IO

#include "AFLibFileKiller.h"
#include "AFLibFileParser.h"
#include "AFLibLog.h"
#include "AFLibStorage.h"
#include "AFLibStringParser.h"
#include "AFLibStringSpacer.h"
#include "AFLibStringTokenizer.h"
#include "AFLibStringTokenizerAlt.h"
#include "AFLibStringTokenizerStr.h"
#include "AFLibStringTokenizerText.h"
#include "AFLibStringWriter.h"
#include "AFLibTextParser.h"

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLibIO;
#endif
#endif

#ifdef INCLUDE_AFLIB_MATH
#undef INCLUDE_AFLIB_MATH

#include "AFLibCartesianPoint.h"
#include "AFLibClusterSet.h"
#include "AFLibMathGlobal.h"
#include "AFLibQuantity.h"
#include "AFLibRandom.h"

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLibMath;
#endif
#endif

#ifdef INCLUDE_AFLIB_DB
#undef INCLUDE_AFLIB_DB

#include "AFLibDBField.h"
#include "AFLibDatabaseAF.h"
#include "AFLibDatabasePool.h"
#include "AFLibRecordsetAF.h"
#include "AFLibRecordsetDef.h"
#include "AFLibRowNC.h"

#ifdef INCLUDE_AFLIB_THREAD
#include "AFLibDatabasePool.h"
#endif

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLibDB;
#endif
#endif

#ifdef INCLUDE_AFLIB_THREAD
#undef INCLUDE_AFLIB_THREAD

#include "AFLibExcBreak.h"
#include "AFLibSemaLock.h"
#include "AFLibSemaphoreAF.h"
#include "AFLibThreadSafeConst.h"
#include "AFLibThreadSafeStatic.h"
#include "AFLibWorc.h"
#include "AFLibWorcer.h"
#include "AFLibWork.h"
#include "AFLibWorker.h"

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLibThread;
#endif
#endif

#ifdef INCLUDE_AFLIB_CALC
#undef INCLUDE_AFLIB_CALC

#include "AFLibCalcData.h"
#include "AFLibCalcStack.h"
#include "AFLibCalculator.h"
#include "AFLibCompiler.h"
#include "AFLibIDManager.h"
#include "AFLibLexAnalyzerCCode.h"

#ifdef USE_AFLIB_NAMESPACES
using namespace AFLibCalc;
#endif
#endif

#ifdef USE_AFLIB_NAMESPACES
#undef USE_AFLIB_NAMESPACES
#endif
