#pragma once
#define USE_ASYNCHRONOUS_AGENTS_LAYER1

#ifdef USE_ASYNCHRONOUS_AGENTS_LAYER
#include <agents.h>
//using namespace Concurrency;
#endif//USE_ASYNCHRONOUS_AGENTS_LAYER

#define _IN_
#define _OUT_
#define _IN_OUT_

namespace NuiSinect {



#ifdef USE_ASYNCHRONOUS_AGENTS_LAYER
typedef 
enum _PipeLineCMD
{
    PIPELINE_RUN  = 0,
    PIPELINE_STOP = 1
} PipeLineCMD;

typedef 
struct _PipeLineStageData
{
    _PipeLineStageData() 
        : m_PipelineControl(PIPELINE_RUN)
    {

    }
    PipeLineCMD m_PipelineControl;
} PipeLineStageData;

class IPipelineStage :
    public Concurrency::agent
{
public:
    IPipelineStage(void);
    virtual ~IPipelineStage(void);

    //virtual void Reset() = 0;
    virtual int RunFrame() = 0;
    virtual void Shutdown();

protected:
    virtual void run();
};

#else

class IPipelineStage
{
public:
    IPipelineStage(void);
    virtual ~IPipelineStage(void);

    virtual int RunFrame() = 0;
};

#endif//USE_ASYNCHRONOUS_AGENTS_LAYER

}
