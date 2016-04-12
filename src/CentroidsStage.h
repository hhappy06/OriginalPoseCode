#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {


/*
    ����: ��ȡ���岿λ�����ĵ�
    ����: _NUI_IMAGE_FRAME, BodyPartDistribution
    ���: ���岿λ���ĵ�(BodyPartCentroids)
    ��Ա: ��
*/
class CentroidsStage :
    public IPipelineStage
{
public:
    CentroidsStage(void);
    ~CentroidsStage(void);

    BOOL Initialize();
    BOOL ShutDown();
    int RunFrame();
};

}
