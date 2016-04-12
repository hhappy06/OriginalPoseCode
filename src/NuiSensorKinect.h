#pragma once
#include "NuiSinect.h"
#ifdef USE_DEVICE_KINECT

namespace NuiSinect
{
#ifndef _SMOOTH_FILTER_STATE_
#define _SMOOTH_FILTER_STATE_
typedef
enum _SMOOTH_FILTER_STATE
{
    SMOOTH_FILTER_INITIAL  = 0,
    SMOOTH_FILTER_NTRACKED = SMOOTH_FILTER_INITIAL + 1,
    SMOOTH_FILTER_TRACKED  = SMOOTH_FILTER_NTRACKED + 1
}   SMOOTH_FILTER_STATE;
#endif _SMOOTH_FILTER_STATE_

#ifndef _Joints_Smoothing_State_
#define _Joints_Smoothing_State_
typedef 
struct _Joints_Smoothing_State
{
    _Joints_Smoothing_State()
    {
        memset(&m_Bn, 0, 20 * sizeof(Vector4));
        memset(&m_Xn, 0, 20 * sizeof(Vector4));
        memset(&m_X, 0, 20 * sizeof(Vector4));
        memset(&m_FilterState, 0, 20 * sizeof(SMOOTH_FILTER_STATE));
    }
    Vector4 m_Bn[20];
    Vector4 m_Xn[20];
    Vector4 m_X[20];
    SMOOTH_FILTER_STATE m_FilterState[20];
}   Joints_Smoothing_State;
#endif _Joints_Smoothing_State_

class NuiSensorKinect
    : public INuiSensor
{
public:
    NuiSensorKinect(void);

    ~NuiSensorKinect(void);

    virtual HRESULT STDMETHODCALLTYPE NuiInitialize( 
        /* [in] */ DWORD dwFlags);

    virtual void STDMETHODCALLTYPE NuiShutdown( void);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamOpen( 
        /* [in] */ NUI_IMAGE_TYPE eImageType,
        /* [in] */ NUI_IMAGE_RESOLUTION eResolution,
        /* [in] */ DWORD dwImageFrameFlags,
        /* [in] */ DWORD dwFrameLimit,
        /* [in] */ HANDLE hNextFrameEvent,
        /* [out] */ HANDLE *phStreamHandle);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamSetImageFrameFlags( 
        /* [in] */ HANDLE hStream,
        /* [in] */ DWORD dwImageFrameFlags);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamGetNextFrame( 
        /* [in] */ HANDLE hStream,
        /* [in] */ DWORD dwMillisecondsToWait,
        /* [retval][out] */ NUI_IMAGE_FRAME *pImageFrame);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamReleaseFrame( 
        /* [in] */ HANDLE hStream,
        /* [in] */ NUI_IMAGE_FRAME *pImageFrame);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingEnable( 
        /* [in] */ HANDLE hNextFrameEvent,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingDisable( void);


    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonSetTrackedSkeletons( 
        /* [size_is][in] */ DWORD *TrackingIDs);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonGetNextFrame( 
        /* [in] */ DWORD dwMillisecondsToWait,
        /* [out][in] */ NUI_SKELETON_FRAME *pSkeletonFrame);

    // 骨骼平滑
    virtual HRESULT STDMETHODCALLTYPE NuiTransformSmooth( 
        NUI_SKELETON_FRAME *pSkeletonFrame,
        const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams = NULL);
private:
    // 算法处理主函数(回调函数)
    static BOOL _StaticSkeletalTrackingCallback();
    BOOL SkeletalTrackingCallback();
private:
    // this
    static NuiSensorKinect * m_self;
    // smooth用
    NUI_TRANSFORM_SMOOTH_PARAMETERS m_defaultSmoothingParams;
    Joints_Smoothing_State          m_msSmoothingState[6];
    //Joints_Smoothing_State          m_smoothingState;
};


}

#endif USE_DEVICE_KINECT