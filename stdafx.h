
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once
#define MEASURE_TIME(X) X
#define ASSERT assert
//#include <QDebug>
#include <iostream>
using namespace std;
#include "Util.h"
#include <string.h>
#include <assert.h>
#include <limits.h>
#ifdef WIN32
#define USE_DEVICE_KINECT

#ifdef  _DEBUG  
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#ifndef DBG_NEW      
#define DBG_NEW new(_NORMAL_BLOCK,__FILE__,__LINE__)
#define new DBG_NEW  
#endif
#endif  

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif
#include <windows.h>

//#include "targetver.h"

//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的
//
//// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
//#define _AFX_ALL_WARNINGS
//
//#include <afxwin.h>         // MFC 核心组件和标准组件
//#include <afxext.h>         // MFC 扩展
//
//
//#include <afxdisp.h>        // MFC 自动化类
//
//
//
//#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
//#endif
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
//#endif // _AFX_NO_AFXCMN_SUPPORT
//
//#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
//
//
//
//
//// Direct2D Header Files
//#include <d2d1.h>
//#include <d2d1helper.h>
//
//// DirectWrite
//#include <dwrite.h>
//
//// timeGetTime
//#include <mmsystem.h>
//
//
//#pragma comment ( lib, "winmm.lib" )
//#pragma comment ( lib, "d2d1.lib" )
//#pragma warning ( disable: 4995 )
//
//
//
//#ifdef _UNICODE
//#if defined _M_IX86
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#elif defined _M_X64
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#else
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#endif
//#endif
//
//
//#ifndef _DEFINE_SAFERELEASE_
//#define _DEFINE_SAFERELEASE_
//// Safe release for interfaces
//template<class Interface>
//inline void SafeRelease( Interface *& pInterfaceToRelease )
//{
//    if ( pInterfaceToRelease != NULL )
//    {
//        pInterfaceToRelease->Release();
//        pInterfaceToRelease = NULL;
//    }
//}
//#endif//_DEFINE_SAFERELEASE_
//
//#pragma warning (disable : 4005)
//
//#ifdef USE_DEVICE_KINECT
//#pragma comment( lib, "Kinect10" )
//#endif
#else


#define INVALID_HANDLE_VALUE   -1
#define _MAX_PATH       260 /* max. length of full pathname */
typedef void                *HANDLE;
//#define HANDLE            *void
#define MAX_PATH        260
#define TRUE                true
#define FALSE               false
#define __stdcall
#define __declspec(x)
#define __cdecl
//#define max(a,b)           (((a) > (b)) ? (a) : (b))
//#define min(a,b)            (((a) < (b)) ? (a) : (b))

typedef int                        BOOL;
typedef unsigned char       BYTE;
typedef float                     FLOAT;
typedef FLOAT                  *PFLOAT;
typedef signed char                      CHAR;
typedef unsigned char       UCHAR;
typedef unsigned char       *PUCHAR;
typedef short                    SHORT;
typedef unsigned short      USHORT;
typedef unsigned short      *PUSHORT;
typedef long                      LONG;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef long long              LONGLONG;
typedef unsigned long long  ULONGLONG;
typedef ULONGLONG           *PULONGLONG;
typedef unsigned long       ULONG;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;
typedef void                VOID;

typedef char               *LPSTR;
typedef const char         *LPCSTR;
typedef wchar_t            WCHAR;
typedef WCHAR              *LPWSTR;
typedef const WCHAR        *LPCWSTR;
typedef DWORD              *LPDWORD;
typedef unsigned long      UINT_PTR;
typedef UINT_PTR           SIZE_T;
typedef LONGLONG           USN;
typedef BYTE               BOOLEAN;
typedef void                *PVOID;
typedef void                *LPVOID;
typedef struct _FILETIME {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
     }    FILETIME;

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;
  ULONGLONG QuadPart;
} LARGE_INTEGER,
 *PLARGE_INTEGER;

typedef unsigned char byte;
typedef int  HRESULT;
#define WINAPI
#define S_OK 1
#define S_FALSE -1
#define STDMETHODCALLTYPE




#ifdef WIN32

#else
#include <sys/time.h>
#endif


#endif

double gettimeus();
