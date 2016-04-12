#pragma once
#include "stdafx.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef bool ( *pCallBackFunc )();

enum NUI_DEVICE_TYPE
{
    NUI_DEVICE_KINECT = 0,
    NUI_DEVICE_TCP_CLIENT,
    NUI_DEVICE_VIDEO_FILE
};

enum { DEP_IMAGE_SIZE = 640*480*2,
       RGB_IMAGE_SIZE = 640*480*3 };

class INuiImageDevice
{
public:
    INuiImageDevice(void);
    virtual ~INuiImageDevice(void);
    static INuiImageDevice * CreateDevice(const NUI_DEVICE_TYPE eDeviceType = NUI_DEVICE_KINECT);
    static void destroyDevice();
public:
    virtual int  InitDevice(DWORD lParam = 0, DWORD wParam = 0) = 0;
    virtual void UnInit() = 0;
#if 1
    virtual int  SetDepthCallBack(pCallBackFunc) = 0;
#else

#endif
    enum {LOADSTATE_NOTINTHREAD, LOADSTATE_LOADING, LOADSTATE_WAITING, LOADSTATE_BUFFERING};
    enum {CALCSTATE_NOTINTHREAD, CALCSTATE_CALCING, CALCSTATE_WAITING, CALCSTATE_GETING};

    std::atomic<int>  m_loadState;// = LOADSTATE_NOTINTHREAD;
    std::atomic<int>  m_calcState ;//= CALCSTATE_NOTINTHREAD;
    std::atomic<bool> m_bufferReady;
    std::mutex     m_bufferMutex;
    std::condition_variable m_getdepthCondVar;

//    std::mutex     m_calc_sync;
//    std::condition_variable m_depthReadyCondVar;

    bool           m_firstLoad ;
 //   std::mutex     m_depthMapMutex;
 //   std::condition_variable m_depthMapCondVar;

    virtual const USHORT * GetDepthMap() = 0;
protected:
         bool m_continueProcessing;
private:
    static INuiImageDevice * m_sDeviceInstance;

};

