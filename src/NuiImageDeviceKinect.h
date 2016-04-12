#ifndef _NUIIMAGEDEVICEKINECT_H
#define _NUIIMAGEDEVICEKINECT_H
#include "StdAfx.h"
#ifdef USE_DEVICE_KINECT
#include <NuiApi.h>
#include "INuiImageDevice.h"

class NuiImageDeviceKinect
    : public INuiImageDevice
{
    static const int  cDepthWidth  = 320;
    static const int  cDepthHeight = 240;
public:
    NuiImageDeviceKinect();
    ~NuiImageDeviceKinect();

    int InitDevice(DWORD lParam = 0, DWORD wParam = 0);
    void UnInit();
    int SetDepthCallBack(pCallBackFunc);
    const USHORT * GetDepthMap();
protected:
    static DWORD WINAPI     Nui_ProcessThread( LPVOID pParam );
    DWORD WINAPI            Nui_ProcessThread( );

    // Current kinect
    INuiSensor *            m_pNuiSensor;
    BSTR                    m_instanceId;
    // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;

    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_pDepthStreamHandle;

    USHORT *      m_pTempDepthBuffer;
    int           m_DepthFramesTotal;

    pCallBackFunc m_pFuncGotDepthAlert;

private:
};
#endif USE_DEVICE_KINECT

#endif _NUIIMAGEDEVICEKINECT_H