#include <QFile>
#include <QDataStream>
#include "stdafx.h"

#include <assert.h>
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include "NuiSinect.h"
//#include <CL/cl.h>
#include "skeletonObj.h"
//#include "const.h"
#include "buildForest.h"
//#include "microsoftData.h"
#include "skeletonInterface.h"
//#include "drawSkeletons.h"
//
//
//#include "depth2color.h"
using namespace std;
//using namespace cv;
//#include "buildForest.h"




#include "clUtil.h"

#include "opencl.h"



extern Vector4 g_vInitedCentroid_copy[6][31];

/* convert the kernel file into a string */
int convertToString(std::string& s)
{
    char*  str;
    QFile file(":/model/res/rfpredict.cl");
    QDataStream in(&file);
    if(file.open(QIODevice::ReadOnly))
	{
		size_t fileSize;
        str = new  char [100 * 1024];
        if(!str)
        {
            file.close();
            return -2;
        }
        fileSize = in.readRawData(str, 100*1024);
        assert(fileSize < 100 * 1024);
        file.close();
        str[fileSize] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
    //cout<<"Error: failed to open file\n:"<<filename<<endl;
    assert(0);
	return -1;
}

double cl_get_run_time(cl_event *thisEvent, const char *func_name)
{
	cl_int iErrNum;
	cl_ulong iStartTime = 0; 
	cl_ulong iEndTime = 0;

	iErrNum = clGetEventProfilingInfo(*thisEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &iStartTime, NULL);
	clCHECKERROR(iErrNum, CL_SUCCESS);
	iErrNum = clGetEventProfilingInfo(*thisEvent, CL_PROFILING_COMMAND_END,   sizeof(cl_ulong), &iEndTime, NULL);
	clCHECKERROR(iErrNum, CL_SUCCESS);

	//printf("This %s func run time is %5f ms \n", func_name, (iEndTime - iStartTime)*1e-6f );
	return (iEndTime - iStartTime)*1e-6f;

}

void createKernels( SkeletonObj * a_this, cl_program program, cl_int iErrNum );

int initCLDeviceAndKernel(SkeletonObj * a_this)
{

	cl_int  ret;
	cl_int iErrNum;
	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;//the NO. of platforms
	cl_platform_id platform = NULL;//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout<<"Error: Getting platforms!"<<endl;
		return 1;
	}
	cout << " numPlatforms:" << numPlatforms <<endl;
	/*For clarity, choose the first available platform. */
	if(numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint	            numDevices = 0;
	//cl_device_id        *m_devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);	
	cout << " numDevices of gpu" << numDevices <<endl;	
	if (numDevices == 0) //no GPU available.
	{
		cout << "No GPU device available."<<endl;
		return 1;
	}
	else
	{
		a_this->m_devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));

		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, a_this->m_devices, NULL);
	}

	/*Step 3: Create context.*/
	a_this->m_context = clCreateContext(NULL,1, a_this->m_devices,NULL,NULL, &ret);
	clCHECKERROR(ret, CL_SUCCESS);	
	/*Step 4: Creating command queue associate with the context.*/
	cl_command_queue_properties iCommandQueueProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE|CL_QUEUE_PROFILING_ENABLE;
	a_this->m_commandQueue = clCreateCommandQueue(a_this->m_context, a_this->m_devices[0], iCommandQueueProperties, &iErrNum);
	clCHECKERROR(iErrNum, CL_SUCCESS);

	/*Step 5: Create program object */
