#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {


/*
    功能: 前景分割, 在深度帧上增加playerIndex.
    输入/输出: _NUI_IMAGE_FRAME
    成员: CodebookProcessor, snHumanTrack, cvlabeling_imagelab
*/
class BGRStage :
    public IPipelineStage
{
public:
    BGRStage(void);
    ~BGRStage(void);

    BOOL Initialize();
    BOOL ShutDown();
    int RunFrame();
};

}
