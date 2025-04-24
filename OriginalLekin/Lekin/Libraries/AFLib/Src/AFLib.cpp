#include "StdAfx.h"

#include "Local.h"

#define INCLUDE_AFLIB
#define INCLUDE_AFLIB_GUI
#define INCLUDE_AFLIB_GUI_COMMON_CTL
#define INCLUDE_AFLIB_GUI_ATL
#define INCLUDE_AFLIB_IO
#define INCLUDE_AFLIB_THREAD
#define INCLUDE_AFLIB_MATH
#define INCLUDE_AFLIB_DB
#define INCLUDE_AFLIB_CALC
#define USE_AFLIB_NAMESPACES
#include "AFLib.h"

namespace AFLib {
const T3State t3Start(-1, 0);
const T3State t3Undef(2, 0);
const COleDateTime odtZero;
const COleDateTimeSpan odtsZero(0);
}  // namespace AFLib

namespace AFLibMath {
const double Nan = sqrt(-1.);
const float NanF = sqrtf(-1.F);
const UINT NanUI = 0xABF1ACDC;
const int NanI = int(NanUI);
const UINT64 NanUI64 = MAKEUINT64(NanI, NanI);
const INT64 NanI64 = INT64(NanUI);
const double Const100 = 100;
}  // namespace AFLibMath

namespace AFLibPrivate {
AFX_EXTENSION_MODULE AFLibDLL = {NULL};
}

namespace AFLibThread {
TThreadSafeStatic<AFLibIO::TStringBufferA> StaticStringBufferStorageA;
TThreadSafeStatic<AFLibIO::TStringBufferW> StaticStringBufferStorageW;
}  // namespace AFLibThread

using namespace AFLibPrivate;

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            TRACE0("AFLib.DLL Initializing!\n");

            // Extension DLL one-time initialization
            if (!AfxInitExtensionModule(AFLibDLL, hInstance)) return 0;

            // Insert this DLL into the resource chain
            // NOTE: If this Extension DLL is being implicitly linked to by
            //  an MFC Regular DLL (such as an ActiveX Control)
            //  instead of an MFC application, then you will want to
            //  remove this line from DllMain and put it in a separate
            //  function exported from this Extension DLL.  The Regular DLL
            //  that uses this Extension DLL should then explicitly call that
            //  function to initialize this Extension DLL.  Otherwise,
            //  the CDynLinkLibrary object will not be attached to the
            //  Regular DLL's resource chain, and serious problems will
            //  result.

            new CDynLinkLibrary(AFLibDLL);
            AfxInitRichEdit2();
            break;

        case DLL_PROCESS_DETACH:
            TRACE0("AFLib.DLL Terminating!\n");

            // Terminate the library before destructors are called
            AfxTermExtensionModule(AFLibDLL);
            break;
    }

    return 1;
}
