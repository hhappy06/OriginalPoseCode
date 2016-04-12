#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {

/*
    功能: 提取人体骨骼
    输入: _NUI_IMAGE_FRAME, BodyPartCentroids
    输出: Skeletons
    成员: 无
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
