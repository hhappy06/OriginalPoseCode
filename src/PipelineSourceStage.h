#pragma once
#include "IPipelineStage.h"
#include "INuiImageDevice.h"

namespace NuiSinect {


/*
    功能: 把原始深度图数据转换为_NUI_IMAGE_TYPE结构, 分辨率调整为320*240.
    输入: 原始深度数据(640*480)
    输出: _NUI_IMAGE_FRAME
    成员: 无
*/
class PipelineSourceStage :
    public IPipelineStage
{
public:
    PipelineSourceStage(void);
    ~PipelineSourceStage(void);

    BOOL Initialize();
    BOOL ShutDown();
    int RunFrame();
private:
private:
};

}

