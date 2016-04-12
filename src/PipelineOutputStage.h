#pragma once
#include "IPipelineStage.h"

namespace NuiSinect {


/*
    ����: �γɹǼ�����ṹ,
        Set�Ǽ�Event.
    ����: Skeletons
    ���: ��
    ��Ա: _NUI_SKELETON_FRAME
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
