#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {


/*
    功能: 形成骨架输出结构,
        Set骨架Event.
    输入: Skeletons
    输出: 无
    成员: _NUI_SKELETON_FRAME
*/
class PipelineOutputStage :
    public IPipelineStage
{
public:
    PipelineOutputStage(void);
    ~PipelineOutputStage(void);

    BOOL Initialize();
    BOOL ShutDown();
    int RunFrame();
};

}
