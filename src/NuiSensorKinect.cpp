#include "stdafx.h"
#include <iostream>
#ifdef USE_DEVICE_KINECT
#include <NuiApi.h>
#include "NuiSensorKinect.h"

//using namespace NuiSinect;

static INuiSensor * m_pNuiSensor = NULL;

namespace NuiSinect {

NuiSensorKinect * NuiSensorKinect::m_self = NULL;


NuiSensorKinect::NuiSensorKinect(void)
{
    //平滑参数默认值
    m_defaultSmoothingParams.fSmoothing          = 0.5f;
    m_defaultSmoothingParams.fCorrection         = 0.5f;
    m_defaultSmoothingParams.fPrediction         = 0.5f;
    m_defaultSmoothingParams.fJitterRadius       = 0.05f;
    m_defaultSmoothingParams.fMaxDeviationRadius = 0.04f;

}


NuiSensorKinect::~NuiSensorKinect(void)
{
}



HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiInitialize( 
    /* [in] */ DWORD dwFlags)
{
    HRESULT  hr;

    if ( !m_pNuiSensor )
    {
        //hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    hr = m_pNuiSensor->NuiInitialize(dwFlags);
    return hr;
}

void STDMETHODCALLTYPE NuiSensorKinect::NuiShutdown( void)
{
    if ( !m_pNuiSensor )
        m_pNuiSensor->NuiShutdown();
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiImageStreamOpen( 
    /* [in] */ NUI_IMAGE_TYPE eImageType,
    /* [in] */ NUI_IMAGE_RESOLUTION eResolution,
    /* [in] */ DWORD dwImageFrameFlags,
    /* [in] */ DWORD dwFrameLimit,
    /* [in] */ HANDLE hNextFrameEvent,
    /* [out] */ HANDLE *phStreamHandle)
{
    if ( !m_pNuiSensor )
        return m_pNuiSensor->NuiImageStreamOpen(eImageType, eResolution, dwImageFrameFlags, dwFrameLimit, hNextFrameEvent, phStreamHandle);
    else
        return S_FALSE;
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiImageStreamSetImageFrameFlags( 
    /* [in] */ HANDLE hStream,
    /* [in] */ DWORD dwImageFrameFlags)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiImageStreamGetNextFrame( 
    /* [in] */ HANDLE hStream,
    /* [in] */ DWORD dwMillisecondsToWait,
    /* [retval][out] */ NUI_IMAGE_FRAME *pImageFrame)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiImageStreamReleaseFrame( 
    /* [in] */ HANDLE hStream,
    /* [in] */ NUI_IMAGE_FRAME *pImageFrame)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiSkeletonTrackingEnable( 
    /* [in] */ HANDLE hNextFrameEvent,
    /* [in] */ DWORD dwFlags)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiSkeletonTrackingDisable( void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiSkeletonSetTrackedSkeletons( 
    /* [size_is][in] */ DWORD *TrackingIDs)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE NuiSensorKinect::NuiSkeletonGetNextFrame( 
    /* [in] */ DWORD dwMillisecondsToWait,
    /* [out][in] */ NUI_SKELETON_FRAME *pSkeletonFrame)
{
    return S_OK;
}


HRESULT NuiSensorKinect::NuiTransformSmooth(NUI_SKELETON_FRAME *pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS *apSmoothingParams/* = NULL*/)
{
    NUI_TRANSFORM_SMOOTH_PARAMETERS smoothingParams, params;

    if (pSkeletonFrame == NULL)
    {
        return S_FALSE;
    }
    if (apSmoothingParams == NULL)
    {
        memcpy(&smoothingParams, &m_defaultSmoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
    }
    else
    {
        memcpy(&smoothingParams, apSmoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
    }


    // 检查平滑参数
    if (smoothingParams.fJitterRadius < 0.001f)
    {
        smoothingParams.fJitterRadius = 0.001f;
    }

    for (int skeleton_i = 0; skeleton_i < 6; skeleton_i ++)
    {
        if (pSkeletonFrame->SkeletonData[skeleton_i].eTrackingState !=  NUI_SKELETON_TRACKED)
        {
            continue;
        }
        
        NUI_SKELETON_DATA * p_skeleton_data = &(pSkeletonFrame->SkeletonData[skeleton_i]);
        Joints_Smoothing_State * p_smoothingState = &m_msSmoothingState[skeleton_i];

        for (int joint_i = 0; joint_i < 20; joint_i ++)
        {
            memcpy(&params, &smoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
            if (p_skeleton_data->eSkeletonPositionTrackingState[joint_i] == NUI_SKELETON_POSITION_NOT_TRACKED)
            {
                continue;
            }
            else if (p_skeleton_data->eSkeletonPositionTrackingState[joint_i] == NUI_SKELETON_POSITION_INFERRED)
            {
                params.fJitterRadius       *= 2;
                params.fMaxDeviationRadius *= 2;
            }
            
            Vector4 X,   X1, // 输入关节坐标/上次输入关节坐标
                     Xr,      // JitterRadius修正
                     Xn,  Xn1,// Smoothing
                     Bn,  Bn1,// Correction
                     Xnp,     // Prediction
                     Xnpr,    // MaxDeviationRadius修正
                     Xo;      // 输出关节坐标

            Bn1 = p_smoothingState->m_Bn[joint_i];
            Xn1 = p_smoothingState->m_Xn[joint_i];
            X1  = p_smoothingState->m_X[joint_i];

            X.x = p_skeleton_data->SkeletonPositions[joint_i].x;
            X.y = p_skeleton_data->SkeletonPositions[joint_i].y;
            X.z = p_skeleton_data->SkeletonPositions[joint_i].z;
            X.w = p_skeleton_data->SkeletonPositions[joint_i].w;

            if (X.x == 0.0f && X.y == 0.0f && X.z == 0.0f)
            {
                p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_INITIAL;
            }

            if (p_smoothingState->m_FilterState[joint_i] == SMOOTH_FILTER_INITIAL)
            {
                p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_NTRACKED;
                Xn.x = X.x; Xn.y = X.y; Xn.z = X.z; Xn.w = X.w;
                Bn.x = Bn.y = Bn.z = Bn.w = 0;
            }
            else if (p_smoothingState->m_FilterState[joint_i] == SMOOTH_FILTER_NTRACKED)
            {
                // Smoothing
                // X~n.x = (Xn.x + X(n-1).x) * 0.5
                Xn.x = (X.x + X1.x) * 0.5f;
                Xn.y = (X.y + X1.y) * 0.5f;
                Xn.z = (X.z + X1.z) * 0.5f;
                Xn.w = (X.w + X1.w) * 0.5f;

                // Correction
                // bn.x = fCorrection * ((Xn.x + X(n-1).x) * 0.5 - X~(n-1).x) + (1 - fCorrection) * b(n-1).x
                Bn.x = params.fCorrection * (Xn.x - Xn1.x) + (1 - params.fCorrection) * Bn1.x;
                Bn.y = params.fCorrection * (Xn.y - Xn1.y) + (1 - params.fCorrection) * Bn1.y;
                Bn.z = params.fCorrection * (Xn.z - Xn1.z) + (1 - params.fCorrection) * Bn1.z;
                Bn.w = params.fCorrection * (Xn.w - Xn1.w) + (1 - params.fCorrection) * Bn1.w;

                // Update Filter Status
                p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_TRACKED;
            }
            else // SMOOTH_FILTER_TRACKED
            {
                // check JitterRadius, repair X got Xr
                float X_move_distance = sqrt((X.x-Xn1.x)*(X.x-Xn1.x) + (X.y-Xn1.y)*(X.y-Xn1.y) + (X.z-Xn1.z)*(X.z-Xn1.z));

                if (X_move_distance > params.fJitterRadius)
                {
                    Xr = X;
                }
                else
                {
                    float XMoveRatio = X_move_distance / params.fJitterRadius;
                    ASSERT(XMoveRatio <= 1.0f);
                    Xr.x = (1 - XMoveRatio) * Xn1.x + XMoveRatio * X.x;
                    Xr.y = (1 - XMoveRatio) * Xn1.y + XMoveRatio * X.y;
                    Xr.z = (1 - XMoveRatio) * Xn1.z + XMoveRatio * X.z;
                    Xr.w = (1 - XMoveRatio) * Xn1.w + XMoveRatio * X.w;
                }
                // Smoothing
                // (1 - fSmoothing) * Xn.x + fSmoothing * (X~(n-1).x + b(n-1).x)
                Xn.x = (1 - params.fSmoothing) * Xr.x + params.fSmoothing * (Xn1.x + Bn1.x);
                Xn.y = (1 - params.fSmoothing) * Xr.y + params.fSmoothing * (Xn1.y + Bn1.y);
                Xn.z = (1 - params.fSmoothing) * Xr.z + params.fSmoothing * (Xn1.z + Bn1.z);
                Xn.w = (1 - params.fSmoothing) * Xr.w + params.fSmoothing * (Xn1.w + Bn1.w);

                // Correction
                // fCorrection * (X~n.x - X~(n-1).x) + (1 - fCorrection) * b(n-1).x = bn.x
                Bn.x = params.fCorrection * (Xn.x - Xn1.x) + (1 - params.fCorrection) * Bn1.x;
                Bn.y = params.fCorrection * (Xn.y - Xn1.y) + (1 - params.fCorrection) * Bn1.y;
                Bn.z = params.fCorrection * (Xn.z - Xn1.z) + (1 - params.fCorrection) * Bn1.z;
                Bn.w = params.fCorrection * (Xn.w - Xn1.w) + (1 - params.fCorrection) * Bn1.w;
            }

            // Prediction
            Xnp.x = Xn.x + params.fPrediction * Bn.x;
            Xnp.y = Xn.y + params.fPrediction * Bn.y;
            Xnp.z = Xn.z + params.fPrediction * Bn.z;
            Xnp.w = Xn.w + params.fPrediction * Bn.w;

            // check MaxDeviationRadius, repair Xnp got Xno
            float predict_diff_distance = sqrt((Xnp.x - X.x)*(Xnp.x - X.x) + (Xnp.y - X.y)*(Xnp.y - X.y) + (Xnp.z - X.z)*(Xnp.z - X.z));;

            if (predict_diff_distance > params.fMaxDeviationRadius)
            {
                float EffectDevRatio = params.fMaxDeviationRadius / predict_diff_distance;
                ASSERT(EffectDevRatio <= 1.0f);

                // X~((n+k)|n).x * EffectDevRatio + (1 - EffectDevRatio) * SkePos[i].x
                Xnpr.x = Xnp.x * EffectDevRatio + (1 - EffectDevRatio) * X.x;
                Xnpr.y = Xnp.y * EffectDevRatio + (1 - EffectDevRatio) * X.y;
                Xnpr.z = Xnp.z * EffectDevRatio + (1 - EffectDevRatio) * X.z;
                Xnpr.w = Xnp.w * EffectDevRatio + (1 - EffectDevRatio) * X.w;

                Xo.x = Xnpr.x; Xo.y = Xnpr.y; Xo.z = Xnpr.z; Xo.w = Xnpr.w;
            }
            else
            {
                Xo.x = Xnp.x; Xo.y = Xnp.y; Xo.z = Xnp.z; Xo.w = Xnp.w;
            }
            
            p_smoothingState->m_Bn[joint_i] = Bn;
            p_smoothingState->m_Xn[joint_i] = Xn;
            p_smoothingState->m_X[joint_i] = X;

            p_skeleton_data->SkeletonPositions[joint_i].x = Xo.x;
            p_skeleton_data->SkeletonPositions[joint_i].y = Xo.y;
            p_skeleton_data->SkeletonPositions[joint_i].z = Xo.z;
            p_skeleton_data->SkeletonPositions[joint_i].w = 1.0f;
        }
    }

    return S_OK;
}

#if 0
HRESULT NuiSensorKinect::TransformSmooth(Vector4 * joints, const NUI_TRANSFORM_SMOOTH_PARAMETERS * apSmoothingParams/* = NULL*/)
{
    NUI_TRANSFORM_SMOOTH_PARAMETERS smoothingParams, params;

    if (joints == NULL)
    {
        return S_FALSE;
    }
    if (apSmoothingParams == NULL)
    {
        memcpy(&smoothingParams, &m_smoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
    }
    else
    {
        memcpy(&smoothingParams, apSmoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
    }


    // 检查平滑参数
    if (smoothingParams.fJitterRadius < 0.001f)
    {
        smoothingParams.fJitterRadius = 0.001f;
    }

    Joints_Smoothing_State * p_smoothingState = &m_smoothingState;

    for (int joint_i = 0; joint_i < 20; joint_i ++)
    {
        memcpy(&params, &smoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
#if 0
        // 目前暂无JointTrackingState功能
        if (p_skeleton_data->eSkeletonPositionTrackingState[joint_i] == NUI_SKELETON_POSITION_NOT_TRACKED)
        {
            continue;
        }
        else if (p_skeleton_data->eSkeletonPositionTrackingState[joint_i] == NUI_SKELETON_POSITION_INFERRED)
        {
            params.fJitterRadius       *= 2;
            params.fMaxDeviationRadius *= 2;
        }
#else
        // Joint的xyz全为0则说明此Joint提取失败, 暂不做Smooth
        if (joints[joint_i].x == 0 && joints[joint_i].y == 0 && joints[joint_i].z == 0)
        {
            continue;
        }
        // Joint的xyz全为NAN则说明此Joint提取失败, 暂不做Smooth, 且赋0
        if (*((UINT*)&(joints[joint_i].x)) == 0xFFC00000 
            || *((UINT*)&(joints[joint_i].y)) == 0xFFC00000 
            || *((UINT*)&(joints[joint_i].z)) == 0xFFC00000)
        {
            joints[joint_i].x = 0.0f;
            joints[joint_i].y = 0.0f;
            joints[joint_i].z = 0.0f;
            continue;
        }
#endif

        Vector4 X,   X1, // 输入关节坐标/上次输入关节坐标
            Xr,      // JitterRadius修正
            Xn,  Xn1,// Smoothing
            Bn,  Bn1,// Correction
            Xnp,     // Prediction
            Xnpr,    // MaxDeviationRadius修正
            Xo;      // 输出关节坐标

        Bn1 = p_smoothingState->m_Bn[joint_i];
        Xn1 = p_smoothingState->m_Xn[joint_i];
        X1  = p_smoothingState->m_X[joint_i];

        X.x = joints[joint_i].x;
        X.y = joints[joint_i].y;
        X.z = joints[joint_i].z;
        X.w = joints[joint_i].w;

        if (X.x == 0.0f && X.y == 0.0f && X.z == 0.0f)
        {
            p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_INITIAL;
        }

        if (p_smoothingState->m_FilterState[joint_i] == SMOOTH_FILTER_INITIAL)
        {
            p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_NTRACKED;
            Xn.x = X.x; Xn.y = X.y; Xn.z = X.z; Xn.w = X.w;
            Bn.x = Bn.y = Bn.z = Bn.w = 0;
        }
        else if (p_smoothingState->m_FilterState[joint_i] == SMOOTH_FILTER_NTRACKED)
        {
            // Smoothing
            // X~n.x = (Xn.x + X(n-1).x) * 0.5
            Xn.x = (X.x + X1.x) * 0.5f;
            Xn.y = (X.y + X1.y) * 0.5f;
            Xn.z = (X.z + X1.z) * 0.5f;
            Xn.w = (X.w + X1.w) * 0.5f;

            // Correction
            // bn.x = fCorrection * ((Xn.x + X(n-1).x) * 0.5 - X~(n-1).x) + (1 - fCorrection) * b(n-1).x
            Bn.x = params.fCorrection * (Xn.x - Xn1.x) + (1 - params.fCorrection) * Bn1.x;
            Bn.y = params.fCorrection * (Xn.y - Xn1.y) + (1 - params.fCorrection) * Bn1.y;
            Bn.z = params.fCorrection * (Xn.z - Xn1.z) + (1 - params.fCorrection) * Bn1.z;
            Bn.w = params.fCorrection * (Xn.w - Xn1.w) + (1 - params.fCorrection) * Bn1.w;

            // Update Filter Status
            p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_TRACKED;
        }
        else // SMOOTH_FILTER_TRACKED
        {
            // check JitterRadius, repair X got Xr
            float X_move_distance = sqrt((X.x-Xn1.x)*(X.x-Xn1.x) + (X.y-Xn1.y)*(X.y-Xn1.y) + (X.z-Xn1.z)*(X.z-Xn1.z));

            if (X_move_distance > params.fJitterRadius)
            {
                Xr = X;
            }
            else
            {
                float XMoveRatio = X_move_distance / params.fJitterRadius;
                assert(XMoveRatio <= 1.0f);
                Xr.x = (1 - XMoveRatio) * Xn1.x + XMoveRatio * X.x;
                Xr.y = (1 - XMoveRatio) * Xn1.y + XMoveRatio * X.y;
                Xr.z = (1 - XMoveRatio) * Xn1.z + XMoveRatio * X.z;
                Xr.w = (1 - XMoveRatio) * Xn1.w + XMoveRatio * X.w;
            }
            // Smoothing
            // (1 - fSmoothing) * Xn.x + fSmoothing * (X~(n-1).x + b(n-1).x)
            Xn.x = (1 - params.fSmoothing) * Xr.x + params.fSmoothing * (Xn1.x + Bn1.x);
            Xn.y = (1 - params.fSmoothing) * Xr.y + params.fSmoothing * (Xn1.y + Bn1.y);
            Xn.z = (1 - params.fSmoothing) * Xr.z + params.fSmoothing * (Xn1.z + Bn1.z);
            Xn.w = (1 - params.fSmoothing) * Xr.w + params.fSmoothing * (Xn1.w + Bn1.w);

            // Correction
            // fCorrection * (X~n.x - X~(n-1).x) + (1 - fCorrection) * b(n-1).x = bn.x
            Bn.x = params.fCorrection * (Xn.x - Xn1.x) + (1 - params.fCorrection) * Bn1.x;
            Bn.y = params.fCorrection * (Xn.y - Xn1.y) + (1 - params.fCorrection) * Bn1.y;
            Bn.z = params.fCorrection * (Xn.z - Xn1.z) + (1 - params.fCorrection) * Bn1.z;
            Bn.w = params.fCorrection * (Xn.w - Xn1.w) + (1 - params.fCorrection) * Bn1.w;
        }

        // Prediction
        Xnp.x = Xn.x + params.fPrediction * Bn.x;
        Xnp.y = Xn.y + params.fPrediction * Bn.y;
        Xnp.z = Xn.z + params.fPrediction * Bn.z;
        Xnp.w = Xn.w + params.fPrediction * Bn.w;

        // check MaxDeviationRadius, repair Xnp got Xno
        float predict_diff_distance = sqrt((Xnp.x - X.x)*(Xnp.x - X.x) + (Xnp.y - X.y)*(Xnp.y - X.y) + (Xnp.z - X.z)*(Xnp.z - X.z));;

        if (predict_diff_distance > params.fMaxDeviationRadius)
        {
            float EffectDevRatio = params.fMaxDeviationRadius / predict_diff_distance;
            assert(EffectDevRatio <= 1.0f);

            // X~((n+k)|n).x * EffectDevRatio + (1 - EffectDevRatio) * SkePos[i].x
            Xnpr.x = Xnp.x * EffectDevRatio + (1 - EffectDevRatio) * X.x;
            Xnpr.y = Xnp.y * EffectDevRatio + (1 - EffectDevRatio) * X.y;
            Xnpr.z = Xnp.z * EffectDevRatio + (1 - EffectDevRatio) * X.z;
            Xnpr.w = Xnp.w * EffectDevRatio + (1 - EffectDevRatio) * X.w;

            Xo.x = Xnpr.x; Xo.y = Xnpr.y; Xo.z = Xnpr.z; Xo.w = Xnpr.w;
        }
        else
        {
            Xo.x = Xnp.x; Xo.y = Xnp.y; Xo.z = Xnp.z; Xo.w = Xnp.w;
        }

        p_smoothingState->m_Bn[joint_i] = Bn;
        p_smoothingState->m_Xn[joint_i] = Xn;
        p_smoothingState->m_X[joint_i] = X;

        joints[joint_i].x = Xo.x;
        joints[joint_i].y = Xo.y;
        joints[joint_i].z = Xo.z;
        joints[joint_i].w = 1.0f;
    }

    return S_OK;
}
#endif


#define FAIL(R) (R==FALSE)
BOOL NuiSensorKinect::_StaticSkeletalTrackingCallback()
{
    if (NuiSensorKinect::m_self)
    {
        return NuiSensorKinect::m_self->SkeletalTrackingCallback();
    }
    return FALSE;
}


BOOL NuiSensorKinect::SkeletalTrackingCallback()
{
    BOOL bContinue = TRUE;

    while (bContinue)
    {
        BOOL res = TRUE;

        // TODO: 初始化局部变量


        // TODO: 等待深度帧更新Event, 获取并复制一份用于处理
        // res = pOutputStream->GetOutputBufferForProcessing( m_hKillSkeletalThread, bufferIndex, frameNumber, &spInputFrame, &spOutputFrame)
        // TODO: 检查深度帧是否获取成功
        if (FAIL(res))
        {
            bContinue = FALSE;
            continue;
        }

        // 检查并初始化INuiFrameTexture * pDepthFrame;

        // KinectNui::STSkeletalTracker::ReceiveFrame()
        //     检查pDepthFrame, AddRef
        //     m_pPipeline->Reset(), 仅当前景消失时\全身模式和坐立模式切换时Reset.

        // KinectNui::STSkeletalTracker::GetSegmentation(struct INuiFrameTexture *)
        //    检查m_spDepthFrame->AddRef();
        //    if (m_pPipeline == NULL) return;
        //    m_spDepthFrame->LockRect(0, &locked, 0, 0);
        //    m_pPipeline->SetDepthFrame(reinterpret_cast<const USHORT*>(locked.pBits), pFrame.p ? pFrame.Obj()->GetFrameNumber() : 0);
        //    pMaskFrame->LockRect(0, &locked, 0, 0);
        //    m_pPipeline->SetDepthSegmentationMap320x240x3bits(reinterpret_cast<USHORT*>(locked.pBits));
        //    m_pPipeline->SetPlayerTrackingIDs(m_trackingIDs);
        //    m_pPipeline->RunSegmentation();
        //    pMaskFrame->UnlockRect();
        //    m_spDepthFrame->UnlockRect();
        //    m_spDepthFrame->Release();

        // Proxy_GetDepthFlags(fIsNearMode, fDepthClamp, fFarModeIsNonZero)

        // KinectNui::STSkeletalTracker::SetTrackedSkeletons()

        // KinectNui::STSkeletalTracker::GetSkeletons(struct _NUI_SKELETON_FRAME *)

        // 如果是双人跟踪
        // NUI_UpdatePlayerTrackingIDsWithSkeletons_TwoPlayer(_NUI_SKELETON_FRAME *,uint,uint *)

        // 输出深度帧
        // 80 : DownsampleDepthPlusPlayerIndex320to80(unsigned short *,unsigned short *)
        // 320: KinectNui::OutputStream::CopyDepthToOutput()
        // 640: UpsampleBlendDepthPlusPlayer320to640(unsigned short *,unsigned short const *,unsigned short const *)

        // SetEvent(深度帧)
        // KinectNui::OutputStream::NotifyBufferProcessed()

        // copy骨骼数据
        // SetEvent(APP骨架)

        // KinectNui::MainNui::ConditionallyFireFrameEndEvent(KinectNui::MainNui::EventTypes)
    }
    return TRUE;
}

}

#endif USE_DEVICE_KINECT
