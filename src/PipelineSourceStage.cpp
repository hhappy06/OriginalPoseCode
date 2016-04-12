#include "stdafx.h"
#include "PipelineSourceStage.h"


namespace NuiSinect {


PipelineSourceStage::PipelineSourceStage()
{
}


PipelineSourceStage::~PipelineSourceStage(void)
{
}


BOOL PipelineSourceStage::Initialize()
{
    return TRUE;
}

BOOL PipelineSourceStage::ShutDown()
{
    return TRUE;
}

int PipelineSourceStage::RunFrame()
{
    
    return 0;
}


}