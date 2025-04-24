#pragma once

#pragma warning(disable : 4244)

#define _WIN32_WINNT 0x0502                 // Win NT version (XP SP1)
#define _WIN32_IE 0x0601                    // Internet Explorer 6.0 SP1
#define VC_EXTRALEAN                        // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define NOMINMAX                            // do not include min-max macros
#define _USE_MATH_DEFINES                   // use commonly-defined math constants like M_PI

#include <afx.h>
#include <afxext.h>    // MFC extensions
#include <afxtempl.h>  // MFC standard template
#include <afxwin.h>    // MFC core and standard components
#include <atlbase.h>   // ATL registry classes

#include <tchar.h>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