//	const char *filename = "rfpredict.cl";
	string sourceStr;
    status = convertToString(sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = {strlen(source)};
	a_this->m_program = clCreateProgramWithSource(a_this->m_context, 1, &source, sourceSize, &ret);
	clCHECKERROR(ret, CL_SUCCESS);
	if (a_this->m_program == NULL)
	{
		cerr << "Failed to create CL program from source." << endl;
		cerr <<"clCreateProgramWithSource Error No:"<<ret;
		getchar();
		return 1;
	}

	/*Step 6: Build program. */
	//const char *complier_option = "-cl-mad-enable -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math";
	status=clBuildProgram(a_this->m_program, 1, a_this->m_devices, NULL, NULL, NULL);


	//status=clBuildProgram(program, 1,devices,NULL,NULL,NULL);
	if (status != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(a_this->m_program, a_this->m_devices[0], CL_PROGRAM_BUILD_LOG,
			sizeof(buildLog), buildLog, NULL);
		cerr << "Error in kernel: " << endl;
		cerr << buildLog;
		clReleaseProgram(a_this->m_program);
		return 1;
	}


	/*Step 8: Create kernel object */



	///*Step 9: Sets m_kernel arguments.*/
	//status = clSetKernelArg(a_this->m_kernel, 0, sizeof(cl_mem), (void *)&a_this->m_pBPC->m_forest.m_cl_tree);
	////	printf("status = %d", status);
	//clCHECKERROR(status, CL_SUCCESS);
	//status = clSetKernelArg(a_this->m_kernel, 1, sizeof(cl_mem), (void *)&a_this->m_pBPC->m_forest.m_cl_value);
	//clCHECKERROR(status, CL_SUCCESS);
	return CL_SUCCESS;
}

