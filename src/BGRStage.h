#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {


/*
    ����: ǰ���ָ�, �����֡������playerIndex.
    ����/���: _NUI_IMAGE_FRAME
    ��Ա: CodebookProcessor, snHumanTrack, cvlabeling_imagelab
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
