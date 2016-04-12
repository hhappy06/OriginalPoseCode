#ifndef _NUIIMAGEDEVICEVIDEOFILE_H
#define _NUIIMAGEDEVICEVIDEOFILE_H
#include "stdafx.h"
//#include <boost/array.hpp>
//#include <boost/asio.hpp>
//#include <boost/thread.hpp>
#include <mutex>
#include <atomic>

#include "INuiImageDevice.h"

class NuiImageDeviceVideoFile
    : public INuiImageDevice
{
    static const int   cDepthWidth  = 320;
    static const int   cDepthHeight = 240;
public:
    NuiImageDeviceVideoFile(void);
    ~NuiImageDeviceVideoFile(void);

    int InitDevice(DWORD lParam = 0, DWORD wParam = 0);
    void UnInit();
    int SetDepthCallBack(pCallBackFunc);
    const USHORT * GetDepthMap();

private:
    BOOL LoadDepthImage(int i);

    // TimerThread
    static DWORD WINAPI     TimerThread(LPVOID pParam);
    DWORD WINAPI            TimerThread1();

    // Nui_ProcessThread
    static DWORD WINAPI     Nui_ProcessThread( LPVOID pParam );
    DWORD WINAPI            Nui_ProcessThread1( );

    // io_service
   // boost::asio::io_service m_io_service;

    // thread handling
    HANDLE        m_hThTimerThread;
    HANDLE        m_hEvTimerThreadStop;
    HANDLE        m_hThNuiProcessThread;
    HANDLE        m_hEvNuiProcessThreadStop;

    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_pDepthStreamHandle;

    USHORT *      m_pTempDepthBuffer;
    USHORT *      m_pDepthFileBuffer;
    int           m_DepthFramesTotal;
    int           m_TimerEventCount;

    pCallBackFunc m_pFuncGotDepthAlert;
};



#endif _NUIIMAGEDEVICEVIDEOFILE_H