int cl_CreateBuffers(SkeletonObj * a_this)
{
	cl_int  ret;
	cl_int iErrNum;
	cl_int	status;
	//a_this->m_cl_tree = clCreateBuffer(a_this->m_context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR , 
	//	sizeof(TreeNode) * TREENODECNT * 3,   NULL, &iErrNum);
	//clCHECKERROR(iErrNum, CL_SUCCESS);

	//a_this->m_cl_value = clCreateBuffer(a_this->m_context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR ,
	//	sizeof(NodeValue) * VALUE_COUNT,    NULL, &iErrNum);
	//clCHECKERROR(iErrNum, CL_SUCCESS);

	//TreeNode *t = (TreeNode *)clEnqueueMapBuffer(a_this->m_commandQueue,a_this-> m_cl_tree, CL_TRUE, CL_MAP_WRITE, 0, 
	//	sizeof(TreeNode) * TREENODECNT * 3, 0, NULL, NULL, &ret);
	//clCHECKERROR(ret, CL_SUCCESS);
	////for (int i = 0; i < TREE_COUNT; i++)
	////{
	////	memcpy(t + i * TREENODECNT, ((BodyPartClassifier * )a_this->m_pBPC)->m_forest.m_ppTree[i], TREENODECNT * sizeof(TreeNode));
	////}

	//NodeValue * pValue = (NodeValue *)clEnqueueMapBuffer(a_this->m_commandQueue, a_this->m_cl_value, CL_TRUE, CL_MAP_WRITE, 0, 
	//	sizeof(NodeValue) * VALUE_COUNT, 0, NULL, NULL, &ret);
	////memcpy(pValue, ((BodyPartClassifier * )a_this->m_pBPC)->m_forest.pValue, VALUE_COUNT * sizeof(NodeValue));

	//clCHECKERROR(ret, CL_SUCCESS);
	//ret = clEnqueueUnmapMemObject(a_this->m_commandQueue, a_this->m_cl_tree, t, 0, NULL, NULL);
	//clCHECKERROR(ret, CL_SUCCESS);
	//ret = clEnqueueUnmapMemObject(a_this->m_commandQueue, a_this->m_cl_value, pValue, 0, NULL, NULL);
	//clCHECKERROR(ret, CL_SUCCESS);

	a_this->cl_DepthFrame = clCreateBuffer(a_this->m_context, CL_MEM_READ_ONLY/*|CL_MEM_ALLOC_HOST_PTR */, 
		320 * 240 * 2,    NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);

	a_this->cl_LabelRaw = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		160*120*31* sizeof(short),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);


	a_this->cl_Label = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		160*120,   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);

	a_this->cl_iCount = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		6*31*sizeof(int),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);
	a_this->cl_eBodyPartTrackingState = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		6*sizeof(NUI_BODY_PART_TRACKING_STATE),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);
	a_this->cl_eBodyPartPositionTrackingState = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		6*31*sizeof(NUI_BODY_PART_POSITION_TRACKING_STATE),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);
	a_this->cl_vCentroid = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		6*31*sizeof(Vector4),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);
	a_this->cl_m_pvCoordWorldSpacet = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		120*160*sizeof(Vector4),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);
	a_this->cl_m_pfBodyPartProbility = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		6*31*120*160*sizeof(float),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);

	a_this->cl_m_CentroidData = clCreateBuffer(a_this->m_context, CL_MEM_READ_WRITE/*|CL_MEM_ALLOC_HOST_PTR */, 
		sizeof(NUI_CENTROIDS_STAGE_DATA),   NULL, &status);
	clCHECKERROR(status, CL_SUCCESS);
	char c = 0x00;
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_m_CentroidData, &c, 1, 0,  sizeof(NUI_CENTROIDS_STAGE_DATA), 0, NULL, NULL);
	return status;
}
int releaseBuffers(SkeletonObj * a_this)
{
	cl_int status;
	status = clReleaseMemObject(a_this->cl_DepthFrame);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_LabelRaw);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_Label);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_iCount);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_eBodyPartTrackingState);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_eBodyPartPositionTrackingState);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_vCentroid);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_m_pvCoordWorldSpacet);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_m_pfBodyPartProbility);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseMemObject(a_this->cl_m_CentroidData);//Release mem object.
	clCHECKERROR(status, CL_SUCCESS);
	return status;
}
int initCL(void * aathis)
{
	cl_int ret;
	SkeletonObj * a_this =  (SkeletonObj *)aathis;
	initCLDeviceAndKernel(a_this);
	BuildForestFromResource(a_this);
	createKernels(a_this, a_this->m_program, ret);
	cl_CreateBuffers(a_this);
	a_this->isCLinited = true;
	return true;
}
int bodyPartGpu(void * aathis, unsigned short * depthDataWithIndex )
{
	SkeletonObj * a_this =  (SkeletonObj *)aathis;
	cl_int status;
	cl_int ret;
	cl_event iThisEvent;
	{
		MEASURE_TIME(Util tu("rawToLabel"));
        //clearLabelRaw(aathis);

		/*
		unsigned short *pimage = (unsigned short *)clEnqueueMapBuffer(a_this->m_commandQueue,  a_this->cl_DepthFrame, CL_TRUE, CL_MAP_WRITE, 0, 
		320 * 240 * 2, 0, NULL, NULL, &ret);
		clCHECKERROR(ret, CL_SUCCESS);
		memcpy(pimage, depthDataWithIndex, 320 * 240 * 2);
		ret = clEnqueueUnmapMemObject(a_this->m_commandQueue, a_this->cl_DepthFrame, pimage, 0, NULL, NULL);
		*/


//		char c = 0x00;
//		status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_LabelRaw, &c, 1, 0,  sizeof(a_this->m_pLabelRaw), 0, NULL, NULL);
//		int k = sizeof(a_this->m_pLabelRaw);
//		clCHECKERROR(status, CL_SUCCESS);
//		int k1 = sizeof(a_this->m_pLabel);
//		status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_Label, &c, 1, 0,  sizeof(a_this->m_pLabel), 0, NULL, NULL);
//		clCHECKERROR(status, CL_SUCCESS);
		MEASURE_TIME(a_this->clEnqueueFillBufferTimeD = tu.time());
	}


	{
		MEASURE_TIME(Util tu("rawToLabel"));
		status = clEnqueueWriteBuffer(a_this->m_commandQueue, a_this->cl_DepthFrame, CL_TRUE, 0, 320 * 240 * 2, depthDataWithIndex, 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS);
		MEASURE_TIME(a_this->clEnqueueWriteBufferTimeD  = tu.time());
	}


	{
		MEASURE_TIME(Util tu("rawToLabel"));
		status = clSetKernelArg(a_this->m_kernel_rfpredict, 0, sizeof(cl_mem), (void *)&a_this->m_cl_tree);
		// printf("status = %d", status);
		clCHECKERROR(status, CL_SUCCESS);
		status = clSetKernelArg(a_this->m_kernel_rfpredict, 1, sizeof(cl_mem), (void *)&a_this->m_cl_value);
		clCHECKERROR(status, CL_SUCCESS);
		status = clSetKernelArg(a_this->m_kernel_rfpredict, 2, sizeof(cl_mem), (void *)&a_this->cl_DepthFrame);
		clCHECKERROR(status, CL_SUCCESS);
		status = clSetKernelArg(a_this->m_kernel_rfpredict, 3, sizeof(cl_mem), (void *)&a_this->cl_LabelRaw);
		clCHECKERROR(status, CL_SUCCESS);
		status = clSetKernelArg(a_this->m_kernel_rfpredict, 4, sizeof(cl_mem), (void *)&a_this->cl_Label);
		clCHECKERROR(status, CL_SUCCESS);

		MEASURE_TIME(a_this->clSetKernelArgTimeD = tu.time());
	}

	{
		MEASURE_TIME(Util tu("rawToLabel"));
		size_t global_work_size[2] = {120 , 160 };
		//size_t local_work_size[3]  = {OUT_ROWS/20, OUT_COLS/20, 1};
		status = clEnqueueNDRangeKernel(a_this->m_commandQueue,a_this->m_kernel_rfpredict, 2, NULL, 
			global_work_size, NULL, 0, NULL, &iThisEvent);
		clCHECKERROR(status, CL_SUCCESS); 
		ret = clWaitForEvents(1, &iThisEvent);
		clCHECKERROR(ret, CL_SUCCESS);
		clFinish(a_this->m_commandQueue);
		MEASURE_TIME(a_this->clEnqueueNDRangeKernelFinishTimeD = tu.time());
	}

	{
		MEASURE_TIME(Util tu("rawToLabel"));
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_LabelRaw, CL_TRUE, 0, 160*120*31* sizeof(short), &a_this->m_pLabelRaw[0], 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); 
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_Label, CL_TRUE, 0, 160*120, &a_this->m_pLabel[0], 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); 
		MEASURE_TIME(a_this->clEnqueueReadBufferTimeD = tu.time());
	}
	a_this->clEnqueueRangKernelTimeD = cl_get_run_time( &iThisEvent, "clEnqueueRangKernel");
	return ret;
}

