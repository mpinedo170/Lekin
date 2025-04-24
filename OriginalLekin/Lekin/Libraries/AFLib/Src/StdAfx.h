#pragma once

#define _WIN32_WINNT 0x0502                 // Win NT version (XP SP1)
#define _WIN32_IE 0x0601                    // Internet Explorer 6.0 SP1
#define VC_EXTRALEAN                        // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define NOMINMAX                            // do not include min-max macros
#define _USE_MATH_DEFINES                   // use commonly-defined math constants like M_PI

#include <conio.h>    // console i/o
#include <algorithm>  // STL template algorithms, including min, max
#include <cfloat>     // C floating point definitions
#include <cmath>      // C math functions
using namespace std;

#include <afxdb.h>  // MFC ODBC database
#include <afxdllx.h>  // MFC extension DLL support
#include <afxinet.h>  // MFC internet connections
#include <afxmt.h>  // MFC multi-threading capabilities
#include <afxpriv.h>  // MFC private (undocumented) classes, like CPeviewDC
#include <afxrich.h>  // MFC rich edit classes
#include <afxtempl.h>  // MFC standard template classes
#include <afxwin.h>  // MFC core and standard components
#include <atlimage.h>  // ATL images
#include <atlsecurity.h>  // ATL NT security

#include <shlobj.h>  // SHGetSpecialFolderPath() and other shell functions
#include <tlhelp32.h>  // ToolHelp: Win-95 process enumeration
#include <vdmdbg.h>  // Win-NT 16-bit process enumeration
#include <winioctl.h>  // DeviceIoControl() flags

#define AFLIB __declspec(dllexport)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
