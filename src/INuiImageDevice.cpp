#include "stdafx.h"
#include "INuiImageDevice.h"
//#include "NuiImageDeviceKinect.h"
//#include "NuiImageDeviceTCPClient.h"
#include "NuiImageDeviceVideoFile.h"

#ifndef _DEFINE_SAFERELEASE_
#define _DEFINE_SAFERELEASE_
// Safe release for interfaces
template<class Interface>
inline void SafeRelease( Interface *& pInterfaceToRelease )
{
    if ( pInterfaceToRelease != NULL )
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}
#endif//_DEFINE_SAFERELEASE_

INuiImageDevice * INuiImageDevice::m_sDeviceInstance = NULL;


INuiImageDevice::INuiImageDevice(void):
	m_firstLoad(true),
	m_continueProcessing(true)
{
}

INuiImageDevice::~INuiImageDevice(void)
{
   // delete m_sDeviceInstance;
}

INuiImageDevice * INuiImageDevice::CreateDevice(const NUI_DEVICE_TYPE eDeviceType/* = NUI_DEVICE_KINECT*/)
{
    if (m_sDeviceInstance == NULL)
    {
        INuiImageDevice * pNuiImageDevice = NULL;
        switch (eDeviceType)
        {
//        case NUI_DEVICE_KINECT:
//#ifdef USE_DEVICE_KINECT
//            pNuiImageDevice = new NuiImageDeviceKinect();
//            ASSERT(pNuiImageDevice);
//#endif
//            break;
//        case NUI_DEVICE_TCP_CLIENT:
//            pNuiImageDevice = new NuiImageDeviceTCPClient();
//            ASSERT(pNuiImageDevice);
//            break;
        case NUI_DEVICE_VIDEO_FILE:
            //ASSERT("NUI_DEVICE_VIDEO_FILE NOT IMPLEMENTED!");
            pNuiImageDevice = new NuiImageDeviceVideoFile();
            ASSERT(pNuiImageDevice);
            break;
        default:
            ASSERT("ERROR NUI_DEVICE_TYPE");
            break;
        }
        if (m_sDeviceInstance == NULL)
        {
            m_sDeviceInstance = pNuiImageDevice;
        }
        else if(pNuiImageDevice)
        {
            delete pNuiImageDevice;
        }
    }
    return m_sDeviceInstance;
}

void INuiImageDevice::destroyDevice()
{
    if (m_sDeviceInstance != NULL)
    {
        delete m_sDeviceInstance;
        m_sDeviceInstance = NULL;
    }
}