int initCentroid(SkeletonObj * a_this, unsigned short * depthDataWithIndex )
{

	cl_int status;
	cl_int ret;
	cl_event iThisEvent;

	// init variables;
//	char c = 0x00;
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_iCount, &c, 1, 0,  6*31*sizeof(int), 0, NULL, NULL);
//	clCHECKERROR(status, CL_SUCCESS);
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartTrackingState, &c, 1, 0,  6*sizeof(NUI_BODY_PART_TRACKING_STATE), 0, NULL, NULL);
//	clCHECKERROR(status, CL_SUCCESS);
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartPositionTrackingState, &c, 1, 0,  6*31*sizeof(NUI_BODY_PART_POSITION_TRACKING_STATE), 0, NULL, NULL);
//	clCHECKERROR(status, CL_SUCCESS);
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_vCentroid, &c, 1, 0,  6*31*sizeof(Vector4), 0, NULL, NULL);
//	clCHECKERROR(status, CL_SUCCESS);
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_m_pvCoordWorldSpacet, &c, 1, 0,  120*160*sizeof(Vector4), 0, NULL, NULL);
//	clCHECKERROR(status, CL_SUCCESS);
//	status = clEnqueueFillBuffer(a_this->m_commandQueue, a_this->cl_m_pfBodyPartProbility, &c, 1, 0,  6*31*120*160*sizeof(float), 0, NULL, NULL);
//	clCHECKERROR(status, CL_SUCCESS);

	status = clSetKernelArg(a_this->m_kernel_initCentroid, 0, sizeof(cl_mem), (void *)&a_this->cl_DepthFrame);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 1, sizeof(cl_mem), (void *)&a_this->cl_LabelRaw);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 2, sizeof(cl_mem), (void *)&a_this->cl_iCount);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 3, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartTrackingState);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 4, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartPositionTrackingState);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 5, sizeof(cl_mem), (void *)&a_this->cl_vCentroid);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 6, sizeof(cl_mem), (void *)&a_this->cl_m_pvCoordWorldSpacet);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_initCentroid, 7, sizeof(cl_mem), (void *)&a_this->cl_m_pfBodyPartProbility);
	clCHECKERROR(status, CL_SUCCESS);


	{
		MEASURE_TIME(Util tu("rawToLabel"));
		size_t global_work_size[2] = {120 , 160 };
		status = clEnqueueNDRangeKernel(a_this->m_commandQueue,a_this->m_kernel_initCentroid, 2, NULL, 
			global_work_size, NULL, 0, NULL, &iThisEvent);
		clCHECKERROR(status, CL_SUCCESS); 
		ret = clWaitForEvents(1, &iThisEvent);
		clCHECKERROR(ret, CL_SUCCESS);
		clFinish(a_this->m_commandQueue);
		MEASURE_TIME(a_this->gpuInitCentroidTimeD = tu.time());
	}
