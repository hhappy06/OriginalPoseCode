#include "stdafx.h"
#include "ExemplarStage.h"


namespace NuiSinect {


ExemplarStage::ExemplarStage(void)
{
}


ExemplarStage::~ExemplarStage(void)
{
}


BOOL ExemplarStage::Initialize(_IN_ const signed char *pszModelFilePath)
{
    return TRUE;
}


BOOL ExemplarStage::ShutDown()
{
    return TRUE;
}


BOOL ExemplarStage::RunFrame(/*_IN_ const NUI_IMAGE_FRAME *pImageFrame, _OUT_ BodyPartProbility *pBodyPartProbility*/)
{

    return 0;
}

}
