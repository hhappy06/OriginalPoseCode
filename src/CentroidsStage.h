#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {


/*
    功能: 提取身体部位的中心点
    输入: _NUI_IMAGE_FRAME, BodyPartDistribution
    输出: 身体部位中心点(BodyPartCentroids)
    成员: 无
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