#ifdef _DEBUG
	//		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_CentroidData, CL_TRUE, 0, sizeof( a_this->m_CentroidData), &a_this->m_CentroidData, 0, NULL, NULL);
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_vCentroid, CL_TRUE, 0, 6 * 31 * sizeof(Vector4), a_this->vCentroid, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 
	//int k = sizeof(a_this->iCount);
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_iCount, CL_TRUE, 0, sizeof(a_this->iCount), a_this->iCount, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 
	//k = sizeof(a_this->eBodyPartTrackingState);
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartTrackingState), a_this->eBodyPartTrackingState, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 
	//k = sizeof(a_this->eBodyPartPositionTrackingState);
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartPositionTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartPositionTrackingState), a_this->eBodyPartPositionTrackingState, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 	

#endif
	return ret;
}

int centroidReduction(SkeletonObj * a_this )
{

	cl_int status;
	cl_int ret;
	cl_event iThisEvent;
	status = clSetKernelArg(a_this->m_kernel_centroidReduction, 0, sizeof(cl_mem), (void *)&a_this->cl_m_CentroidData);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_centroidReduction, 1, sizeof(cl_mem), (void *)&a_this->cl_iCount);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_centroidReduction, 2, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartPositionTrackingState);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_centroidReduction, 3, sizeof(cl_mem), (void *)&a_this->cl_vCentroid);
	clCHECKERROR(status, CL_SUCCESS);

	{
		MEASURE_TIME(Util tu("rawToLabel"));
		size_t global_work_size[2] = {6 , 31 };
		status = clEnqueueNDRangeKernel(a_this->m_commandQueue,a_this->m_kernel_centroidReduction, 2, NULL, 
			global_work_size, NULL, 0, NULL, &iThisEvent);
		clCHECKERROR(status, CL_SUCCESS); 
		ret = clWaitForEvents(1, &iThisEvent);
		clCHECKERROR(ret, CL_SUCCESS);
		clFinish(a_this->m_commandQueue);
		MEASURE_TIME(a_this->gpuInitCentroidTimeD = tu.time());
	}

	{
		MEASURE_TIME(Util tu("rawToLabel"));
		// take data back;
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_CentroidData, CL_TRUE, 0, sizeof( a_this->m_CentroidData), &a_this->m_CentroidData, 0, NULL, NULL);
		int k = sizeof( a_this->m_centroid);
		clCHECKERROR(status, CL_SUCCESS); 
	/*	status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartPositionTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartPositionTrackingState), a_this->eBodyPartPositionTrackingState, 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); 
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartTrackingState), a_this->eBodyPartTrackingState, 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); 
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_CentroidData, CL_TRUE, 0, sizeof(a_this->cl_vCentroid), &a_this->m_CentroidData, 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); 
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_pvCoordWorldSpacet, CL_TRUE, 0, sizeof(a_this->m_pvCoordWorldSpace), a_this->m_pvCoordWorldSpace, 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); 
		status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_pfBodyPartProbility, CL_TRUE, 0, sizeof(a_this->m_pfBodyPartProbility), a_this->m_pfBodyPartProbility, 0, NULL, NULL);
		clCHECKERROR(status, CL_SUCCESS); */
#ifdef _DEBUG
		//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_vCentroid, CL_TRUE, 0, 6 * 31 * sizeof(Vector4), a_this->vCentroid, 0, NULL, NULL);
		//clCHECKERROR(status, CL_SUCCESS); 
		//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_iCount, CL_TRUE, 0, sizeof(a_this->iCount), a_this->iCount, 0, NULL, NULL);
		//clCHECKERROR(status, CL_SUCCESS); 
		//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartTrackingState), a_this->eBodyPartTrackingState, 0, NULL, NULL);
		//clCHECKERROR(status, CL_SUCCESS); 
		//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartPositionTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartPositionTrackingState), a_this->eBodyPartPositionTrackingState, 0, NULL, NULL);
		//clCHECKERROR(status, CL_SUCCESS); 	

