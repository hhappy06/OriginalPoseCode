#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {

/*
    ����: ��ȡ�������
    ����: _NUI_IMAGE_FRAME, BodyPartCentroids
    ���: Skeletons
    ��Ա: ��
*/
class ModelFittingStage :
    public IPipelineStage
{
public:
    ModelFittingStage(void);
    ~ModelFittingStage(void);

    BOOL Initialize();
    BOOL ShutDown();
    int RunFrame();
};


}
