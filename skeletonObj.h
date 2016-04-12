#ifndef skeletonStruct_H
#define skeletonStruct_H
#include <CL/cl.h>
typedef struct  tagSkeletonObj
{
	void * m_pBPC;
	bool isCLinited;					//是否初始化标识量
	cl_context m_context;
	cl_command_queue m_commandQueue;
	cl_program m_program ;
	cl_kernel m_kernel_rfpredict;
	cl_kernel m_kernel_initCentroid;
	cl_kernel m_kernel_centroidReduction;
	cl_kernel m_kernel_meanshift;
	cl_kernel m_kernel_meanshiftReduction;
	cl_mem  m_cl_tree;
	cl_mem  m_cl_value;
	cl_mem cl_DepthFrame;
	cl_mem cl_LabelRaw;
	cl_mem cl_Label;

	cl_mem cl_iCount;
	cl_mem cl_eBodyPartTrackingState;
	cl_mem cl_eBodyPartPositionTrackingState;
	cl_mem cl_vCentroid;
	cl_mem cl_m_pvCoordWorldSpacet;
	cl_mem cl_m_pfBodyPartProbility;
	cl_mem cl_m_CentroidData;
	//cl_mem cl_iCount;
	cl_device_id        *m_devices;
	unsigned short * m_pDepthFrame;;
	unsigned char  m_pLabel[160*120];
	unsigned short  m_pLabelRaw[160*120*31];
	float    m_pfBodyPartProbility[6 *120 * 160 * 31];
	Vector4  m_pvCoordWorldSpace[120 * 160];
	Vector4 vCentroid[6][31];
#ifdef _DEBUG

	int iCount[6][31];
	 NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31];
	 NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6];
#endif
	//unsigned short *m_mappedLabelRaw;
	Vector4 m_centroid[31];
	Vector4 m_joints[20];
	NUI_CENTROIDS_STAGE_DATA m_CentroidData;
	NUI_SKELETON_FRAME       m_SkeletonData;
	NUI_CENTROIDS_STAGE_DATA m_CentroidDataOfGravity;
	NUI_SKELETON_FRAME       m_SkeletonDataOfGravity;

	double bodyPartClassificationD;
	double centroidD;
	double skeletonD;
	double smoothD;
	double clEnqueueRangKernelTimeD;
	double clEnqueueNDRangeKernelFinishTimeD;
	double rawToLabelTimeD;
	double clEnqueueReadBufferTimeD;
	double clEnqueueFillBufferTimeD;
	double clEnqueueWriteBufferTimeD;
	double clSetKernelArgTimeD;
	double cpuPredictRawParallelTimeD;
	double cpuRawToLabelTimeD;

	double cpu_meanshiftTimed;
	double gpuReadCentrpodTimeD;
	double gpuMeanshiftTimeD;

	double initCentroid ;
	double calcCentroid ;
	double initCentroidMem ;

	double gpuInitCentroidTimeD;
	bool m_useGpu;					//是否使用GPU计算部位分类的标识量
	bool doMeanshift;
	bool GrayGravityCentroid;
	//bool m_GpuUsed;
	////////////////////////////////
//	bool g_UseGPU ;
	bool g_actionBlobBodyPart;
	bool NoHugeArray;				//什么作用？
}SkeletonObj;

#endif