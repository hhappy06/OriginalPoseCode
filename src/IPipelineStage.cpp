#include "stdafx.h"
#include "IPipelineStage.h"
namespace NuiSinect {


IPipelineStage::IPipelineStage(void)
{
}


IPipelineStage::~IPipelineStage(void)
{
}

#ifdef USE_ASYNCHRONOUS_AGENTS_LAYER
void IPipelineStage::Shutdown()
{
    done();
}

void IPipelineStage::run()
{
    while (this->RunFrame())
    {
    }

    this->Shutdown();
}
#endif//USE_ASYNCHRONOUS_AGENTS_LAYER

}
