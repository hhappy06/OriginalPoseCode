#ifndef TEST_GROW_H
#define TEST_GROW_H

	//void run();
	int initCL(void * a_this);
	void ReleaseCL(void * aathis);
	int bodyPartGpu(void * a_this, unsigned short * depthDataWithIndex );
	int Centroid(void * aathis, unsigned short * depthDataWithIndex );
	int cl_CreateBuffers(SkeletonObj * a_this);
//#ifdef __cplusplus

#endif



