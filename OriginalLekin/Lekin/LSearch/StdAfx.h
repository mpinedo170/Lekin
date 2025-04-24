#pragma once

#define _WIN32_WINNT 0x0502  // Win NT version (XP SP1)
#define _WIN32_IE 0x0601     // Internet Explorer 6.0 SP1
#define VC_EXTRALEAN         // Exclude rarely-used stuff from Windows headers
#define OEMRESOURCE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define _CRT_SECURE_NO_WARNINGS

#include <afxtempl.h>
#include <afxwin.h>  // MFC core and standard components
#include <math.h>
