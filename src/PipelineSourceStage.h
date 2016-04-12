#pragma once
#include "IPipelineStage.h"
#include "INuiImageDevice.h"

namespace NuiSinect {


/*
    ����: ��ԭʼ���ͼ����ת��Ϊ_NUI_IMAGE_TYPE�ṹ, �ֱ��ʵ���Ϊ320*240.
    ����: ԭʼ�������(640*480)
    ���: _NUI_IMAGE_FRAME
    ��Ա: ��
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