#endif
		MEASURE_TIME(a_this->gpuReadCentrpodTimeD = tu.time());
	}
	return ret;
}
int meanshift(SkeletonObj * a_this)
{
	cl_int status;
	cl_int ret;
	cl_event iThisEvent;
	status = clSetKernelArg(a_this->m_kernel_meanshift, 0, sizeof(cl_mem), (void *)&a_this->cl_m_CentroidData);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshift, 1, sizeof(cl_mem), (void *)&a_this->cl_iCount);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshift, 2, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartPositionTrackingState);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshift, 3, sizeof(cl_mem), (void *)&a_this->cl_vCentroid);
	clCHECKERROR(status, CL_SUCCESS);

	status = clSetKernelArg(a_this->m_kernel_meanshift, 4, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartTrackingState);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshift, 5, sizeof(cl_mem), (void *)&a_this->cl_m_pvCoordWorldSpacet);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshift, 6, sizeof(cl_mem), (void *)&a_this->cl_m_pfBodyPartProbility);
	clCHECKERROR(status, CL_SUCCESS);

	{
		MEASURE_TIME(Util tu("rawToLabel"));
		size_t global_work_size[2] = {6 , 31 };
		status = clEnqueueNDRangeKernel(a_this->m_commandQueue,a_this->m_kernel_meanshift, 2, NULL, 
			global_work_size, NULL, 0, NULL, &iThisEvent);
		clCHECKERROR(status, CL_SUCCESS); 
		ret = clWaitForEvents(1, &iThisEvent);
		clCHECKERROR(ret, CL_SUCCESS);

		clFinish(a_this->m_commandQueue);
		MEASURE_TIME(a_this->gpuMeanshiftTimeD = tu.time());
	}
#ifdef _DEBUG
/*			status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_CentroidData, CL_TRUE, 0, sizeof( a_this->m_CentroidData), &a_this->m_CentroidData, 0, NULL, NULL);
	status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_vCentroid, CL_TRUE, 0, 6 * 31 * sizeof(Vector4), a_this->vCentroid, 0, NULL, NULL);
	clCHECKERROR(status, CL_SUCCESS); 
	status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_iCount, CL_TRUE, 0, sizeof(a_this->iCount), a_this->iCount, 0, NULL, NULL);
	clCHECKERROR(status, CL_SUCCESS); 
	status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartTrackingState), a_this->eBodyPartTrackingState, 0, NULL, NULL);
	clCHECKERROR(status, CL_SUCCESS); 
	status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartPositionTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartPositionTrackingState), a_this->eBodyPartPositionTrackingState, 0, NULL, NULL);
	clCHECKERROR(status, CL_SUCCESS); */	

#endif

	return ret;
}
int meanshiftReduction(SkeletonObj * a_this)
{
	cl_int status;
	cl_int ret;
	cl_event iThisEvent;
	status = clSetKernelArg(a_this->m_kernel_meanshiftReduction, 0, sizeof(cl_mem), (void *)&a_this->cl_m_CentroidData);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshiftReduction, 1, sizeof(cl_mem), (void *)&a_this->cl_iCount);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshiftReduction, 2, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartPositionTrackingState);
	clCHECKERROR(status, CL_SUCCESS);
	status = clSetKernelArg(a_this->m_kernel_meanshiftReduction, 3, sizeof(cl_mem), (void *)&a_this->cl_vCentroid);
	clCHECKERROR(status, CL_SUCCESS);

	status = clSetKernelArg(a_this->m_kernel_meanshiftReduction, 4, sizeof(cl_mem), (void *)&a_this->cl_eBodyPartTrackingState);
	clCHECKERROR(status, CL_SUCCESS);


	{
		MEASURE_TIME(Util tu("rawToLabel"));
		size_t global_work_size[2] = {6 , 31 };
		status = clEnqueueNDRangeKernel(a_this->m_commandQueue,a_this->m_kernel_meanshiftReduction, 2, NULL, 
			global_work_size, NULL, 0, NULL, &iThisEvent);
		clCHECKERROR(status, CL_SUCCESS); 
		ret = clWaitForEvents(1, &iThisEvent);
		clCHECKERROR(ret, CL_SUCCESS);
		clFinish(a_this->m_commandQueue);
		MEASURE_TIME(a_this->gpuMeanshiftTimeD = tu.time());
	}
	status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_m_CentroidData, CL_TRUE, 0, sizeof(a_this->m_CentroidData), &a_this->m_CentroidData, 0, NULL, NULL);
	clCHECKERROR(status, CL_SUCCESS); 
