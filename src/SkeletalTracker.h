#pragma once
#include "PipelineSourceStage.h"
#include "BGRStage.h"
#include "ExemplarStage.h"
#include "CentroidsStage.h"
#include "ModelFittingStage.h"
#include "PipelineOutputStage.h"

namespace NuiSinect {
/*
    功能: 把原始深度图数据转换为_NUI_IMAGE_TYPE结构, 分辨率调整为320*240.
    输入: 原始深度数据(640*480)
    输出: _NUI_IMAGE_FRAME
    成员: 无
*/
//class PipelineSourceStage;
/*
    功能: 前景分割, 在深度帧上增加playerIndex.
    输入/输出: _NUI_IMAGE_FRAME
    成员: CodebookProcessor, snHumanTrack, cvlabeling_imagelab
*/
//class BGRStage;
/*
    功能: 把原始深度图数据转换为_NUI_IMAGE_TYPE结构,
        分辨率调整为320*240,
        Set深度图Event.
    输入: _NUI_IMAGE_FRAME
    输出: 身体部位概率分布(BodyPartDistribution)
    成员: BodyPartLabeller
*/
//class ExemplarStage;
/*
    功能: 提取身体部位的中心点
    输入: _NUI_IMAGE_FRAME, BodyPartDistribution
    输出: 身体部位中心点(BodyPartCentroids)
    成员: 无
*/
//class CentroidsStage;
/*
    功能: 提取人体骨骼
    输入: _NUI_IMAGE_FRAME, BodyPartCentroids
    输出: Skeletons
    成员: 无
*/
//class ModelFittingStage;
/*
    功能: 形成骨架输出结构,
        Set骨架Event.
    输入: Skeletons
    输出: 无
    成员: _NUI_SKELETON_FRAME
*/
//class PipelineOutputStage;


class SkeletalTracker
{
public:

    SkeletalTracker(void);
    ~SkeletalTracker(void);

    BOOL Initialize();
    BOOL ShutDown();

private:
    // 算法处理主函数(回调函数)
    static BOOL _StaticSkeletalTrackingProc();

    BOOL SkeletalTrackingProc();
    //BOOL RunFrame();
private:
    // Self
    static SkeletalTracker *  m_pSelf;

    // NuiImageDevice
    INuiImageDevice     * m_pNuiImageDevice;

    // algorithm pipeline
    PipelineSourceStage * m_pPipelineSourceStage;
    BGRStage            * m_pBGRStage;
    ExemplarStage       * m_pExemplarStage;
    CentroidsStage      * m_pCentroidsStage;
    ModelFittingStage   * m_pModelFittingStage;
    PipelineOutputStage * m_pPipelineOutputStage;

    // random forest
    //RandomForest        * m_pForest;

};

}

