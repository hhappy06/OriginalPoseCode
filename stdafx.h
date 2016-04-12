
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

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
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif
#include <windows.h>

//#include "targetver.h"

//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��
//
//// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
//#define _AFX_ALL_WARNINGS
//
//#include <afxwin.h>         // MFC ��������ͱ�׼���
//#include <afxext.h>         // MFC ��չ
//
//
//#include <afxdisp.h>        // MFC �Զ�����
//
//
//
//#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
//#endif
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
//#endif // _AFX_NO_AFXCMN_SUPPORT
//
//#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��
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