#ifdef _DEBUG
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_vCentroid, CL_TRUE, 0, 6 * 31 * sizeof(Vector4), a_this->vCentroid, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_iCount, CL_TRUE, 0, sizeof(a_this->iCount), a_this->iCount, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartTrackingState), a_this->eBodyPartTrackingState, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 
	//status = clEnqueueReadBuffer(a_this->m_commandQueue, a_this->cl_eBodyPartPositionTrackingState, CL_TRUE, 0, sizeof(a_this->eBodyPartPositionTrackingState), a_this->eBodyPartPositionTrackingState, 0, NULL, NULL);
	//clCHECKERROR(status, CL_SUCCESS); 	

#endif
	return ret;
}
int Centroid(void * aathis, unsigned short * depthDataWithIndex )
{
	SkeletonObj * a_this =  (SkeletonObj *)aathis;
	initCentroid(a_this, depthDataWithIndex);
	centroidReduction(a_this);
	memcpy(g_vInitedCentroid_copy, a_this->vCentroid, 6 * 31 * sizeof(Vector4));
	meanshift(a_this);
	meanshiftReduction(a_this);
	return true;
}

void createKernels( SkeletonObj * a_this, cl_program program, cl_int iErrNum )
{
	a_this->m_kernel_rfpredict = clCreateKernel(program,"rfpredict", &iErrNum);
	clCHECKERROR(iErrNum, CL_SUCCESS);
	int i = sizeof(a_this->m_kernel_rfpredict);
	a_this->m_kernel_initCentroid = clCreateKernel(program,"initCentroid", &iErrNum);
	clCHECKERROR(iErrNum, CL_SUCCESS);
	a_this->m_kernel_centroidReduction = clCreateKernel(program,"centroidReduction", &iErrNum);
	clCHECKERROR(iErrNum, CL_SUCCESS);
	a_this->m_kernel_meanshift = clCreateKernel(program,"meanshift", &iErrNum);
	clCHECKERROR(iErrNum, CL_SUCCESS);
	a_this->m_kernel_meanshiftReduction = clCreateKernel(program,"meanshiftReduction", &iErrNum);
	clCHECKERROR(iErrNum, CL_SUCCESS);
}

void releaseKernels( SkeletonObj * a_this)
{
	cl_int status;
	status = clReleaseKernel(a_this->m_kernel_rfpredict);//Release kernel.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseKernel(a_this->m_kernel_initCentroid);//Release kernel.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseKernel(a_this->m_kernel_centroidReduction);//Release kernel.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseKernel(a_this->m_kernel_meanshift);//Release kernel.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseKernel(a_this->m_kernel_meanshiftReduction);//Release kernel.
	clCHECKERROR(status, CL_SUCCESS);
}
void ReleaseCL(void * aathis)
{
	SkeletonObj * a_this =  (SkeletonObj *)aathis;
	cl_int status;
	releaseKernels(a_this);
	cl_releaseForest(a_this);
	releaseBuffers(a_this);
	status = clFinish(a_this->m_commandQueue);
	status = clReleaseProgram(a_this->m_program);//Release program.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseCommandQueue(a_this->m_commandQueue);//Release command queue.
	clCHECKERROR(status, CL_SUCCESS);
	status = clReleaseContext(a_this->m_context);//Release context.
	clCHECKERROR(status, CL_SUCCESS);
	if (a_this->m_devices != NULL)
	{
		free(a_this->m_devices);
		a_this->m_devices= NULL;
	}

}
