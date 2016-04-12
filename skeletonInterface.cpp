#include "stdafx.h"
//#include <CL/cl.h>
#include "NuiSinect.h"
#include "RandomForest.h"
#include "BodyPartClassifier.h"
#include "skeletonInterface.h"

#include "skeletonObj.h"
#include "opencl.h"
#include "clUtil.h"

//bool s_UseGPU;
extern Vector4 g_vInitedCentroid_copy[6][31];
extern double g_cpuPredictRawParallelTimeD ;
extern double g_cpuRawToLabelTimeD;
extern double g_initCentroid ;
extern double g_calcCentroid ;
extern double g_initCentroidMem ;
extern bool g_NoHugeArray;
extern bool g_doMeanshift;
extern bool g_GrayGravityCentroid;
//#ifdef WIN32
//int parallelism_enabled ;
//#else
//extern int parallelism_enabled ;
//#endif

//bool NoBodyPartProbility;


//void skeletonSetNoBodyPartProbility(bool enabled)
//{
//	NoBodyPartProbility = enabled;
//}


//void skeletonSetOpenmp(bool enabled)
//{
//	parallelism_enabled = enabled;
//}
 int g_folder;
 int g_framecnt;
void setFolderFrame(int folder, int framecnt)
{
	g_folder = folder;
	g_framecnt = framecnt;
}
void setGrayGravityCentroid(void * aa, bool GrayGravityCentroid)
{
	SkeletonObj * a_this = (SkeletonObj * )aa;
	a_this->GrayGravityCentroid = GrayGravityCentroid;
}
void setDoMeanshift(void * aa, bool doMeanshift)
{
	SkeletonObj * a_this = (SkeletonObj * )aa;
	a_this->doMeanshift = doMeanshift;
}


void setSkeletonUseGpu(void * aa, bool useGpu)
{
	SkeletonObj * a_this = (SkeletonObj * )aa;
	a_this->m_useGpu = useGpu;
}

void setSkeletonUseHugeArray(void * aa, bool useHugeArray)
{
	SkeletonObj * a_this = (SkeletonObj * )aa;
	a_this->NoHugeArray = useHugeArray;

}

void initSkeletonObj(void **aathis, bool UseGpu) 
{
	SkeletonObj ** ppthis =  (SkeletonObj **)aathis;
	SkeletonObj * a_this = new SkeletonObj;
	*aathis = a_this;
	a_this->m_pBPC = new BodyPartClassifier();
	//a_this->m_pBPC = new BodyPartClassifier(a_this->m_context);
	assert(a_this->m_pBPC);
	if (!((BodyPartClassifier * )a_this->m_pBPC)->BuildForestFromResource())
	{
		assert(0);
		//AfxMessageBox("模型文件加载失败!");
	}
	else
	{
		//cl_CreateBuffers(a_this);
	}
	clearLabelRaw(a_this);
	//	a_this->m_context = NULL;
	a_this->m_useGpu = UseGpu;
	a_this->isCLinited = false;
	a_this->NoHugeArray = false;
	a_this->GrayGravityCentroid = false;
	g_NoHugeArray = a_this->NoHugeArray;
	if (UseGpu)
	{
		initCL(a_this);
	}



}

