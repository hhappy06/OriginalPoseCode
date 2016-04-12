#include "stdafx.h"
#include "SkeletalTracker.h"

namespace NuiSinect {

SkeletalTracker * SkeletalTracker::m_pSelf = NULL;


SkeletalTracker::SkeletalTracker(void)
{
    // NuiImageDevice
    m_pNuiImageDevice       = NULL;
    // algorithm pipeline
    m_pPipelineSourceStage  = NULL;
    m_pBGRStage             = NULL;
    m_pExemplarStage        = NULL;
    m_pCentroidsStage       = NULL;
    m_pModelFittingStage    = NULL;
    m_pPipelineOutputStage  = NULL;
}


SkeletalTracker::~SkeletalTracker(void)
{
    if (m_pNuiImageDevice)
    {
        ShutDown();
    }
}


BOOL SkeletalTracker::Initialize()
{
    // initialize NuiImageDevice
    int res;
    if (m_pNuiImageDevice == NULL)
    {
        m_pNuiImageDevice = INuiImageDevice::CreateDevice();
        if (m_pNuiImageDevice == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pNuiImageDevice);

    res = m_pNuiImageDevice->InitDevice();
    if (res != 0)
    {
        // 初始化错误
        /*CString str;
        str.Format("Kinect初始化失败! res = %08X", res);
        AfxMessageBox(str);*/
        return FALSE;
    }
    m_pNuiImageDevice->SetDepthCallBack((pCallBackFunc)_StaticSkeletalTrackingProc);

    // initialize algorithm pipelines
    // PipelineSourceStage
    if (m_pPipelineSourceStage == NULL)
    {
        m_pPipelineSourceStage = new PipelineSourceStage();
        if (m_pPipelineSourceStage == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pPipelineSourceStage);
    m_pPipelineSourceStage->Initialize();

    // BGRStage
    if (m_pBGRStage == NULL)
    {
        m_pBGRStage = new BGRStage();
        if (m_pBGRStage == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pBGRStage);
    m_pBGRStage->Initialize();

    // ExemplarStage
    if (m_pExemplarStage == NULL)
    {
        m_pExemplarStage = new ExemplarStage();
        if (m_pExemplarStage == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pExemplarStage);
    m_pExemplarStage->Initialize(0);

    //CentroidsStage
    if (m_pCentroidsStage == NULL)
    {
        m_pCentroidsStage = new CentroidsStage();
        if (m_pCentroidsStage == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pCentroidsStage);
    m_pCentroidsStage->Initialize();

    //ModelFittingStage
    if (m_pModelFittingStage == NULL)
    {
        m_pModelFittingStage = new ModelFittingStage();
        if (m_pModelFittingStage == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pModelFittingStage);
    m_pModelFittingStage->Initialize();

    // PipelineOutputStage
    if (m_pPipelineOutputStage == NULL)
    {
        m_pPipelineOutputStage = new PipelineOutputStage();
        if (m_pPipelineOutputStage == NULL)
        {
            return FALSE;
        }
    }
    ASSERT(m_pPipelineOutputStage);
    m_pPipelineOutputStage->Initialize();


    return TRUE;
}


BOOL SkeletalTracker::ShutDown()
{
    // 停止NuiImageDevice, 包括处理线程.
    // 线程停止后, 各pipeline stages不会再被调用.
    if (m_pNuiImageDevice)
    {
        m_pNuiImageDevice->UnInit();
        delete m_pNuiImageDevice;
        m_pNuiImageDevice = NULL;
    }
    //TRACE("%s: m_pNuiImageDevice\n", __FUNCTION__);
    return TRUE;

    // shut down algorithm pipelines
    if (m_pPipelineSourceStage)
    {
        m_pPipelineSourceStage->ShutDown();
        delete m_pPipelineSourceStage;
        m_pPipelineSourceStage = NULL;
    }

    if (m_pBGRStage)
    {
        m_pBGRStage->ShutDown();
        delete m_pBGRStage;
        m_pBGRStage = NULL;
    }

    if (m_pExemplarStage)
    {
        m_pExemplarStage->ShutDown();
        delete m_pExemplarStage;
        m_pExemplarStage = NULL;
    }
    
    if (m_pCentroidsStage)
    {
        m_pCentroidsStage->ShutDown();
        delete m_pCentroidsStage;
        m_pCentroidsStage = NULL;
    }

    if (m_pModelFittingStage)
    {
        m_pModelFittingStage->ShutDown();
        delete m_pModelFittingStage;
        m_pModelFittingStage = NULL;
    }

    if (m_pPipelineOutputStage)
    {
        m_pPipelineOutputStage->ShutDown();
        delete m_pPipelineOutputStage;
        m_pPipelineOutputStage = NULL;
    }
}


BOOL SkeletalTracker::_StaticSkeletalTrackingProc()
{
    if (SkeletalTracker::m_pSelf)
    {
        return m_pSelf->SkeletalTrackingProc();
    }
    return FALSE;
}


#define MEASURE_TIME(X)
BOOL SkeletalTracker::SkeletalTrackingProc()
{
    //Util tu("算法耗时");
    const int frameWidth = 320;
    const int frameHeight = 240;
    const USHORT * pDepthFrame = m_pNuiImageDevice->GetDepthMap();;
    const USHORT * pBufferRun = pDepthFrame;
    const USHORT * pBufferEnd = pBufferRun + (320 * 240);

    // STEP2: 检查微软深度图有无PlayerIndex, 最终应该是BGR模块检测
    BOOL bHavePlayer = FALSE;
    pBufferRun = pDepthFrame;
    while ( pBufferRun < pBufferEnd )
    {
        USHORT depth     = *pBufferRun;
        if(depth & 0x0007)
        {
            bHavePlayer = TRUE;
            break;
        }
        ++pBufferRun;
    }
    // 耗时估计:
    // STEP1之前: 12% ~ 13%
    // STEP3之前: 12% ~ 13%
    // STEP4之前: 23% ~ 25%
    // STEP5之前: 66% ~ 70%, STEP4消耗资源太多
    if (bHavePlayer)
    {
        m_pPipelineSourceStage->RunFrame();
        m_pBGRStage->RunFrame();
        m_pExemplarStage->RunFrame();
        m_pCentroidsStage->RunFrame();
        m_pModelFittingStage->RunFrame();
        m_pPipelineOutputStage->RunFrame();
#if 0
        {
            // STEP3: 人体部位分类
            MEASURE_TIME(Util tu("人体部位分类"));
            m_pBPC.ExemplarStage_RunFrame(1.0f, pDepthFrame, m_pLabelRaw, 1, LABEL_DATA);
        }

        {
            // STEP4: 提取部位中心
            MEASURE_TIME(Util tu("提取部位中心"));
            m_pBPC.CentroidsStage_RunFrame(pDepthFrame, m_pLabelRaw, m_centroid);
        }

        {
            // STEP5: 提取人体骨骼
            MEASURE_TIME(Util tu("提取人体骨骼"));
            m_pBPC.ModelFittingStage_RunFrame(m_centroid, m_joints);
        }

        {
            // STEP6: 平滑人体骨骼
            MEASURE_TIME(Util tu("平滑人体骨骼"));
            NUI_TRANSFORM_SMOOTH_PARAMETERS SmoothingParams;
            SmoothingParams.fSmoothing          = 0.5f;
            SmoothingParams.fCorrection         = 0.5f;
            SmoothingParams.fPrediction         = 0.5f;
            SmoothingParams.fJitterRadius       = 0.05f;
            SmoothingParams.fMaxDeviationRadius = 0.04f;
            if (S_OK != m_pBPC.TransformSmooth(m_joints, &SmoothingParams))
            {
                assert(0);
            }
        }
#endif
    }

    return TRUE;
}


}
