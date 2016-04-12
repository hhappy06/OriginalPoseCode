#pragma once
#include "PipelineSourceStage.h"
#include "BGRStage.h"
#include "ExemplarStage.h"
#include "CentroidsStage.h"
#include "ModelFittingStage.h"
#include "PipelineOutputStage.h"

namespace NuiSinect {
/*
    ����: ��ԭʼ���ͼ����ת��Ϊ_NUI_IMAGE_TYPE�ṹ, �ֱ��ʵ���Ϊ320*240.
    ����: ԭʼ�������(640*480)
    ���: _NUI_IMAGE_FRAME
    ��Ա: ��
*/
//class PipelineSourceStage;
/*
    ����: ǰ���ָ�, �����֡������playerIndex.
    ����/���: _NUI_IMAGE_FRAME
    ��Ա: CodebookProcessor, snHumanTrack, cvlabeling_imagelab
*/
//class BGRStage;
/*
    ����: ��ԭʼ���ͼ����ת��Ϊ_NUI_IMAGE_TYPE�ṹ,
        �ֱ��ʵ���Ϊ320*240,
        Set���ͼEvent.
    ����: _NUI_IMAGE_FRAME
    ���: ���岿λ���ʷֲ�(BodyPartDistribution)
    ��Ա: BodyPartLabeller
*/
//class ExemplarStage;
/*
    ����: ��ȡ���岿λ�����ĵ�
    ����: _NUI_IMAGE_FRAME, BodyPartDistribution
    ���: ���岿λ���ĵ�(BodyPartCentroids)
    ��Ա: ��
*/
//class CentroidsStage;
/*
    ����: ��ȡ�������
    ����: _NUI_IMAGE_FRAME, BodyPartCentroids
    ���: Skeletons
    ��Ա: ��
*/
//class ModelFittingStage;
/*
    ����: �γɹǼ�����ṹ,
        Set�Ǽ�Event.
    ����: Skeletons
    ���: ��
    ��Ա: _NUI_SKELETON_FRAME
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
    // �㷨����������(�ص�����)
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

