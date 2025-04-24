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

#include <afxadv.h>  // advanced -- CRecenFileList
#include <afxcmn.h>  // MFC support for Windows Common Controls
#include <afxdisp.h>  // for AfxEnableControlContainer()
#include <afxext.h>  // MFC extensions
#include <afxmt.h>  // multi-thread
#include <afxpriv.h>  // private -- WM_COMMANDHELP
#include <afxsock.h>  // MFC sockets
#include <afxtempl.h>  // MFC standard template
#include <afxwin.h>  // MFC core and standard components
#include <atlimage.h>

#include <dlgs.h>
#include <float.h>
#include <math.h>
#include <winspool.h>
#include <winver.h>

#include <fstream>
#include <iostream>
#include <xutility>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