void releaseSkeletonObj(void * aathis)
{
	SkeletonObj * a_this  = (SkeletonObj*)  aathis;
	if (a_this->isCLinited)
	{
		ReleaseCL(a_this);
	}

	delete (BodyPartClassifier*)a_this->m_pBPC;
	delete a_this;
}
void getvCentroidInterface(void * a_this, Vector4 Centroid[6][31])
{
	memcpy(Centroid, g_vInitedCentroid_copy, sizeof(g_vInitedCentroid_copy));
}
/**
*@xu-li:
*函数名称：calcSkeletonFromForeground
*函数描述：
*输入参数：1.骨架对象void *aathis； 2.前景数据unsigned short * depthDataWithIndex；
*输出参数：1.骨架节点NUI_SKELETON_FRAME *SinectSkeletons
*返回值：成功1；
*/
int calcSkeletonFromForeground(void *aathis ,unsigned short * depthDataWithIndex, NUI_SKELETON_FRAME *SinectSkeletons, int file_id )
{
	SkeletonObj * a_this = (SkeletonObj *)aathis;
	a_this->m_pDepthFrame = depthDataWithIndex;			//@xu-li:深度图？
	//unsigned short *  pLabelRaw;
	g_NoHugeArray = a_this->NoHugeArray;				//@xu-li:什么判断标识量？
	g_doMeanshift = a_this->doMeanshift;				//@xu-li:是否使用meanshift的标识位
	g_GrayGravityCentroid = a_this->GrayGravityCentroid;//@xu-li:是否使用灰度重心法标识量
	{
		//	cl_int ret;
		// STEP3: 人体部位分类

		if (a_this->m_useGpu)		//@xu-li:判断是否使用GPU计算
		{
			if (!a_this->isCLinited)
			{
				initCL(aathis);		//@xu-li:初始化OpenCL设备
			}
			{
				MEASURE_TIME(Util tu("Nui_GotDepthAlert: bodyPartClassification"));
				bodyPartGpu(a_this, depthDataWithIndex);							//@xu-li:使用GPU做OpenCL计算
				MEASURE_TIME(a_this->bodyPartClassificationD = tu.time());
			}
			//{
			//	MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
			//	Centroid(a_this, depthDataWithIndex);
			//	
			//	MEASURE_TIME(a_this->centroidD = tu.time());
			//}
			//{
			//	MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
			//	((BodyPartClassifier * )a_this->m_pBPC)->meanshift(a_this->vCentroid, a_this->eBodyPartPositionTrackingState,
			//		a_this->eBodyPartTrackingState, &a_this->m_CentroidData, a_this->iCount, a_this->m_pvCoordWorldSpace, a_this->m_pfBodyPartProbility);
			//	MEASURE_TIME(a_this->cpu_meanshiftTimed = tu.time());
			//}

		} 
		else//@xu-li:使用CPU做部位分类？
		{
			{
				MEASURE_TIME(Util tu("Nui_GotDepthAlert: bodyPartClassification"));
				((BodyPartClassifier * )a_this->m_pBPC)->ExemplarStage_RunFrame(1.0f, depthDataWithIndex, a_this->m_pLabelRaw, 1, a_this->m_pLabel, a_this->m_useGpu);
				a_this->cpuPredictRawParallelTimeD = g_cpuPredictRawParallelTimeD;
				a_this->cpuRawToLabelTimeD = g_cpuRawToLabelTimeD;


				MEASURE_TIME(a_this->bodyPartClassificationD = tu.time());
				
				//////////////////////////////////////////////////////////////////////////
				//@xu-li添加代码为保存部位
				char file_name[100];
				sprintf_s(file_name, 100, "D:/Task/image/Depth_Two_%05d.label_raw_gpu", file_id);	
				out_to_file(file_name, &a_this->m_pLabelRaw[0]);
				//////////////////////////////////////////////////////////////////////////
			}
		}
		{
			// STEP4: 提取部位中心
			MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
			g_GrayGravityCentroid = false;									//@xu-li:是否使用“灰度重心法”标识量，false不使用，并未用到；
			((BodyPartClassifier * )a_this->m_pBPC)->CentroidsStage_RunFrame(depthDataWithIndex, a_this->m_pLabelRaw, &a_this->m_CentroidData,
				a_this->m_pvCoordWorldSpace, a_this->m_pfBodyPartProbility);//@xu-li:m_pfBodyPartProbility每个像素点对所有6人31部位概率概率的首地址
			a_this->initCentroidMem = g_initCentroidMem;					//@xu-li:这三个参数的作用
			a_this->initCentroid = g_initCentroid;
			a_this->calcCentroid = g_calcCentroid;
			MEASURE_TIME(a_this->centroidD = tu.time());
		}


		{
			// STEP5: 提取人体骨骼
			MEASURE_TIME(Util tu("Nui_GotDepthAlert: skeleton"));
			//((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(m_centroid, m_joints);
			((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(&a_this->m_CentroidData, &a_this->m_SkeletonData);
			MEASURE_TIME(a_this->skeletonD = tu.time());
		}

		


		//@xu-li:什么参数？？
		NUI_TRANSFORM_SMOOTH_PARAMETERS SmoothingParams;
		SmoothingParams.fSmoothing          = 0.8f; //0.5f
		SmoothingParams.fCorrection         = 0.5f; //0.5f
		SmoothingParams.fPrediction         = 0.5f; //0.5f
		SmoothingParams.fJitterRadius       = 0.05f; //0.05f
		SmoothingParams.fMaxDeviationRadius = 0.04f; //0.04f


		{
			// STEP6: 平滑人体骨骼
			MEASURE_TIME(Util tu("Nui_GotDepthAlert: smooth"));

			if (S_OK != ((BodyPartClassifier * )a_this->m_pBPC)->NuiTransformSmooth(&a_this->m_SkeletonData, &SmoothingParams))
			{
				assert(0);
			}
			*SinectSkeletons = a_this->m_SkeletonData;	//@xu-li:参数作用
			MEASURE_TIME(a_this->smoothD = tu.time());

		}
		g_GrayGravityCentroid = true;					//@xu-li:以下使用灰度重心法，重新计算中心点并提取骨架
		((BodyPartClassifier * )a_this->m_pBPC)->CentroidsStage_RunFrame(depthDataWithIndex, a_this->m_pLabelRaw, &a_this->m_CentroidDataOfGravity,
			a_this->m_pvCoordWorldSpace, a_this->m_pfBodyPartProbility);
		((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(&a_this->m_CentroidDataOfGravity, &a_this->m_SkeletonDataOfGravity);
		if (S_OK != ((BodyPartClassifier * )a_this->m_pBPC)->NuiTransformSmooth(&a_this->m_SkeletonDataOfGravity, &SmoothingParams))
		{
			assert(0);
		}
	}
	return 1;
}
//@xu-li
int calcSkeletonFromForeground_sf(void *aathis ,unsigned short * depthDataWithIndex, NUI_SKELETON_FRAME *SinectSkeletons, NUI_SKELETON_FRAME *SinectSkeletons_sf, int file_id )
{
	SkeletonObj * a_this = (SkeletonObj *)aathis;
	NUI_SKELETON_FRAME  SkeletonData_sf = {0};
	a_this->m_pDepthFrame = depthDataWithIndex;			//@xu-li:深度图？
	//unsigned short *  pLabelRaw;
	g_NoHugeArray = a_this->NoHugeArray;				//@xu-li:什么判断标识量？
	g_doMeanshift = a_this->doMeanshift;				//@xu-li:是否使用meanshift的标识位
	g_GrayGravityCentroid = a_this->GrayGravityCentroid;//@xu-li:是否使用灰度重心法标识量
	{
		//	cl_int ret;
		// STEP3: 人体部位分类

		if (a_this->m_useGpu)		//@xu-li:判断是否使用GPU计算
		{
			if (!a_this->isCLinited)
			{
				initCL(aathis);		//@xu-li:初始化OpenCL设备
			}
			{
				MEASURE_TIME(Util tu("Nui_GotDepthAlert: bodyPartClassification"));
				bodyPartGpu(a_this, depthDataWithIndex);							//@xu-li:使用GPU做OpenCL计算
				MEASURE_TIME(a_this->bodyPartClassificationD = tu.time());
			}
			//{
			//	MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
			//	Centroid(a_this, depthDataWithIndex);
			//	
			//	MEASURE_TIME(a_this->centroidD = tu.time());
			//}
			//{
			//	MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
			//	((BodyPartClassifier * )a_this->m_pBPC)->meanshift(a_this->vCentroid, a_this->eBodyPartPositionTrackingState,
			//		a_this->eBodyPartTrackingState, &a_this->m_CentroidData, a_this->iCount, a_this->m_pvCoordWorldSpace, a_this->m_pfBodyPartProbility);
			//	MEASURE_TIME(a_this->cpu_meanshiftTimed = tu.time());
			//}

		} 
		else//@xu-li:使用CPU做部位分类？
		{
			{
				MEASURE_TIME(Util tu("Nui_GotDepthAlert: bodyPartClassification"));
				((BodyPartClassifier * )a_this->m_pBPC)->ExemplarStage_RunFrame(1.0f, depthDataWithIndex, a_this->m_pLabelRaw, 1, a_this->m_pLabel, a_this->m_useGpu);
				a_this->cpuPredictRawParallelTimeD = g_cpuPredictRawParallelTimeD;
				a_this->cpuRawToLabelTimeD = g_cpuRawToLabelTimeD;


				MEASURE_TIME(a_this->bodyPartClassificationD = tu.time());
				
				//////////////////////////////////////////////////////////////////////////
				//@xu-li添加代码为保存部位
				char file_name[100];
				sprintf_s(file_name, 100, "D:/Task/image/Depth_Two_%05d.label_raw_gpu", file_id);	
				out_to_file(file_name, &a_this->m_pLabelRaw[0]);
				//////////////////////////////////////////////////////////////////////////
			}
		}
		{
			// STEP4: 提取部位中心
			MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
			g_GrayGravityCentroid = false;									//@xu-li:是否使用“灰度重心法”标识量，false不使用，并未用到；
			((BodyPartClassifier * )a_this->m_pBPC)->CentroidsStage_RunFrame(depthDataWithIndex, a_this->m_pLabelRaw, &a_this->m_CentroidData,
				a_this->m_pvCoordWorldSpace, a_this->m_pfBodyPartProbility);//@xu-li:m_pfBodyPartProbility每个像素点对所有6人31部位概率概率的首地址
			a_this->initCentroidMem = g_initCentroidMem;					//@xu-li:这三个参数的作用
			a_this->initCentroid = g_initCentroid;
			a_this->calcCentroid = g_calcCentroid;
			MEASURE_TIME(a_this->centroidD = tu.time());
		}


		{
			// STEP5: 提取人体骨骼
			MEASURE_TIME(Util tu("Nui_GotDepthAlert: skeleton"));
			//((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(m_centroid, m_joints);
			
			//((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(&a_this->m_CentroidData, &a_this->m_SkeletonData);//@xu-li
			
			((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(&a_this->m_CentroidData, &a_this->m_SkeletonData, &SkeletonData_sf);
			MEASURE_TIME(a_this->skeletonD = tu.time());
		}

		


		//@xu-li:什么参数？？
		NUI_TRANSFORM_SMOOTH_PARAMETERS SmoothingParams;
		SmoothingParams.fSmoothing          = 0.8f; //0.5f
		SmoothingParams.fCorrection         = 0.5f; //0.5f
		SmoothingParams.fPrediction         = 0.5f; //0.5f
		SmoothingParams.fJitterRadius       = 0.05f; //0.05f
		SmoothingParams.fMaxDeviationRadius = 0.04f; //0.04f


		{
			// STEP6: 平滑人体骨骼
			MEASURE_TIME(Util tu("Nui_GotDepthAlert: smooth"));
			
			//if (S_OK != ((BodyPartClassifier * )a_this->m_pBPC)->NuiTransformSmooth(&a_this->m_SkeletonData, &SmoothingParams))//@xu-li
			//{
			//	assert(0);
			//}
			*SinectSkeletons_sf = SkeletonData_sf;
			*SinectSkeletons = a_this->m_SkeletonData;	//@xu-li:参数作用
			MEASURE_TIME(a_this->smoothD = tu.time());

		}
		g_GrayGravityCentroid = true;					//@xu-li:以下使用灰度重心法，重新计算中心点并提取骨架
		((BodyPartClassifier * )a_this->m_pBPC)->CentroidsStage_RunFrame(depthDataWithIndex, a_this->m_pLabelRaw, &a_this->m_CentroidDataOfGravity,
			a_this->m_pvCoordWorldSpace, a_this->m_pfBodyPartProbility);
		((BodyPartClassifier * )a_this->m_pBPC)->ModelFittingStage_RunFrame(&a_this->m_CentroidDataOfGravity, &a_this->m_SkeletonDataOfGravity);
		if (S_OK != ((BodyPartClassifier * )a_this->m_pBPC)->NuiTransformSmooth(&a_this->m_SkeletonDataOfGravity, &SmoothingParams))
		{
			assert(0);
		}
	}
	return 1;
}


void ClassifyOnePixelInterface(void * aathis, const unsigned short * pImage, int y, int x, const float player_scale, const int DepthUnusual, int * maxclass)
{
	SkeletonObj * a_this = (SkeletonObj *)aathis;
	((BodyPartClassifier * )a_this->m_pBPC)->classifyOnePixelClass(pImage, y, x, player_scale, DepthUnusual, maxclass, a_this->m_pLabelRaw);
}

BASEAPI void rawToLabelinterface(void * aathis, unsigned char * pLabel, const unsigned short * pImage, int labelConvMode, unsigned short * pLabelRaw )
{
	SkeletonObj * a_this = (SkeletonObj *)aathis;
	((BodyPartClassifier * )a_this->m_pBPC)->rawToLabel(pLabel, pImage, 1, pLabelRaw);
}
//
//void clearLabelRaw( void * a_pSkObj )
//{
//	SkeletonObj * a_this = (SkeletonObj *)a_pSkObj;
//	memset(a_this->m_pLabelRaw, 0, 160 * 120 * 31 * sizeof(unsigned short));
//}

void clearLabelRaw( void * a_pSkObj )
{
	SkeletonObj * a_this = (SkeletonObj *)a_pSkObj;
	//cl_int ret;

	//unsigned short *plabelRaw = (unsigned short *)clEnqueueMapBuffer(a_this->m_commandQueue,  a_this->cl_LabelRaw, CL_TRUE, CL_MAP_WRITE, 0, 
	//	160 * 120 * 31 * sizeof(unsigned short), 0, NULL, NULL, &ret);
	//clCHECKERROR(ret, CL_SUCCESS);

	memset(a_this->m_pLabelRaw, 0, 160 * 120 * 31 * sizeof(unsigned short));
	//	ret = clEnqueueUnmapMemObject(a_this->m_commandQueue, a_this->cl_LabelRaw, plabelRaw, 0, NULL, NULL);
	//	clCHECKERROR(ret, CL_SUCCESS);
}


//@xu-li添加函数保持部位
void get_human_part(void *aathis,  unsigned char * out_part)
{
	SkeletonObj * a_this = (SkeletonObj *)aathis;
	out_part = (unsigned char*)a_this->m_pLabelRaw;
	int i = 1;
}
bool out_to_file(const char * afilename, unsigned short * aout)
{
	int DATA_SIZE = 0;
    DATA_SIZE = 160*120*31*2;
	unsigned char* aaout = (unsigned char*)aout;
    FILE * fp;
    fopen_s(&fp, afilename, "wb");
    if (!fp)
    {
        return false;
    }

    if (fwrite(aaout, 1, DATA_SIZE, fp) != DATA_SIZE)
    {
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}
