#include <QFile>
#include <QDataStream>
#include "stdafx.h"
//#include <cl/cl.h>
//#include <ppl.h>
#include <iostream>
#include "zlib/zlib.h"
//#include "../resource.h"
#include "Util.h"

#include "RandomForest.h"
#include "BodyPartClassifier.h"
#include <fstream>

#include <iostream>
#include <float.h>

#include "clUtil.h"
#include <cassert>
#define NUI_IMAGE_PLAYER_INDEX_SHIFT          3
#define NUI_IMAGE_PLAYER_INDEX_MASK           ((1 << NUI_IMAGE_PLAYER_INDEX_SHIFT)-1) //*#define NUI_IMAGE_PLAYER_INDEX_MASK 7

//@xu-li:
#define THE_FIRST_TIME_FOR_FILTERING 1
#define NOT_THE_FIRST_TIME_FOR_FILTERING 0
//#define USE_PARALLEL
#ifdef USE_PARALLEL
#define PARALLEL_FOR_BEGIN(V, S, E) Concurrency::parallel_for(S, E, [&](int V)
#define PARALLEL_FOR_END            });
#else
#define PARALLEL_FOR_BEGIN(V, S, E) for (int V = S; V < E; y ++)
#define PARALLEL_FOR_END            }
#endif
#include <limits.h>

#define FILEOUTPUT(x)

double g_initCentroid = 0;
double g_calcCentroid = 0;
double g_initCentroidMem = 0;
double g_cpuPredictRawParallelTimeD = 0;
double g_cpuRawToLabelTimeD = 0;
bool g_doMeanshift = true;
Vector4 g_vInitedCentroid_copy[6][31];


//extern int parallelism_enabled;
bool g_NoHugeArray;
bool g_GrayGravityCentroid = false;

#ifdef WIN32
	
#endif // WIN32

//void getvCentroid(Vector4 Centroid[6][31])
//{
//	memcpy(Centroid, g_vInitedCentroid_copy, sizeof(g_vInitedCentroid_copy));
//}

BodyPartClassifier::BodyPartClassifier()
{
	//m_pLabelRaw = new unsigned short[160*120*31];
	//ASSERT(m_pLabelRaw);
	//m_pMask     = new unsigned char [160*120];
	//ASSERT(m_pMask);
	// CentroidsStage_RunFrame��
	init();

}

//BodyPartClassifier::BodyPartClassifier( cl_context a_context )
//{
//	init();
//	//m_context =  a_context;
//	//cl_int iErrNum;
//	//if(m_context)
//	//{
//	//	assert(m_pfBodyPartProbility);
//	//	cl_m_pfBodyPartProbility = clCreateBuffer(m_context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR, 6*31*120*160 * sizeof(float), m_pfBodyPartProbility, &iErrNum);
//	//	clCHECKERROR(iErrNum, CL_SUCCESS);
//	//}
//
//}


BodyPartClassifier::~BodyPartClassifier()
{
	//if (m_pLabelRaw)
	//{
	//    delete [] m_pLabelRaw;
	//    m_pLabelRaw = NULL;
	//}
	//if (m_pMask)
	//{
	//    delete [] m_pMask;
	//    m_pMask = NULL;
	//}

	//// CentroidsStage_RunFrame��
	//if (m_pvCoordWorldSpace)
	//{
	//	delete [] m_pvCoordWorldSpace;
	//	m_pvCoordWorldSpace = NULL;
	//}
	//if (m_pfBodyPartProbility)
	//{
	//	delete [] m_pfBodyPartProbility;
	//	m_pfBodyPartProbility = NULL;
	//}
}


//bool BodyPartClassifier::BuildForestFromFile(const char * szFileName)
//{
//    if (!szFileName || !szFileName[0])
//    {
//        return FALSE;
//    }
//
//    FILE * fp;
//#ifdef WIN32
//    fopen_s(&fp, szFileName, "rb");
//#else
//    fp = fopen(szFileName, "rb");
//#endif
//    if (!fp)
//    {
//        return FALSE;
//    }
//
//    int a = fseek(fp, 0, SEEK_END);
//    const int cbSize = ftell(fp);
//    fseek(fp, 0, SEEK_SET);
//
//    char * pForestData = new char [cbSize+32];
//    if (pForestData == NULL)
//    {
//        return FALSE;
//    }
//#ifdef WIN32
//    if (cbSize != fread_s(pForestData, cbSize+32, 1, cbSize, fp))
//#else
//    if (fread(pForestData, 1, cbSize+32, fp) != cbSize)
//#endif
//    {
//        delete [] pForestData;
//        fclose(fp);
//        return FALSE;
//    }
//
//    fclose(fp);
//    
//    bool res = m_forest.BuildForest(pForestData, cbSize);
//
//    delete [] pForestData;
//
//    return res;
//}

#define CHUNK 16384
#define MAGIC ((unsigned char)0x96);
//int inf(FILE *source, FILE *dest)
int inf(const unsigned char * pSrc, int srcSize, unsigned char * pDst, int dstSize, int * length)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	int sz, remain, pos;
	remain = srcSize;
	pos = 0;
	*length = 0;
	do {
		// decrypt
		sz = remain > CHUNK ? CHUNK : remain;
		if (sz == 0)
			break;
		memcpy(in, pSrc+pos, sz);
		remain -= sz;
		pos += sz;
		for (int i=0; i<sz; i++)
		{
			in[i] ^= MAGIC;
		}
		strm.avail_in = sz;
		/*strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
		(void)inflateEnd(&strm);
		return Z_ERRNO;
		}
		if (strm.avail_in == 0)
		break;*/
		strm.next_in = in;
		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			memcpy(pDst + *length, out, have);
			*length += have;
			/*if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
			}*/
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

extern int qInitResources_resource_qt();
bool BodyPartClassifier::BuildForestFromResource( )
{
	bool res = FALSE;
#ifdef WIN32___
	//HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDD_SINECTDEMO_DIALOG), MAKEINTRESOURCE(IDD_SINECTDEMO_DIALOG));
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_DATA), (LPCTSTR)"DATA");
	if (hRes == NULL)
	{
		return FALSE;
	}

	HGLOBAL hResLoad = LoadResource(NULL, hRes);
	if (hResLoad == NULL)
	{
		return FALSE;
	}
	const int cbSize = SizeofResource(NULL, hRes);

	LPVOID lpResLock = LockResource(hResLoad);
	if (lpResLock == NULL)
	{
		return FALSE;
	}

	// decrypt & uncompress
	const unsigned char * pSource = (const unsigned char *) lpResLock;
#else
    qInitResources_resource_qt();
	QFile file(":/model/res/XXX.z");							//@xu-li:QFile��QT���࣬���ļ���ַ�����ļ���
	if (!file.open(QIODevice::ReadOnly))
	{
		std::cout <<"resource open error!" <<std::endl;
		assert(0);
	};
	bool exist = file.exists();
	assert(exist);
	QDataStream in(&file);										//@xu-li:����һ��QDataStream�࣬QDataStream��QT���࣬�Զ������ṩ�����������������
	//int QDataStream::readRawData(char * s, int len)
	char * s = new  char [32*1024*1024];
	const  unsigned char * pSource  = (const unsigned char *)s;
	assert(s != 0);
	const int cbSize = in.readRawData(s, 32*1024*1024);
	assert (cbSize > 0);
#endif
	unsigned char * pDest = new unsigned char [32*1024*1024];

	int length;
	int ret = inf(pSource, cbSize, pDest, 32*1024*1024, &length);//@xu-li:���ܣ�
	if (ret == Z_OK)
	{
		res = m_forest.BuildForest((const  char *)pDest, length);
	}

	delete [] pDest;
	delete [] s;
	return res;
}
/**
*	@xu-li:ʹ��CPU����λ����
*	����˵����
*	const float player_scale��
*	���ͼ���׵�ַ��const unsigned short * pImage
*	ͼ�����������ص��31����λͶƱ�Ľ��ָ���С160*120*31��unsigned short * pLabelRaw
*	int labelConvMode��
*	unsigned char * pLabel��
*	���������
*	�Ƿ�ʹ��GPU���㲿λ����ı�ʶ����bool useGpu��
*/
void BodyPartClassifier::ExemplarStage_RunFrame(const float player_scale, const unsigned short * pImage, unsigned short * pLabelRaw, int labelConvMode,unsigned char * pLabel, bool useGpu)
{
	assert(pLabel);
	assert(pLabelRaw);
	if (useGpu)//@xu-li:�ж��Ƿ�ʹ��GPU,�˲����ڽӿں������жϴ˴�δʵ��
	{
		PredictRawGpu(player_scale, pImage, pLabelRaw);
	} 
	else
	{
		{
			MEASURE_TIME(Util tu("rawToLabel"));
			PredictRawParallel(player_scale, pImage, pLabelRaw);
			MEASURE_TIME(g_cpuPredictRawParallelTimeD = tu.time());
		}

	}


	//  memcpy(pLabelRaw, m_pLabelRaw, 160*120*31*sizeof(unsigned short));
	{
		MEASURE_TIME(Util tu("rawToLabel"));
		if (pLabel)
		{
			rawToLabel(pLabel, pImage, labelConvMode, pLabelRaw);

		}
		MEASURE_TIME(g_cpuRawToLabelTimeD = tu.time());
	}
}

/**
*@xu-li:
*����˵����
*const float player_scale
*���ͼ���׵�ַ��const unsigned short * pImage
*ͼ�����������ص��31����λͶƱ�Ľ��ָ���С160*120*31��unsigned short * pLabelRaw
*/
void BodyPartClassifier::PredictRawParallel(const float player_scale, const unsigned short * pImage, unsigned short * pLabelRaw)
{

	memset(pLabelRaw, 0, 160*120*31*2);
	//  memset(m_pMask, 0, 160*120);

	const int DepthUnusual = 6000;//�����Ч��Χ6m
#ifdef ANDROID
	FILEOUTPUT(std::ofstream file("/mnt/extSdCard/ubuntu/treeTravel.txt")) ;
#else
	FILEOUTPUT(std::ofstream file("./treeTravel.txt")) ;//ofstream�������ʽ���ļ�
#endif


/**
	//@xu-li��
	//Ϊ������⣬��ע�͵�Դ�룬���ú���
#ifdef USE_PARALLEL
	Concurrency::parallel_for(int(0), 120, [&](int y)
#else
	for (int y = 0; y < 120; y ++)
#endif
	{
		for (int x = 0; x < 160; x ++)
		{
#ifdef _DEBUG
			if (x == 88 && y == 112)
			{
				int sss = 0;
			}
#endif
			classifyOnePixel(pImage, y, x, player_scale, DepthUnusual, pLabelRaw);
		}
#ifdef USE_PARALLEL
	});
#else
	}
#endif
	FILEOUTPUT( file.close());

*///@xu-li
	/////////////////////////////////////////////////
	//@xu-li��
	for (int y = 0; y < 120; y ++)
	{
		for (int x = 0; x < 160; x ++)
		{
#ifdef _DEBUG						//@xu-li:���ã�
			if (x == 88 && y == 112)
			{
				int sss = 0;
			}
#endif
			classifyOnePixel(pImage, y, x, player_scale, DepthUnusual, pLabelRaw);
		}
	}
	FILEOUTPUT( file.close());
	//@xu-li��end
	/////////////////////////////////////////////////
	

}


int BodyPartClassifier::GetClassDistWithNeighbor2X2( const unsigned short * pImage, unsigned short * class_dist, int y, int x, int method, unsigned short * pLabelRaw )
{
	if (method == 0)
	{
		memcpy(class_dist, pLabelRaw + (y*160 + x) * 31, 31*sizeof(short));
		return 1;
	}

	unsigned short * class_dist_1 = pLabelRaw + (y*160 + x-1) * 31;
	unsigned short * class_dist_2 = pLabelRaw + (y*160 + x) * 31;
	unsigned short * class_dist_3 = pLabelRaw + ((y+1)*160 + x-1) * 31;
	unsigned short * class_dist_4 = pLabelRaw + ((y+1)*160 + x) * 31;

	unsigned short player_index_1 = pImage[(y*2) * 320 + ((x-1)*2)]     & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_2 = pImage[(y*2) * 320 + ((x)*2)]       & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_3 = pImage[((y+1)*2) * 320 + ((x-1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_4 = pImage[((y+1)*2) * 320 + ((x)*2)]   & NUI_IMAGE_PLAYER_INDEX_SHIFT;

	int k1 = (player_index_1 == player_index_2);
	int k2 = 1;
	int k3 = (player_index_3 == player_index_2);
	int k4 = (player_index_4 == player_index_2);

	memset(class_dist, 0, 31*sizeof(short));
	for (int cid = 0; cid < 31; cid++)
	{
		class_dist[cid] += class_dist_1[cid] * k1;
		class_dist[cid] += class_dist_2[cid] * k2;
		class_dist[cid] += class_dist_3[cid] * k3;
		class_dist[cid] += class_dist_4[cid] * k4;
	}

	return (k1+k2+k3+k4);
}


int BodyPartClassifier::GetClassDistWithNeighbor3X3( const unsigned short * pImage, unsigned short * class_dist, int y, int x, int method, unsigned short * pLabelRaw )
{
	if (method == 0)
	{
		memcpy(class_dist, pLabelRaw + (y*160 + x) * 31, 31*sizeof(short));
		return 1;
	}

	unsigned short * class_dist_1 = pLabelRaw + ((y-1)*160 + x-1) * 31;
	assert((*class_dist_1) < 3* 255);
	unsigned short * class_dist_2 = pLabelRaw + ((y-1)*160 + x+0) * 31;
	unsigned short * class_dist_3 = pLabelRaw + ((y-1)*160 + x+1) * 31;

	unsigned short * class_dist_4 = pLabelRaw + ((y+0)*160 + x-1) * 31;
	unsigned short * class_dist_5 = pLabelRaw + ((y+0)*160 + x+0) * 31;
	unsigned short * class_dist_6 = pLabelRaw + ((y+0)*160 + x+1) * 31;

	unsigned short * class_dist_7 = pLabelRaw + ((y+1)*160 + x-1) * 31;
	unsigned short * class_dist_8 = pLabelRaw + ((y+1)*160 + x+0) * 31;
	unsigned short * class_dist_9 = pLabelRaw + ((y+1)*160 + x+1) * 31;

	unsigned short player_index_1 = pImage[((y-1)*2) * 320 + ((x-1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_2 = pImage[((y-1)*2) * 320 + ((x+0)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_3 = pImage[((y-1)*2) * 320 + ((x+1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;

	unsigned short player_index_4 = pImage[((y+0)*2) * 320 + ((x-1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_5 = pImage[((y+0)*2) * 320 + ((x+0)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_6 = pImage[((y+0)*2) * 320 + ((x+1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;

	unsigned short player_index_7 = pImage[((y+1)*2) * 320 + ((x-1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_8 = pImage[((y+1)*2) * 320 + ((x+0)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index_9 = pImage[((y+1)*2) * 320 + ((x+1)*2)] & NUI_IMAGE_PLAYER_INDEX_SHIFT;

	int k1 = (player_index_1 == player_index_5);
	int k2 = (player_index_2 == player_index_5);
	int k3 = (player_index_3 == player_index_5);

	int k4 = (player_index_4 == player_index_5);
	int k5 = (player_index_5 == player_index_5);
	int k6 = (player_index_6 == player_index_5);

	int k7 = (player_index_7 == player_index_5);
	int k8 = (player_index_8 == player_index_5);
	int k9 = (player_index_9 == player_index_5);

	memset(class_dist, 0, 31*sizeof(short));
	for (int cid = 0; cid < 31; cid++)
	{
		class_dist[cid] += class_dist_1[cid] * k1;
		class_dist[cid] += class_dist_2[cid] * k2;
		class_dist[cid] += class_dist_3[cid] * k3;
		class_dist[cid] += class_dist_4[cid] * k4;
		class_dist[cid] += class_dist_5[cid] * k5;
		class_dist[cid] += class_dist_6[cid] * k6;
		class_dist[cid] += class_dist_7[cid] * k7;
		class_dist[cid] += class_dist_8[cid] * k8;
		class_dist[cid] += class_dist_9[cid] * k9;
		assert(k1+k2+k3+k4+k5+k6+k7+k8+k9 > 0);
		class_dist[cid] /= (k1+k2+k3+k4+k5+k6+k7+k8+k9);
		if (class_dist[cid] > 3*255)
		{
			assert(0);
			int a = 1;
		}
	}

	return (k1+k2+k3+k4+k5+k6+k7+k8+k9);
}

#define MEASURE_TIME(X) X
//#define MEASURE_TIME(X)

void BodyPartClassifier::CentroidsStage_RunFrame(const unsigned short * pImage, const unsigned short * pLabelRaw, NUI_CENTROIDS_STAGE_DATA * pData,
												 Vector4 * m_pvCoordWorldSpace, float * m_pfBodyPartProbility)
{
	if (pData == NULL)
	{
		return;
	}
	Vector4 vCentroid[6][31];
	NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31];

	NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6];
	int iCount[6][31];
			ASSERT(pData);
	if (!g_GrayGravityCentroid)
	{
		initCentroidpoint(pImage, pLabelRaw, pData, iCount, eBodyPartTrackingState, eBodyPartPositionTrackingState, vCentroid, m_pvCoordWorldSpace, m_pfBodyPartProbility);
		if (g_doMeanshift)
		{
			meanshift(vCentroid, eBodyPartPositionTrackingState, eBodyPartTrackingState, pData, iCount, m_pvCoordWorldSpace, m_pfBodyPartProbility, pLabelRaw, pImage);
		}
	} 
	else
	{

		 GrayGravityCentroid(pImage, pLabelRaw, vCentroid, eBodyPartPositionTrackingState, eBodyPartTrackingState);
	}
	vCentroid2BodyPartCentroids(pData, eBodyPartTrackingState, vCentroid, iCount, eBodyPartPositionTrackingState);


}


NUI_SKELETON_POSITION_TRACKING_STATE MAX_Stage(NUI_BODY_PART_POSITION_TRACKING_STATE s1, NUI_BODY_PART_POSITION_TRACKING_STATE s2)
{
	int i1 = (int)(s1);
	int i2 = (int)(s2);
	int res = std::max<int>(i1, i2);

	return (NUI_SKELETON_POSITION_TRACKING_STATE)res;
}


NUI_SKELETON_POSITION_TRACKING_STATE MIN_Stage(NUI_BODY_PART_POSITION_TRACKING_STATE s1, NUI_BODY_PART_POSITION_TRACKING_STATE s2)
{
	int i1 = (int)(s1);
	int i2 = (int)(s2);
	int res = std::min<int>(i1, i2);

	return (NUI_SKELETON_POSITION_TRACKING_STATE)res;
}


#define GET_SKELETON_1(Z, X) {\
	pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[X];\
	pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
}

#define GET_SKELETON_2(Z, X, Y) {\
	pSkeleton->SkeletonPositions[Z] = (pCentroids->Centroids[X] + pCentroids->Centroids[Y]) * 0.5f; \
	pSkeleton->eSkeletonPositionTrackingState[Z] = MIN_Stage(pCentroids->eBodyPartPositionTrackingState[X], pCentroids->eBodyPartPositionTrackingState[Y]);\
}

#define GET_SKELETON_3(Z, X, Y) {\
	if ((int)pCentroids->eBodyPartPositionTrackingState[X] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
{\
	pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[X];\
	pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
}\
	else if ((int)pCentroids->eBodyPartPositionTrackingState[Y] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
{\
	pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[Y];\
	pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[Y];\
}\
	else\
{\
	pSkeleton->SkeletonPositions[Z] = Vector4();\
	pSkeleton->eSkeletonPositionTrackingState[Z] = NUI_SKELETON_POSITION_NOT_TRACKED;\
}\
}

#define GET_SKELETON_4_1(Z, X, Y, W) {\
	pSkeleton->SkeletonPositions[Z] = (pCentroids->Centroids[X] + pCentroids->Centroids[Y] + pCentroids->Centroids[W]) * 0.3333333f; \
	NUI_BODY_PART_POSITION_TRACKING_STATE ts = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[X], pCentroids->eBodyPartPositionTrackingState[Y]);\
	pSkeleton->eSkeletonPositionTrackingState[Z] = MIN_Stage(ts, pCentroids->eBodyPartPositionTrackingState[W]);\
}

#define GET_SKELETON_4(Z, A, B, C) {\
	pSkeleton->SkeletonPositions[Z].x = (pCentroids->Centroids[A].x + pCentroids->Centroids[B].x + pCentroids->Centroids[C].x) * 0.3333333f; \
	pSkeleton->SkeletonPositions[Z].y = (pCentroids->Centroids[A].y + pCentroids->Centroids[B].y + pCentroids->Centroids[C].y) * 0.3333333f; \
	pSkeleton->SkeletonPositions[Z].z = (pCentroids->Centroids[A].z + pCentroids->Centroids[B].z + pCentroids->Centroids[C].z) * 0.3333333f; \
	NUI_BODY_PART_POSITION_TRACKING_STATE ts = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[A], pCentroids->eBodyPartPositionTrackingState[B]);\
	pSkeleton->eSkeletonPositionTrackingState[Z] = MIN_Stage(ts, pCentroids->eBodyPartPositionTrackingState[C]);\
}


#define GET_SKELETON_5(Z, A, B, C, D) {\
	pSkeleton->SkeletonPositions[Z] = (pCentroids->Centroids[A] + pCentroids->Centroids[B] + pCentroids->Centroids[C] + pCentroids->Centroids[D]) * 0.25f; \
	NUI_BODY_PART_POSITION_TRACKING_STATE ts1 = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[A], pCentroids->eBodyPartPositionTrackingState[B]);\
	NUI_BODY_PART_POSITION_TRACKING_STATE ts2 = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[C], pCentroids->eBodyPartPositionTrackingState[D]);\
	pSkeleton->eSkeletonPositionTrackingState[Z] = MIN_Stage(ts1, ts2);\
}


/////////////////////////////////////////@xu-li//////////////////////////////////////////////////////////////////////////
#define GET_SKELETON_1_sf(Z, X) {\
	pSkeleton_sf->SkeletonPositions[Z] = pCentroids->Centroids[X];\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
}

#define GET_SKELETON_2_sf(Z, X, Y) {\
	pSkeleton_sf->SkeletonPositions[Z] = (pCentroids->Centroids[X] + pCentroids->Centroids[Y]) * 0.5f; \
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = MIN_Stage(pCentroids->eBodyPartPositionTrackingState[X], pCentroids->eBodyPartPositionTrackingState[Y]);\
}

#define GET_SKELETON_3_sf(Z, X, Y) {\
	if ((int)pCentroids->eBodyPartPositionTrackingState[X] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
{\
	pSkeleton_sf->SkeletonPositions[Z] = pCentroids->Centroids[X];\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
}\
	else if ((int)pCentroids->eBodyPartPositionTrackingState[Y] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
{\
	pSkeleton_sf->SkeletonPositions[Z] = pCentroids->Centroids[Y];\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[Y];\
}\
	else\
{\
	pSkeleton_sf->SkeletonPositions[Z] = Vector4();\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = NUI_SKELETON_POSITION_NOT_TRACKED;\
}\
}

#define GET_SKELETON_4_1_sf(Z, X, Y, W) {\
	pSkeleton_sf->SkeletonPositions[Z] = (pCentroids->Centroids[X] + pCentroids->Centroids[Y] + pCentroids->Centroids[W]) * 0.3333333f; \
	NUI_BODY_PART_POSITION_TRACKING_STATE ts_sf = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[X], pCentroids->eBodyPartPositionTrackingState[Y]);\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = MIN_Stage(ts_sf, pCentroids->eBodyPartPositionTrackingState[W]);\
}

#define GET_SKELETON_4_sf(Z, A, B, C) {\
	pSkeleton_sf->SkeletonPositions[Z].x = (pCentroids->Centroids[A].x + pCentroids->Centroids[B].x + pCentroids->Centroids[C].x) * 0.3333333f; \
	pSkeleton_sf->SkeletonPositions[Z].y = (pCentroids->Centroids[A].y + pCentroids->Centroids[B].y + pCentroids->Centroids[C].y) * 0.3333333f; \
	pSkeleton_sf->SkeletonPositions[Z].z = (pCentroids->Centroids[A].z + pCentroids->Centroids[B].z + pCentroids->Centroids[C].z) * 0.3333333f; \
	NUI_BODY_PART_POSITION_TRACKING_STATE ts_sf = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[A], pCentroids->eBodyPartPositionTrackingState[B]);\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = MIN_Stage(ts_sf, pCentroids->eBodyPartPositionTrackingState[C]);\
}


#define GET_SKELETON_5_sf(Z, A, B, C, D) {\
	pSkeleton_sf->SkeletonPositions[Z] = (pCentroids->Centroids[A] + pCentroids->Centroids[B] + pCentroids->Centroids[C] + pCentroids->Centroids[D]) * 0.25f; \
	NUI_BODY_PART_POSITION_TRACKING_STATE ts1_sf = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[A], pCentroids->eBodyPartPositionTrackingState[B]);\
	NUI_BODY_PART_POSITION_TRACKING_STATE ts2_sf = (NUI_BODY_PART_POSITION_TRACKING_STATE)MIN_Stage(pCentroids->eBodyPartPositionTrackingState[C], pCentroids->eBodyPartPositionTrackingState[D]);\
	pSkeleton_sf->eSkeletonPositionTrackingState[Z] = MIN_Stage(ts1_sf, ts2_sf);\
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float Distance(const Vector4 & v1, const Vector4 & v2)
{
	double distance = sqrt(pow((v1.x - v2.x), 2) + pow((v1.y - v2.y), 2) + pow((v1.z - v2.z), 2));
	return static_cast<float>(distance);
}


#define TRACE_BODY_PART_3(X, A, B, C) TRACE(#X": State = %d, Distance = {%.5f, %.5f, %.5f}\n", \
	pCentroids->eBodyPartPositionTrackingState[X], \
	Distance(pCentroids->Centroids[X], pCentroids->Centroids[A]), \
	Distance(pCentroids->Centroids[X], pCentroids->Centroids[B]), \
	Distance(pCentroids->Centroids[X], pCentroids->Centroids[C]));

#define TRACE_BODY_PART_2(X, A, B) TRACE(#X": State = %d, Distance = {%.5f, %.5f}\n", \
	pCentroids->eBodyPartPositionTrackingState[X], \
	Distance(pCentroids->Centroids[X], pCentroids->Centroids[A]), \
	Distance(pCentroids->Centroids[X], pCentroids->Centroids[B]));

/**
*@xu-li:
*�������ƣ�ModelFittingStage_RunFrame
*����������
*���������
*		1.const NUI_CENTROIDS_STAGE_DATA * pCentroidData	��λ���ĵ㣻
*���������
*		1.NUI_SKELETON_FRAME * pSkeletonFrame				�Ǽ����ݣ�
*����ֵ��void��
*/
void BodyPartClassifier::ModelFittingStage_RunFrame(const NUI_CENTROIDS_STAGE_DATA * pCentroidData, NUI_SKELETON_FRAME * pSkeletonFrame)
{
	/*@xu-li
	typedef struct _NUI_CENTROIDS_STAGE_DATA
	{
		NUI_BODY_PART_CENTROIDS BodyPartCentroids[ 6 ];
	} NUI_CENTROIDS_STAGE_DATA;
	*/
	/*@xu-li
	typedef struct _NUI_BODY_PART_CENTROIDS
	{
		NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState;
		Vector4 Centroids[ 31 ];
		int PixelCount[ 31 ];
		NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[ 31 ];
	} NUI_BODY_PART_CENTROIDS;
	*/

	memset(pSkeletonFrame, 0, sizeof(NUI_SKELETON_FRAME));
	for (int pid = 0; pid < 6; pid ++)//@xu=li:�ദ6��31��λѭ�����Ƿ�����ϣ�����������
	{
		/*
		typedef struct _NUI_BODY_PART_CENTROIDS
		{@xu-li
			NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState;
			Vector4 Centroids[ 31 ];
			int PixelCount[ 31 ];
			NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[ 31 ];
		} NUI_BODY_PART_CENTROIDS;
		*/
		const NUI_BODY_PART_CENTROIDS * pCentroids = &(pCentroidData->BodyPartCentroids[pid]);//
		/*@xu-li
		typedef struct _NUI_SKELETON_DATA
		{
			NUI_SKELETON_TRACKING_STATE eTrackingState;
			int dwTrackingID;
			int dwEnrollmentIndex;
			int dwUserIndex;
			Vector4 Position;
			Vector4 SkeletonPositions[ 20 ];
			NUI_SKELETON_POSITION_TRACKING_STATE eSkeletonPositionTrackingState[ 20 ];
			int dwQualityFlags;
		}     NUI_SKELETON_DATA;
		*/
		NUI_SKELETON_DATA * pSkeleton = &(pSkeletonFrame->SkeletonData[pid]);
		if (pCentroids->eBodyPartTrackingState == NUI_BODY_PART_NOT_TRACKED)	//@xu-li:���û�и��ٵ��ˣ���Ǽ�����Ϊû�и��ٵ���Ҳ��һ������p>0.14�Ĳ�λ�㶼û��
		{
			pSkeleton->eTrackingState = NUI_SKELETON_NOT_TRACKED;
			continue;
		}
		ASSERT(pCentroids->eBodyPartTrackingState == NUI_BODY_PART_POSISION_ONLY);

		//   pSkeleton->eTrackingState = NUI_SKELETON_POSITION_ONLY;
		pSkeleton->eTrackingState = NUI_SKELETON_TRACKED;						//@xu-li:����������������Ļ�����Ϊ����˵ĹǼ��Ǹ����ϵģ�����Ӧ�û����ǽڵ�����ˣ�ֻ���ˣ�

		// ����BodyPartλ��

#if 0
		NUI_SKELETON_POSITION_HEAD            ;  //3, (BODY_PART_LU_HEAD + BODY_PART_RU_HEAD) * 0.5f
		NUI_SKELETON_POSITION_SHOULDER_CENTER ;  //2, BODY_PART_NECK
		NUI_SKELETON_POSITION_SHOULDER_LEFT   ;  //4, BODY_PART_L_SHOULDER
		NUI_SKELETON_POSITION_SHOULDER_RIGHT  ;  //8, BODY_PART_R_SHOULDER
		NUI_SKELETON_POSITION_ELBOW_LEFT      ;  //5, BODY_PART_L_ELBOW
		NUI_SKELETON_POSITION_ELBOW_RIGHT     ;  //9, BODY_PART_R_ELBOW
		NUI_SKELETON_POSITION_WRIST_LEFT      ;  //6, BODY_PART_L_WRIST/BODY_PART_L_HAND/0(������)
		NUI_SKELETON_POSITION_WRIST_RIGHT     ;  //10, BODY_PART_R_WRIST/BODY_PART_R_HAND/0(������)
		NUI_SKELETON_POSITION_HAND_LEFT       ;  //7, BODY_PART_L_HAND/BODY_PART_L_WRIST/0(������)
		NUI_SKELETON_POSITION_HAND_RIGHT      ;  //11, BODY_PART_R_HAND/BODY_PART_R_WRIST/0(������)
		NUI_SKELETON_POSITION_KNEE_LEFT       ;  //13, BODY_PART_L_KNEE
		NUI_SKELETON_POSITION_KNEE_RIGHT      ;  //17, BODY_PART_R_KNEE
		NUI_SKELETON_POSITION_ANKLE_LEFT      ;  //14, BODY_PART_L_ANKLE
		NUI_SKELETON_POSITION_ANKLE_RIGHT     ;  //18, BODY_PART_R_ANKLE
		NUI_SKELETON_POSITION_FOOT_LEFT       ;  //15, BODY_PART_L_FOOT
		NUI_SKELETON_POSITION_FOOT_RIGHT      ;  //19, BODY_PART_R_FOOT
		NUI_SKELETON_POSITION_HIP_LEFT        ;  //12, BODY_PART_LW_TORSO(������)
		NUI_SKELETON_POSITION_HIP_RIGHT       ;  //16, BODY_PART_RW_TORSO(������)
		NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
		NUI_SKELETON_POSITION_SPINE           ;  //1, ���ӣ���׵
#endif

		//pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HEAD] = (pCentroids->Centroids[BODY_PART_LU_HEAD] + pCentroids->Centroids[BODY_PART_RU_HEAD]) * 0.5f;
		//pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] = MIN_Stage(pCentroids->eTrackingState[BODY_PART_LU_HEAD], pCentroids->eTrackingState[BODY_PART_RU_HEAD]);
		//pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HEAD] = (pCentroids->Centroids[BODY_PART_LU_HEAD] + pCentroids->Centroids[BODY_PART_RU_HEAD]) * 0.5f;
		//pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] = MIN_Stage(pCentroids->eTrackingState[BODY_PART_LU_HEAD], pCentroids->eTrackingState[BODY_PART_RU_HEAD]);
		/*if ((int)pCentroids->eTrackingState[BODY_PART_L_WRIST] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)
		{
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] = pCentroids->Centroids[BODY_PART_L_WRIST];
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_WRIST_LEFT] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eTrackingState[BODY_PART_L_WRIST];
		}
		else if ((int)pCentroids->eTrackingState[BODY_PART_L_HAND] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)
		{
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] = pCentroids->Centroids[BODY_PART_L_HAND];
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_WRIST_LEFT] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eTrackingState[BODY_PART_L_HAND];
		}
		else
		{
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] = Vector4();
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_WRIST_LEFT] = NUI_SKELETON_POSITION_NOT_TRACKED;
		}*/

		/*@xu-li:���ڹ۲�
			#define GET_SKELETON_1(Z, X) {\
				pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[X];\
				pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
			}

			#define GET_SKELETON_2(Z, X, Y) {\
				pSkeleton->SkeletonPositions[Z] = (pCentroids->Centroids[X] + pCentroids->Centroids[Y]) * 0.5f; \
				pSkeleton->eSkeletonPositionTrackingState[Z] = MIN_Stage(pCentroids->eBodyPartPositionTrackingState[X], pCentroids->eBodyPartPositionTrackingState[Y]);\
			}

			#define GET_SKELETON_3(Z, X, Y) {\
				if ((int)pCentroids->eBodyPartPositionTrackingState[X] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
			{\
				pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[X];\
				pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
			}\
				else if ((int)pCentroids->eBodyPartPositionTrackingState[Y] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
			{\
				pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[Y];\
				pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[Y];\
			}\
				else\
			{\
				pSkeleton->SkeletonPositions[Z] = Vector4();\
				pSkeleton->eSkeletonPositionTrackingState[Z] = NUI_SKELETON_POSITION_NOT_TRACKED;\
			}\
			}
		*/
		// 16 joints
		//@xu-li:GET_SKELETON_1-һ�����ĵ�����Ǽܽڵ㣬ֱ�Ӱ����ĵ㵱���ڵ�
		//@xu-li:GET_SKELETON_2-�������ĵ�����Ǽܽڵ㣬ȡ�����ƽ��ֵ�����м�λ��Ϊ�ڵ�;����״̬Ϊ��Сֵ������״̬��ö��ֵ����С����Ϊ��û�����ϡ��ƶϳ���������
		//@xu-li:GET_SKELETON_3-�������ĵ�����Ǽܽڵ㣬����ʹ��ǰһ���ĵ����ڵ㣬���ǰ��û�и��ٵ�����ȡ���ߣ����߾�Ϊ���ٵ������ж��ùǼܽڵ�û�и��ٵ�
		GET_SKELETON_2(NUI_SKELETON_POSITION_HEAD, BODY_PART_LU_HEAD, BODY_PART_RU_HEAD);		//@xu-li:ͷ����λ���������ĸ���λ��ȡ����ͷ�����ĵ�������ص�Ϊͷ���Ǽܽڵ�
		GET_SKELETON_1(NUI_SKELETON_POSITION_SHOULDER_CENTER, BODY_PART_NECK);					//@xu-li:���Ӳ�λ���ĵ���Ϊ�����Ǽܽڵ�
		GET_SKELETON_1(NUI_SKELETON_POSITION_SHOULDER_LEFT, BODY_PART_L_SHOULDER);				//@xu-li:��粿λ���ĵ���Ϊ���Ǽܹؽڵ�
		GET_SKELETON_1(NUI_SKELETON_POSITION_SHOULDER_RIGHT, BODY_PART_R_SHOULDER);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ELBOW_LEFT, BODY_PART_L_ELBOW);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ELBOW_RIGHT, BODY_PART_R_ELBOW);
		GET_SKELETON_3(NUI_SKELETON_POSITION_WRIST_LEFT, BODY_PART_L_WRIST, BODY_PART_L_HAND);	//@xu-li:���Ƚ�����λ���ĵ㸳������Ǽܽڵ㣬û���ҵ�����λ���ĵ�Ļ�ʹ�����ֲ�λ���ĵ����
		GET_SKELETON_3(NUI_SKELETON_POSITION_WRIST_RIGHT, BODY_PART_R_WRIST, BODY_PART_R_HAND);
		GET_SKELETON_3(NUI_SKELETON_POSITION_HAND_LEFT, BODY_PART_L_HAND, BODY_PART_L_WRIST);	//@xu-li:���Ƚ����ֲ�λ���ĵ���Ϊ���ֹǼ����ĵ㣬û���ҵ��Ļ�ʹ������λ���ĵ����
		GET_SKELETON_3(NUI_SKELETON_POSITION_HAND_RIGHT, BODY_PART_R_HAND, BODY_PART_L_HAND);
		GET_SKELETON_1(NUI_SKELETON_POSITION_KNEE_LEFT, BODY_PART_L_KNEE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_KNEE_RIGHT, BODY_PART_R_KNEE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ANKLE_LEFT, BODY_PART_L_ANKLE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ANKLE_RIGHT, BODY_PART_R_ANKLE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_FOOT_LEFT, BODY_PART_L_FOOT);
		GET_SKELETON_1(NUI_SKELETON_POSITION_FOOT_RIGHT, BODY_PART_R_FOOT);
		//
		// TODO: fix hand and wrist
#if 0
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT])*1.5f;
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] = NUI_SKELETON_POSITION_INFERRED;
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT])*1.5f;
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] = NUI_SKELETON_POSITION_INFERRED;
#else	//@xu-li:�˶���֤�ֺ���ľ����Ƿ����
		float dis_rhand_rwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);//@xu-li:ŷ�Ͼ���
		float dis_rhand_lwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);
		float dis_lhand_rwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);
		float dis_lhand_lwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);
		//TRACE("dis_rhand_rwrist = %f, dis_lhand_lwrist = %f\n", dis_rhand_rwrist, dis_lhand_lwrist);
		if (dis_rhand_rwrist > 0.3)//@xu-li:ͬ���ֺ������������ϵŷ�Ͼ������0.3m
		{
			//@xu-li:�˴���ʽ��Ϊ�����Ľڵ�λ����׼ȷ�ģ��ô��������¶�λ�ֵ�λ��
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT])*1.5f;
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] = NUI_SKELETON_POSITION_INFERRED;
		}
		if (dis_lhand_lwrist > 0.3)
		{
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT])*1.5f;
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] = NUI_SKELETON_POSITION_INFERRED;
		}
		dis_rhand_rwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);//@xu-li:���¼����������ŷ�Ͼ���
		dis_lhand_lwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);
		//TRACE("dis_rhand_rwrist_after = %f, dis_lhand_lwrist_after = %f\n", dis_rhand_rwrist, dis_lhand_lwrist);
#endif
		// HIP //@xu-li:����ȷ������4��δȷ���ĹǼܽڵ�
		Vector4 tmp = (pCentroids->Centroids[BODY_PART_LU_HEAD] + pCentroids->Centroids[BODY_PART_RU_HEAD]) * 0.5f;
		if (tmp.x < FLT_MIN && tmp.x > -FLT_MIN &&
			tmp.y < FLT_MIN && tmp.y > -FLT_MIN &&
			tmp.z < FLT_MIN && tmp.z > -FLT_MIN)		//@xu-li:�����˵ͷ��������Ϊ0�����ڹ��ḽ������ʱ��Ϊ�°����Ѿ����ٳ������ˣ������������°�����
		{
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO);			//@xu-li:������������Ϊ����
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO);		
			//NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER] = Vector4();	//@xu-li:�β�����
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] = NUI_SKELETON_POSITION_NOT_TRACKED;
			//NUI_SKELETON_POSITION_SPINE           ;  //1, ����
			// ��ʱ����, ��δȷ��
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_SPINE] = Vector4();		//@xu-li:��׵--
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SPINE] = NUI_SKELETON_POSITION_NOT_TRACKED;
		}
		else
		{
#if 1
			// ���¾���δȷ��
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO);
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO);

			GET_SKELETON_5(NUI_SKELETON_POSITION_SPINE, BODY_PART_LU_TORSO, BODY_PART_RU_TORSO, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			//GET_SKELETON_4(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_HIP_RIGHT);

			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER] = (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_SPINE] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_LEFT]*2.0f + pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_RIGHT]*2.0f)) * 0.2f;
			int tmp1 = std::min<int>((int)pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_LEFT], (int)pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_RIGHT]);
			int tmp2 = std::min<int>((int)pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SPINE], tmp1);
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] = (NUI_SKELETON_POSITION_TRACKING_STATE)tmp2;

			/*GET_SKELETON_2(NUI_SKELETON_POSITION_SPINE, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_SPINE].y -= 0.35; //(0.86f * (pCentroids->Centroids[BODY_PART_RW_TORSO].x - pCentroids->Centroids[BODY_PART_LW_TORSO].x));
			GET_SKELETON_4(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_SPINE);*/
#else
			//NUI_SKELETON_POSITION_HIP_LEFT        ;  //12, BODY_PART_LW_TORSO(������)
			GET_SKELETON_2(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO, BODY_PART_LU_LEG);
			//NUI_SKELETON_POSITION_HIP_RIGHT       ;  //16, BODY_PART_RW_TORSO(������)
			GET_SKELETON_2(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO, BODY_PART_RU_LEG);
			//NUI_SKELETON_POSITION_SPINE           ;  //1, ����
			GET_SKELETON_2(NUI_SKELETON_POSITION_SPINE, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			//NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
			GET_SKELETON_4(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_SPINE);
#endif
		}
		/**************************************************************************************************************************/
		//@xu-li:

		printf("/*****************************************************/\n");
		printf("/*****************************************************/\n");
		
		//@xu-li:ÿһ���ڵ㶼�Ƕ����˲��ģ��еĿ����������������˲��ǻ�û���ҵ���ͻȻ��ĳһʱ���ҵ��ˣ�����Ҫ�����ж�ÿһ���ڵ�ĸ������
		for(int joint=0; joint<20; joint++)
		{
			if( NUI_SKELETON_POSITION_TRACKED == pSkeleton->eSkeletonPositionTrackingState[ joint ] )//@���û�и����ϣ��Ƿ�Ҳ������˲�������������ʽ�����ϣ��ֻص������١��ĸ�������
			{
				if(THE_FIRST_TIME_FOR_FILTERING == isfirstTime[ joint ])
				{
					isfirstTime[joint] = NOT_THE_FIRST_TIME_FOR_FILTERING;
					//for(int i = 0; i<20; i++)							
					{
						pretmp1[pid*20 + joint] = pSkeleton->SkeletonPositions[joint];
						pretmp2[pid*20 + joint] = pSkeleton->SkeletonPositions[joint];
						btmp1[pid*20 + joint] = 0;
						btmp2[pid*20 + joint] = 0;
					}

				}

				pSkeleton->SkeletonPositions[joint] = filter->Exponential_sf(&pSkeleton->SkeletonPositions[joint], &pretmp1[pid*20 + joint], 0.2);

				//pSkeleton->SkeletonPositions[joint] = filter->Exponential_sf(&pSkeleton->SkeletonPositions[joint], &pretmp1[pid*20 + joint], &btmp1[pid*20 + joint], 0.2, 0.8);
			
				//filter->DExponential_sf( &pSkeleton->SkeletonPositions[joint], &pretmp1[pid*20 + joint], &pretmp2[pid*20 + joint], 
				//							&btmp1[pid*20 + joint], &btmp2[pid*20 + joint],0.4, 0.8);//aԽ����ʱԽС����ǰ���ݵ�Ӱ��ԽС��vԽ����ʱԽС���Ա仯Խ����
				
				//filter->JitterRemoval_sf(&pSkeleton->SkeletonPositions[joint], &pretmp1[pid*20 + joint], &btmp1[pid*20 + joint], 0.2, 0.8);

					
			}
			else//@xu-li:������Ҫ�ڹǼܶ�ʧ�����¸�ǰһʱ��Ԥ��ֵ�ĳ�ʼֵ��ֵ
			{
				isfirstTime[ joint ] = THE_FIRST_TIME_FOR_FILTERING;
			}
		}
				
		printf("/*****************************************************/\n");
		printf("/*****************************************************/\n");

			
		//@xu-li:�����������������жϹǼܽڵ����ʵ��
		isTracked( pSkeleton );
		
		/**************************************************************************************************************************/
	}
}

//@xu-li
void BodyPartClassifier::ModelFittingStage_RunFrame(const NUI_CENTROIDS_STAGE_DATA * pCentroidData, NUI_SKELETON_FRAME * pSkeletonFrame,  NUI_SKELETON_FRAME * pSkeletonFrame_sf)
{
	/*@xu-li
	typedef struct _NUI_CENTROIDS_STAGE_DATA
	{
		NUI_BODY_PART_CENTROIDS BodyPartCentroids[ 6 ];
	} NUI_CENTROIDS_STAGE_DATA;
	*/
	/*@xu-li
	typedef struct _NUI_BODY_PART_CENTROIDS
	{
		NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState;
		Vector4 Centroids[ 31 ];
		int PixelCount[ 31 ];
		NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[ 31 ];
	} NUI_BODY_PART_CENTROIDS;
	*/

	memset(pSkeletonFrame, 0, sizeof(NUI_SKELETON_FRAME));
	memset(pSkeletonFrame_sf, 0, sizeof(NUI_SKELETON_FRAME));
	for (int pid = 0; pid < 6; pid ++)//@xu=li:�ദ6��31��λѭ�����Ƿ�����ϣ�����������
	{
		/*
		typedef struct _NUI_BODY_PART_CENTROIDS
		{@xu-li
			NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState;
			Vector4 Centroids[ 31 ];
			int PixelCount[ 31 ];
			NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[ 31 ];
		} NUI_BODY_PART_CENTROIDS;
		*/
		const NUI_BODY_PART_CENTROIDS * pCentroids = &(pCentroidData->BodyPartCentroids[pid]);//
		/*@xu-li
		typedef struct _NUI_SKELETON_DATA
		{
			NUI_SKELETON_TRACKING_STATE eTrackingState;
			int dwTrackingID;
			int dwEnrollmentIndex;
			int dwUserIndex;
			Vector4 Position;
			Vector4 SkeletonPositions[ 20 ];
			NUI_SKELETON_POSITION_TRACKING_STATE eSkeletonPositionTrackingState[ 20 ];
			int dwQualityFlags;
		}     NUI_SKELETON_DATA;
		*/
		NUI_SKELETON_DATA * pSkeleton = &(pSkeletonFrame->SkeletonData[pid]);
		NUI_SKELETON_DATA * pSkeleton_sf = &(pSkeletonFrame_sf->SkeletonData[pid]);
		if (pCentroids->eBodyPartTrackingState == NUI_BODY_PART_NOT_TRACKED)	//@xu-li:���û�и��ٵ��ˣ���Ǽ�����Ϊû�и��ٵ���Ҳ��һ������p>0.14�Ĳ�λ�㶼û��
		{
			pSkeleton->eTrackingState = NUI_SKELETON_NOT_TRACKED;
			pSkeleton_sf->eTrackingState = NUI_SKELETON_NOT_TRACKED;
			continue;
		}
		ASSERT(pCentroids->eBodyPartTrackingState == NUI_BODY_PART_POSISION_ONLY);

		//   pSkeleton->eTrackingState = NUI_SKELETON_POSITION_ONLY;
		pSkeleton->eTrackingState = NUI_SKELETON_TRACKED;						//@xu-li:����������������Ļ�����Ϊ����˵ĹǼ��Ǹ����ϵģ�����Ӧ�û����ǽڵ�����ˣ�ֻ���ˣ�
		pSkeleton_sf->eTrackingState = NUI_SKELETON_TRACKED;						//@xu-li:����������������Ļ�����Ϊ����˵ĹǼ��Ǹ����ϵģ�����Ӧ�û����ǽڵ�����ˣ�ֻ���ˣ�
		// ����BodyPartλ��

#if 0
		NUI_SKELETON_POSITION_HEAD            ;  //3, (BODY_PART_LU_HEAD + BODY_PART_RU_HEAD) * 0.5f
		NUI_SKELETON_POSITION_SHOULDER_CENTER ;  //2, BODY_PART_NECK
		NUI_SKELETON_POSITION_SHOULDER_LEFT   ;  //4, BODY_PART_L_SHOULDER
		NUI_SKELETON_POSITION_SHOULDER_RIGHT  ;  //8, BODY_PART_R_SHOULDER
		NUI_SKELETON_POSITION_ELBOW_LEFT      ;  //5, BODY_PART_L_ELBOW
		NUI_SKELETON_POSITION_ELBOW_RIGHT     ;  //9, BODY_PART_R_ELBOW
		NUI_SKELETON_POSITION_WRIST_LEFT      ;  //6, BODY_PART_L_WRIST/BODY_PART_L_HAND/0(������)
		NUI_SKELETON_POSITION_WRIST_RIGHT     ;  //10, BODY_PART_R_WRIST/BODY_PART_R_HAND/0(������)
		NUI_SKELETON_POSITION_HAND_LEFT       ;  //7, BODY_PART_L_HAND/BODY_PART_L_WRIST/0(������)
		NUI_SKELETON_POSITION_HAND_RIGHT      ;  //11, BODY_PART_R_HAND/BODY_PART_R_WRIST/0(������)
		NUI_SKELETON_POSITION_KNEE_LEFT       ;  //13, BODY_PART_L_KNEE
		NUI_SKELETON_POSITION_KNEE_RIGHT      ;  //17, BODY_PART_R_KNEE
		NUI_SKELETON_POSITION_ANKLE_LEFT      ;  //14, BODY_PART_L_ANKLE
		NUI_SKELETON_POSITION_ANKLE_RIGHT     ;  //18, BODY_PART_R_ANKLE
		NUI_SKELETON_POSITION_FOOT_LEFT       ;  //15, BODY_PART_L_FOOT
		NUI_SKELETON_POSITION_FOOT_RIGHT      ;  //19, BODY_PART_R_FOOT
		NUI_SKELETON_POSITION_HIP_LEFT        ;  //12, BODY_PART_LW_TORSO(������)
		NUI_SKELETON_POSITION_HIP_RIGHT       ;  //16, BODY_PART_RW_TORSO(������)
		NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
		NUI_SKELETON_POSITION_SPINE           ;  //1, ���ӣ���׵
#endif

		//pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HEAD] = (pCentroids->Centroids[BODY_PART_LU_HEAD] + pCentroids->Centroids[BODY_PART_RU_HEAD]) * 0.5f;
		//pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] = MIN_Stage(pCentroids->eTrackingState[BODY_PART_LU_HEAD], pCentroids->eTrackingState[BODY_PART_RU_HEAD]);
		//pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HEAD] = (pCentroids->Centroids[BODY_PART_LU_HEAD] + pCentroids->Centroids[BODY_PART_RU_HEAD]) * 0.5f;
		//pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] = MIN_Stage(pCentroids->eTrackingState[BODY_PART_LU_HEAD], pCentroids->eTrackingState[BODY_PART_RU_HEAD]);
		/*if ((int)pCentroids->eTrackingState[BODY_PART_L_WRIST] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)
		{
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] = pCentroids->Centroids[BODY_PART_L_WRIST];
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_WRIST_LEFT] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eTrackingState[BODY_PART_L_WRIST];
		}
		else if ((int)pCentroids->eTrackingState[BODY_PART_L_HAND] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)
		{
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] = pCentroids->Centroids[BODY_PART_L_HAND];
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_WRIST_LEFT] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eTrackingState[BODY_PART_L_HAND];
		}
		else
		{
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] = Vector4();
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_WRIST_LEFT] = NUI_SKELETON_POSITION_NOT_TRACKED;
		}*/

		/*@xu-li:���ڹ۲�
			#define GET_SKELETON_1(Z, X) {\
				pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[X];\
				pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
			}

			#define GET_SKELETON_2(Z, X, Y) {\
				pSkeleton->SkeletonPositions[Z] = (pCentroids->Centroids[X] + pCentroids->Centroids[Y]) * 0.5f; \
				pSkeleton->eSkeletonPositionTrackingState[Z] = MIN_Stage(pCentroids->eBodyPartPositionTrackingState[X], pCentroids->eBodyPartPositionTrackingState[Y]);\
			}

			#define GET_SKELETON_3(Z, X, Y) {\
				if ((int)pCentroids->eBodyPartPositionTrackingState[X] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
			{\
				pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[X];\
				pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[X];\
			}\
				else if ((int)pCentroids->eBodyPartPositionTrackingState[Y] > (int)NUI_BODY_PART_POSITION_NOT_TRACKED)\
			{\
				pSkeleton->SkeletonPositions[Z] = pCentroids->Centroids[Y];\
				pSkeleton->eSkeletonPositionTrackingState[Z] = (NUI_SKELETON_POSITION_TRACKING_STATE)pCentroids->eBodyPartPositionTrackingState[Y];\
			}\
				else\
			{\
				pSkeleton->SkeletonPositions[Z] = Vector4();\
				pSkeleton->eSkeletonPositionTrackingState[Z] = NUI_SKELETON_POSITION_NOT_TRACKED;\
			}\
			}
		*/
		// 16 joints
		//@xu-li:GET_SKELETON_1-һ�����ĵ�����Ǽܽڵ㣬ֱ�Ӱ����ĵ㵱���ڵ�
		//@xu-li:GET_SKELETON_2-�������ĵ�����Ǽܽڵ㣬ȡ�����ƽ��ֵ�����м�λ��Ϊ�ڵ�;����״̬Ϊ��Сֵ������״̬��ö��ֵ����С����Ϊ��û�����ϡ��ƶϳ���������
		//@xu-li:GET_SKELETON_3-�������ĵ�����Ǽܽڵ㣬����ʹ��ǰһ���ĵ����ڵ㣬���ǰ��û�и��ٵ�����ȡ���ߣ����߾�Ϊ���ٵ������ж��ùǼܽڵ�û�и��ٵ�
		GET_SKELETON_2(NUI_SKELETON_POSITION_HEAD, BODY_PART_LU_HEAD, BODY_PART_RU_HEAD);		//@xu-li:ͷ����λ���������ĸ���λ��ȡ����ͷ�����ĵ�������ص�Ϊͷ���Ǽܽڵ�
		GET_SKELETON_1(NUI_SKELETON_POSITION_SHOULDER_CENTER, BODY_PART_NECK);					//@xu-li:���Ӳ�λ���ĵ���Ϊ�����Ǽܽڵ�
		GET_SKELETON_1(NUI_SKELETON_POSITION_SHOULDER_LEFT, BODY_PART_L_SHOULDER);				//@xu-li:��粿λ���ĵ���Ϊ���Ǽܹؽڵ�
		GET_SKELETON_1(NUI_SKELETON_POSITION_SHOULDER_RIGHT, BODY_PART_R_SHOULDER);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ELBOW_LEFT, BODY_PART_L_ELBOW);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ELBOW_RIGHT, BODY_PART_R_ELBOW);
		GET_SKELETON_3(NUI_SKELETON_POSITION_WRIST_LEFT, BODY_PART_L_WRIST, BODY_PART_L_HAND);	//@xu-li:���Ƚ�����λ���ĵ㸳������Ǽܽڵ㣬û���ҵ�����λ���ĵ�Ļ�ʹ�����ֲ�λ���ĵ����
		GET_SKELETON_3(NUI_SKELETON_POSITION_WRIST_RIGHT, BODY_PART_R_WRIST, BODY_PART_R_HAND);
		GET_SKELETON_3(NUI_SKELETON_POSITION_HAND_LEFT, BODY_PART_L_HAND, BODY_PART_L_WRIST);	//@xu-li:���Ƚ����ֲ�λ���ĵ���Ϊ���ֹǼ����ĵ㣬û���ҵ��Ļ�ʹ������λ���ĵ����
		GET_SKELETON_3(NUI_SKELETON_POSITION_HAND_RIGHT, BODY_PART_R_HAND, BODY_PART_L_HAND);
		GET_SKELETON_1(NUI_SKELETON_POSITION_KNEE_LEFT, BODY_PART_L_KNEE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_KNEE_RIGHT, BODY_PART_R_KNEE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ANKLE_LEFT, BODY_PART_L_ANKLE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_ANKLE_RIGHT, BODY_PART_R_ANKLE);
		GET_SKELETON_1(NUI_SKELETON_POSITION_FOOT_LEFT, BODY_PART_L_FOOT);
		GET_SKELETON_1(NUI_SKELETON_POSITION_FOOT_RIGHT, BODY_PART_R_FOOT);
		//

		//@xu-li
		GET_SKELETON_2_sf(NUI_SKELETON_POSITION_HEAD, BODY_PART_LU_HEAD, BODY_PART_RU_HEAD);		//@xu-li:ͷ����λ���������ĸ���λ��ȡ����ͷ�����ĵ�������ص�Ϊͷ���Ǽܽڵ�
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_SHOULDER_CENTER, BODY_PART_NECK);					//@xu-li:���Ӳ�λ���ĵ���Ϊ�����Ǽܽڵ�
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_SHOULDER_LEFT, BODY_PART_L_SHOULDER);				//@xu-li:��粿λ���ĵ���Ϊ���Ǽܹؽڵ�
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_SHOULDER_RIGHT, BODY_PART_R_SHOULDER);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_ELBOW_LEFT, BODY_PART_L_ELBOW);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_ELBOW_RIGHT, BODY_PART_R_ELBOW);
		GET_SKELETON_3_sf(NUI_SKELETON_POSITION_WRIST_LEFT, BODY_PART_L_WRIST, BODY_PART_L_HAND);	//@xu-li:���Ƚ�����λ���ĵ㸳������Ǽܽڵ㣬û���ҵ�����λ���ĵ�Ļ�ʹ�����ֲ�λ���ĵ����
		GET_SKELETON_3_sf(NUI_SKELETON_POSITION_WRIST_RIGHT, BODY_PART_R_WRIST, BODY_PART_R_HAND);
		GET_SKELETON_3_sf(NUI_SKELETON_POSITION_HAND_LEFT, BODY_PART_L_HAND, BODY_PART_L_WRIST);	//@xu-li:���Ƚ����ֲ�λ���ĵ���Ϊ���ֹǼ����ĵ㣬û���ҵ��Ļ�ʹ������λ���ĵ����
		GET_SKELETON_3_sf(NUI_SKELETON_POSITION_HAND_RIGHT, BODY_PART_R_HAND, BODY_PART_L_HAND);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_KNEE_LEFT, BODY_PART_L_KNEE);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_KNEE_RIGHT, BODY_PART_R_KNEE);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_ANKLE_LEFT, BODY_PART_L_ANKLE);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_ANKLE_RIGHT, BODY_PART_R_ANKLE);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_FOOT_LEFT, BODY_PART_L_FOOT);
		GET_SKELETON_1_sf(NUI_SKELETON_POSITION_FOOT_RIGHT, BODY_PART_R_FOOT);


		//
		// TODO: fix hand and wrist
#if 0
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT])*1.5f;
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] = NUI_SKELETON_POSITION_INFERRED;
		pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT])*1.5f;
		pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] = NUI_SKELETON_POSITION_INFERRED;

		pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT] + (pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT] - pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT])*1.5f;
		pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] = NUI_SKELETON_POSITION_INFERRED;
		pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT] + (pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] - pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT])*1.5f;
		pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] = NUI_SKELETON_POSITION_INFERRED;


#else	//@xu-li:�˶���֤�ֺ���ľ����Ƿ����
		float dis_rhand_rwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);//@xu-li:ŷ�Ͼ���
		float dis_rhand_lwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);
		float dis_lhand_rwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);
		float dis_lhand_lwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);
		

		
		//TRACE("dis_rhand_rwrist = %f, dis_lhand_lwrist = %f\n", dis_rhand_rwrist, dis_lhand_lwrist);
		if (dis_rhand_rwrist > 0.3)//@xu-li:ͬ���ֺ������������ϵŷ�Ͼ������0.3m
		{
			//@xu-li:�˴���ʽ��Ϊ�����Ľڵ�λ����׼ȷ�ģ��ô��������¶�λ�ֵ�λ��
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT])*1.5f;
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] = NUI_SKELETON_POSITION_INFERRED;

			pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT] + (pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT] - pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT])*1.5f;
			pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_RIGHT] = NUI_SKELETON_POSITION_INFERRED;

		}
		if (dis_lhand_lwrist > 0.3)
		{
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] - pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT])*1.5f;
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] = NUI_SKELETON_POSITION_INFERRED;

			pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT] + (pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT] - pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT])*1.5f;
			pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HAND_LEFT] = NUI_SKELETON_POSITION_INFERRED;

		}
		dis_rhand_rwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);//@xu-li:���¼����������ŷ�Ͼ���
		dis_lhand_lwrist = Distance(pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);

		dis_rhand_rwrist = Distance(pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT], pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT]);//@xu-li:���¼����������ŷ�Ͼ���
		dis_lhand_lwrist = Distance(pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT], pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT]);


		//TRACE("dis_rhand_rwrist_after = %f, dis_lhand_lwrist_after = %f\n", dis_rhand_rwrist, dis_lhand_lwrist);
#endif
		// HIP //@xu-li:����ȷ������4��δȷ���ĹǼܽڵ�
		Vector4 tmp = (pCentroids->Centroids[BODY_PART_LU_HEAD] + pCentroids->Centroids[BODY_PART_RU_HEAD]) * 0.5f;
		if (tmp.x < FLT_MIN && tmp.x > -FLT_MIN &&
			tmp.y < FLT_MIN && tmp.y > -FLT_MIN &&
			tmp.z < FLT_MIN && tmp.z > -FLT_MIN)		//@xu-li:�����˵ͷ��������Ϊ0�����ڹ��ḽ������ʱ��Ϊ�°����Ѿ����ٳ������ˣ������������°�����
		{
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO);			//@xu-li:������������Ϊ����
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO);		
			//NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER] = Vector4();	//@xu-li:�β�����
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] = NUI_SKELETON_POSITION_NOT_TRACKED;
			//NUI_SKELETON_POSITION_SPINE           ;  //1, ����
			// ��ʱ����, ��δȷ��
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_SPINE] = Vector4();		//@xu-li:��׵--
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SPINE] = NUI_SKELETON_POSITION_NOT_TRACKED;


			GET_SKELETON_1_sf(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO);			//@xu-li:������������Ϊ����
			GET_SKELETON_1_sf(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO);	
			//NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
			pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER] = Vector4();	//@xu-li:�β�����
			pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] = NUI_SKELETON_POSITION_NOT_TRACKED;
			//NUI_SKELETON_POSITION_SPINE           ;  //1, ����
			// ��ʱ����, ��δȷ��
			pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_SPINE] = Vector4();		//@xu-li:��׵--
			pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SPINE] = NUI_SKELETON_POSITION_NOT_TRACKED;

		}
		else
		{
#if 1
			// ���¾���δȷ��
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO);
			GET_SKELETON_1(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO);

			GET_SKELETON_5(NUI_SKELETON_POSITION_SPINE, BODY_PART_LU_TORSO, BODY_PART_RU_TORSO, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			//GET_SKELETON_4(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_HIP_RIGHT);

			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER] = (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_SPINE] + (pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_LEFT]*2.0f + pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_HIP_RIGHT]*2.0f)) * 0.2f;
			int tmp1 = std::min<int>((int)pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_LEFT], (int)pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_RIGHT]);
			int tmp2 = std::min<int>((int)pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SPINE], tmp1);
			pSkeleton->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] = (NUI_SKELETON_POSITION_TRACKING_STATE)tmp2;

			// ���¾���δȷ��
			GET_SKELETON_1_sf(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO);
			GET_SKELETON_1_sf(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO);

			GET_SKELETON_5_sf(NUI_SKELETON_POSITION_SPINE, BODY_PART_LU_TORSO, BODY_PART_RU_TORSO, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER] = (pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_SPINE] + (pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HIP_LEFT]*2.0f + pSkeleton_sf->SkeletonPositions[NUI_SKELETON_POSITION_HIP_RIGHT]*2.0f)) * 0.2f;
			int tmp1_sf = std::min<int>((int)pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_LEFT], (int)pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_RIGHT]);
			int tmp2_sf = std::min<int>((int)pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SPINE], tmp1);
			pSkeleton_sf->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] = (NUI_SKELETON_POSITION_TRACKING_STATE)tmp2;


			/*GET_SKELETON_2(NUI_SKELETON_POSITION_SPINE, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			pSkeleton->SkeletonPositions[NUI_SKELETON_POSITION_SPINE].y -= 0.35; //(0.86f * (pCentroids->Centroids[BODY_PART_RW_TORSO].x - pCentroids->Centroids[BODY_PART_LW_TORSO].x));
			GET_SKELETON_4(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_SPINE);*/
#else
			//NUI_SKELETON_POSITION_HIP_LEFT        ;  //12, BODY_PART_LW_TORSO(������)
			GET_SKELETON_2(NUI_SKELETON_POSITION_HIP_LEFT, BODY_PART_LW_TORSO, BODY_PART_LU_LEG);
			//NUI_SKELETON_POSITION_HIP_RIGHT       ;  //16, BODY_PART_RW_TORSO(������)
			GET_SKELETON_2(NUI_SKELETON_POSITION_HIP_RIGHT, BODY_PART_RW_TORSO, BODY_PART_RU_LEG);
			//NUI_SKELETON_POSITION_SPINE           ;  //1, ����
			GET_SKELETON_2(NUI_SKELETON_POSITION_SPINE, BODY_PART_LW_TORSO, BODY_PART_RW_TORSO);
			//NUI_SKELETON_POSITION_HIP_CENTER      ;  //0, ����
			GET_SKELETON_4(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_SPINE);
#endif
		}
		/**************************************************************************************************************************/
		//@xu-li:

		printf("/*****************************************************/\n");
		printf("/*****************************************************/\n");
		
		//@xu-li:ÿһ���ڵ㶼�Ƕ����˲��ģ��еĿ����������������˲��ǻ�û���ҵ���ͻȻ��ĳһʱ���ҵ��ˣ�����Ҫ�����ж�ÿһ���ڵ�ĸ������
		for(int joint=0; joint<20; joint++)
		{
			if( NUI_SKELETON_POSITION_TRACKED == pSkeleton_sf->eSkeletonPositionTrackingState[ joint ] )//@���û�и����ϣ��Ƿ�Ҳ������˲�������������ʽ�����ϣ��ֻص������١��ĸ�������
			{
				if(THE_FIRST_TIME_FOR_FILTERING == isfirstTime[ joint ])
				{
					isfirstTime[joint] = NOT_THE_FIRST_TIME_FOR_FILTERING;
					//for(int i = 0; i<20; i++)							
					{
						pretmp1[pid*20 + joint] = pSkeleton_sf->SkeletonPositions[joint];
						pretmp2[pid*20 + joint] = pSkeleton_sf->SkeletonPositions[joint];
						btmp1[pid*20 + joint] = 0;
						btmp2[pid*20 + joint] = 0;
					}

				}


				pSkeleton_sf->SkeletonPositions[joint] = filter->Exponential_sf(&pSkeleton_sf->SkeletonPositions[joint], &pretmp1[pid*20 + joint], 0.2);

				//pSkeleton_sf->SkeletonPositions[joint] = filter->Exponential_sf(&pSkeleton_sf->SkeletonPositions[joint], &pretmp1[pid*20 + joint], &btmp1[pid*20 + joint], 0.2, 0.6);
			
				//filter->DExponential_sf( &pSkeleton_sf->SkeletonPositions[joint], &pretmp1[pid*20 + joint], &pretmp2[pid*20 + joint], 
				//							&btmp1[pid*20 + joint], &btmp2[pid*20 + joint],0.4, 0.8);//aԽ����ʱԽС����ǰ���ݵ�Ӱ��ԽС��vԽ����ʱԽС���Ա仯Խ����
												
				//filter->JitterRemoval_sf(&pSkeleton_sf->SkeletonPositions[joint], &pretmp1[pid*20 + joint], &btmp1[pid*20 + joint], 0.2, 0.8);
					
			}
			else//@xu-li:������Ҫ�ڹǼܶ�ʧ�����¸�ǰһʱ��Ԥ��ֵ�ĳ�ʼֵ��ֵ
			{
				isfirstTime[ joint ] = THE_FIRST_TIME_FOR_FILTERING;
			}
		}
				
		printf("/*****************************************************/\n");
		printf("/*****************************************************/\n");

			
		//@xu-li:�����������������жϹǼܽڵ����ʵ��
		isTracked( pSkeleton_sf );
		
		/**************************************************************************************************************************/
	}
}

/*@xu-li:
*
*�����������������������жϹǼܽڵ����ʵ��
*/
//@xu-li:
void BodyPartClassifier::isTracked(NUI_SKELETON_DATA * pSkeletonData )
{
	float dis_neck_hip = Distance(pSkeletonData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER], pSkeletonData->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER]);//@xu-li:���¼����κ;���ŷ�Ͼ���
	float dis_lknee_lhip = Distance(pSkeletonData->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_LEFT], pSkeletonData->SkeletonPositions[NUI_SKELETON_POSITION_HIP_LEFT]);//@xu-li:���¼������κ���ϥ��ŷ�Ͼ���
	float dis_rknee_rhip = Distance(pSkeletonData->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_RIGHT], pSkeletonData->SkeletonPositions[NUI_SKELETON_POSITION_HIP_RIGHT]);//@xu-li:���¼������κ���ϥ��ŷ�Ͼ���
	
	float rat_ulleg = dis_lknee_lhip / dis_neck_hip;
	float rat_urleg = dis_rknee_rhip / dis_neck_hip;

	printf("\n\n\n/***********************************/\n\n");
	printf("the rat_ulleg = %.5f\n", dis_lknee_lhip);
	printf("the rat_urleg = %.3f\n", rat_urleg);
	printf("\n\n\n/***********************************/\n\n");

}


void BodyPartClassifier::ImageToWorldSpace(const Vector4I & src, Vector4 & dst, int height/* = 120*/, int width/* = 160*/)
{
	//  static const float NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 = 3.501f/1000;
	assert(height >= 120);
	assert(width >= 160);

	dst.z = src.z * 0.001f;	//@xu-li:��λ����m
	dst.x = (src.x - width/2.0f) * (320.0f/width) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * dst.z;
	dst.y = - (src.y - height/2.0f) * (240.0f/height) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * dst.z;
	dst.w = 0;
}


void BodyPartClassifier::WorldToImageSpace(const Vector4 & src, Vector4I & dst, int height/* = 120*/, int width/* = 160*/)
{
	//  static const float NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 = 285.63f;
	assert(height >= 120);
	assert(width >= 160);

	if(src.z > FLT_EPSILON)
	{
		dst.x = (int)(width/2 + src.x * (width/320.0) * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / src.z);
		dst.y = (int)(height/2 - src.y * (height/240.0) * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / src.z);
		dst.z = (int)(src.z * 1000);
		dst.w = 1;
	}
	else
	{
		dst.x = 0;
		dst.y = 0;
		dst.z = 0;
		dst.w = 0;
	}
	//assert(dst.y >= 0 && dst.y < height);
	//assert(dst.x >= 0 && dst.x < width);
}



int BodyPartClassifier::NuiTransformSmooth(NUI_SKELETON_FRAME *pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS *apSmoothingParams/* = NULL*/)
{
	NUI_TRANSFORM_SMOOTH_PARAMETERS smoothingParams, params;

	if (pSkeletonFrame == NULL)
	{
		assert(0);
		return S_FALSE;
	}
	if (apSmoothingParams == NULL)
	{
		memcpy(&smoothingParams, &m_defaultSmoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
	}
	else
	{
		memcpy(&smoothingParams, apSmoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
	}


	// ���ƽ������
	if (smoothingParams.fJitterRadius < 0.001f)
	{
		smoothingParams.fJitterRadius = 0.001f;
	}

	for (int skeleton_i = 0; skeleton_i < 6; skeleton_i ++)											//@xu-li:6�˹Ǽ�
	{
		if (pSkeletonFrame->SkeletonData[skeleton_i].eTrackingState ==  NUI_SKELETON_NOT_TRACKED)	//@xu-li:�Ƿ��д�id������Ǽ�
		{
			continue;
		}

		NUI_SKELETON_DATA * p_skeleton_data = &(pSkeletonFrame->SkeletonData[skeleton_i]);
		Joints_Smoothing_State * p_smoothingState = &m_msSmoothingState[skeleton_i];

		for (int joint_i = 0; joint_i < 20; joint_i ++)															//@xu-li:20���ڵ����
		{
			memcpy(&params, &smoothingParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS));
			if (p_skeleton_data->eSkeletonPositionTrackingState[joint_i] == NUI_SKELETON_POSITION_NOT_TRACKED)	//@xu-li:�Ƿ�����ϸýڵ㣬���Ƿ����õ�
			{
				continue;
			}
			else if (p_skeleton_data->eSkeletonPositionTrackingState[joint_i] == NUI_SKELETON_POSITION_INFERRED)//@xu-li:���߸ýڵ��Ƿ�Ϊ�ƶϵõ���
			{
				params.fJitterRadius       *= 2;
				params.fMaxDeviationRadius *= 2;
			}

			Vector4 X,   X1, // ����ؽ�����/�ϴ�����ؽ�����
				Xr,      // JitterRadius����
				Xn,  Xn1,// Smoothing
				Bn,  Bn1,// Correction
				Xnp,     // Prediction
				Xnpr,    // MaxDeviationRadius����
				Xo;      // ����ؽ�����

			Bn1 = p_smoothingState->m_Bn[joint_i];
			Xn1 = p_smoothingState->m_Xn[joint_i];
			X1  = p_smoothingState->m_X[joint_i];

			X.x = p_skeleton_data->SkeletonPositions[joint_i].x;
			X.y = p_skeleton_data->SkeletonPositions[joint_i].y;
			X.z = p_skeleton_data->SkeletonPositions[joint_i].z;
			X.w = p_skeleton_data->SkeletonPositions[joint_i].w;

			if (X.x == 0.0f && X.y == 0.0f && X.z == 0.0f)
			{
				p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_INITIAL;
			}

			if (p_smoothingState->m_FilterState[joint_i] == SMOOTH_FILTER_INITIAL)
			{
				p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_NTRACKED;
				Xn.x = X.x; Xn.y = X.y; Xn.z = X.z; Xn.w = X.w;
				Bn.x = Bn.y = Bn.z = Bn.w = 0;
			}
			else if (p_smoothingState->m_FilterState[joint_i] == SMOOTH_FILTER_NTRACKED)
			{
				// Smoothing
				// X~n.x = (Xn.x + X(n-1).x) * 0.5
				Xn.x = (X.x + X1.x) * 0.5f;
				Xn.y = (X.y + X1.y) * 0.5f;
				Xn.z = (X.z + X1.z) * 0.5f;
				Xn.w = (X.w + X1.w) * 0.5f;

				// Correction
				// bn.x = fCorrection * ((Xn.x + X(n-1).x) * 0.5 - X~(n-1).x) + (1 - fCorrection) * b(n-1).x
				Bn.x = params.fCorrection * (Xn.x - Xn1.x) + (1 - params.fCorrection) * Bn1.x;
				Bn.y = params.fCorrection * (Xn.y - Xn1.y) + (1 - params.fCorrection) * Bn1.y;
				Bn.z = params.fCorrection * (Xn.z - Xn1.z) + (1 - params.fCorrection) * Bn1.z;
				Bn.w = params.fCorrection * (Xn.w - Xn1.w) + (1 - params.fCorrection) * Bn1.w;

				// Update Filter Status
				p_smoothingState->m_FilterState[joint_i] = SMOOTH_FILTER_TRACKED;
			}
			else // SMOOTH_FILTER_TRACKED
			{
				// check JitterRadius, repair X got Xr
				float X_move_distance = sqrt((X.x-Xn1.x)*(X.x-Xn1.x) + (X.y-Xn1.y)*(X.y-Xn1.y) + (X.z-Xn1.z)*(X.z-Xn1.z));
				/*
				if (joint_i == NUI_SKELETON_POSITION_HAND_RIGHT)
				{
				TRACE("NUI_SKELETON_POSITION_HAND_RIGHT: move_distance = %.5f\n", X_move_distance);
				}*/

				if (X_move_distance > params.fJitterRadius)
				{
					/*
					if (X_move_distance > 15 * params.fJitterRadius)
					{
					//Xr = Xn1;
					Xr.x = Xn1.x + params.fPrediction * Bn1.x;
					Xr.y = Xn1.y + params.fPrediction * Bn1.y;
					Xr.z = Xn1.z + params.fPrediction * Bn1.z;
					Xr.w = Xn1.w + params.fPrediction * Bn1.w;
					X = Xr;
					}
					else
					*/
					{
						Xr = X;
					}
				}
				else
				{
					float XMoveRatio = X_move_distance / params.fJitterRadius;
					assert(XMoveRatio <= 1.0f);
					Xr.x = (1 - XMoveRatio) * Xn1.x + XMoveRatio * X.x;
					Xr.y = (1 - XMoveRatio) * Xn1.y + XMoveRatio * X.y;
					Xr.z = (1 - XMoveRatio) * Xn1.z + XMoveRatio * X.z;
					Xr.w = (1 - XMoveRatio) * Xn1.w + XMoveRatio * X.w;
				}
				// Smoothing
				// (1 - fSmoothing) * Xn.x + fSmoothing * (X~(n-1).x + b(n-1).x)
				Xn.x = (1 - params.fSmoothing) * Xr.x + params.fSmoothing * (Xn1.x + Bn1.x);
				Xn.y = (1 - params.fSmoothing) * Xr.y + params.fSmoothing * (Xn1.y + Bn1.y);
				Xn.z = (1 - params.fSmoothing) * Xr.z + params.fSmoothing * (Xn1.z + Bn1.z);
				Xn.w = (1 - params.fSmoothing) * Xr.w + params.fSmoothing * (Xn1.w + Bn1.w);

				// Correction
				// fCorrection * (X~n.x - X~(n-1).x) + (1 - fCorrection) * b(n-1).x = bn.x
				Bn.x = params.fCorrection * (Xn.x - Xn1.x) + (1 - params.fCorrection) * Bn1.x;
				Bn.y = params.fCorrection * (Xn.y - Xn1.y) + (1 - params.fCorrection) * Bn1.y;
				Bn.z = params.fCorrection * (Xn.z - Xn1.z) + (1 - params.fCorrection) * Bn1.z;
				Bn.w = params.fCorrection * (Xn.w - Xn1.w) + (1 - params.fCorrection) * Bn1.w;
			}

			// Prediction
			Xnp.x = Xn.x + params.fPrediction * Bn.x;
			Xnp.y = Xn.y + params.fPrediction * Bn.y;
			Xnp.z = Xn.z + params.fPrediction * Bn.z;
			Xnp.w = Xn.w + params.fPrediction * Bn.w;

			// check MaxDeviationRadius, repair Xnp got Xno
			float predict_diff_distance = sqrt((Xnp.x - X.x)*(Xnp.x - X.x) + (Xnp.y - X.y)*(Xnp.y - X.y) + (Xnp.z - X.z)*(Xnp.z - X.z));;

			if (predict_diff_distance > params.fMaxDeviationRadius)
			{
				float EffectDevRatio = params.fMaxDeviationRadius / predict_diff_distance;
				assert(EffectDevRatio <= 1.0f);

				// X~((n+k)|n).x * EffectDevRatio + (1 - EffectDevRatio) * SkePos[i].x
				Xnpr.x = Xnp.x * EffectDevRatio + (1 - EffectDevRatio) * X.x;
				Xnpr.y = Xnp.y * EffectDevRatio + (1 - EffectDevRatio) * X.y;
				Xnpr.z = Xnp.z * EffectDevRatio + (1 - EffectDevRatio) * X.z;
				Xnpr.w = Xnp.w * EffectDevRatio + (1 - EffectDevRatio) * X.w;

				Xo.x = Xnpr.x; Xo.y = Xnpr.y; Xo.z = Xnpr.z; Xo.w = Xnpr.w;
			}
			else
			{
				Xo.x = Xnp.x; Xo.y = Xnp.y; Xo.z = Xnp.z; Xo.w = Xnp.w;
			}

			p_smoothingState->m_Bn[joint_i] = Bn;
			p_smoothingState->m_Xn[joint_i] = Xn;
			p_smoothingState->m_X[joint_i] = X;

			p_skeleton_data->SkeletonPositions[joint_i].x = Xo.x;
			p_skeleton_data->SkeletonPositions[joint_i].y = Xo.y;
			p_skeleton_data->SkeletonPositions[joint_i].z = Xo.z;
			p_skeleton_data->SkeletonPositions[joint_i].w = 1.0f;
		}
	}

	return S_OK;
}

/**
 *@xu-li:
 *��������classifyOnePixel
 *������
 *���������

 *���������

 *����ֵ��
 *����˵����
 *���ͼ���׵�ַ��const unsigned short * pImage
 *��ǰ���ص�yֵ��int y
 *��ǰ���ص�xֵ��int x
 *const float player_scale
 *�����Ч���const int DepthUnusual
 *ͼ�����������ص��31����λͶƱ�Ľ��ָ���С160*120*31��unsigned short * pLabelRaw
 */

void BodyPartClassifier::classifyOnePixel( const unsigned short * pImage, int y, int x, const float player_scale, const int DepthUnusual, unsigned short * pLabelRaw )
{
	// prepare
	const unsigned short * pdepth = pImage + (y*2) * 320;							 //@xu-li:x*2+(y*2)*320��һ�����ŵĲ�����ֻȡ320*240�е�ż��λ��

	unsigned short dx_with_player_index = pdepth[x*2];								
	unsigned short player_index = dx_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;//@xu-li:ȡ����λ�����;������ı����
	// check if foreground
	if (!player_index)
	{
		return;
	}
	FILEOUTPUT( file <<"pixel: "<<x <<", "<< y <<std::endl);

	unsigned short * class_dist = pLabelRaw + (y*160 + x ) * 31;					//@xu-li:class_dist��ָ��ĳһ�����ض���31����λ��ͶƱ���
#ifdef _DEBUG
	for (int i = 0; i < 31; i++)
	{
		assert(class_dist[i] == 0);
	}
#endif
	unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;		//@xu-li:ȡ�����ͼ
	assert(dx > 0 && dx < 12000);
	float dxinv = player_scale * 2048.0f / (float) dx; // player_scale * 2048.0 / dx;//@xu-li:��ʽ��������Դ��player_scale=1.0f,�˴�������ʽ��dx�ĵ���,��֪��Ϊʲô����ʽ2048.0f

	// set mask
	//m_pMask[y * 160 + x] = 1;

	// do rf predict
	for (int tree_id = 0; tree_id < 3; tree_id ++)
	{
		// transverst a dtree
		FILEOUTPUT( file << "tree_id:" << tree_id <<std::endl);
		 
		int node_id = 0;
		do
		{
			FILEOUTPUT( file << node_id <<std::endl) ;
			// get tree node
			const TreeNode * n = m_forest.Node(tree_id, node_id);					//@xu-li:ȡ�ڵ�
			ASSERT(n);
			// get node value
			int node_left_id  = n->left;	//@xu-li:�ڵ��������Ǵ˽ڵ��������������ڵ��id�źͼ���˽ڵ������ֵ�����ص��ƫ�������Լ���Ȳ����ֵ
			int node_right_id = n->right;
			signed char ux = n->ux;			//@xu-li:ux/uy/vx/vy��ѵ�����ݣ�ָ���Ǵӵ�ǰ���ƫ��������������Щƫ��������һ��dx�ĵ���dxinv������һ���������ȶ�ƫ������Ӱ��
			signed char uy = n->uy;
			signed char vx = n->vx;
			signed char vy = n->vy;
			short c = n->c;

			// calc p1, p2
			int p1x = (int)(((float)ux * dxinv) + (x*2));//@xu-li:��һ��ƫ�ƺ����ص��x,x*2��ͼ����С��Ե��
			int p1y = (int)(((float)uy * dxinv) + (y*2));//@xu-li:��һ��ƫ�ƺ����ص��y
			int p2x = (int)(((float)vx * dxinv) + (x*2));//@xu-li:�ڶ���ƫ�ƺ����ص��x
			int p2y = (int)(((float)vy * dxinv) + (y*2));//@xu-li:�ڶ���ƫ�ƺ����ص��y

			// calc dp1, dp2
			int dp1 = -1, dp2 = -1;

			dp1 = DepthUnusual;
			if (p1x >= 0 && p1x < 320 && p1y >= 0 && p1y < 240)
			{
				unsigned short dp1_with_player_index = pImage[p1y * 320 + p1x];
				unsigned short player_index_p1 = dp1_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
				if (player_index_p1 == player_index)
				{
					dp1 = dp1_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
					assert(dp1 > 0 && dp1 < 12000);
				}
			}

			if (vx == 0 && vy == 0)
			{
				dp2 = dx;
			}
			else
			{
				dp2 = DepthUnusual;
				if (p2x >= 0 && p2x < 320 && p2y >= 0 && p2y < 240)
				{
					unsigned short dp2_with_player_index = pImage[p2y * 320 + p2x];
					unsigned short player_index_p2 = dp2_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
					if (player_index_p2 == player_index)
					{
						dp2 = dp2_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
						assert(dp2 > 0 && dp2 < 12000);
					}
				}
			}

			//printf("node_id = %8d, uv = {%4d, %4d, %4d, %4d}, p1 = {%4d, %4d}, p2 = {%4d, %4d}, dp1 = %4d, dp2 = %4d, c = %5d, node_id_son = %8d\n", node_id, ux, uy, vx, vy, p1x, p1y, p2x, p2y, dp1, dp2, c, (dp1 - dp2) < (int)c ? node_left_id : node_right_id);
			// compare with threshold, make left or right
			if ((dp1 - dp2) < (int)c)//@xu-li:����ƫ�����ص����Ȳ�
			{
				node_id = node_left_id;
			}
			else
			{
				node_id = node_right_id;
			}
		}
		while (node_id > 0);
		FILEOUTPUT( file << "class_dist:"<<std::endl);
		// get node value, sum class_dist
		assert(node_id < 0);
		int value_id = -(node_id + 1);
		const NodeValue * v = m_forest.Value(value_id);
		assert(v->v[0].cnt + v->v[1].cnt + v->v[2].cnt + v->v[3].cnt + v->v[4].cnt <= 255);
		for (int vi = 0; vi < 5; vi++)
		{
			// get tree node


			unsigned char id  = v->v[vi].id;	//@xu-li:�����ص����Ĳ�λid
			assert(id < 31);
			//assert(id > 0);
			unsigned char cnt = v->v[vi].cnt;	//@xu-li:��Ӧ���ಿλ��ͶƱ��
			//assert(cnt > 0);
			class_dist[id] += cnt;
			assert(class_dist[id] <= 3 * 255);
			FILEOUTPUT( file << "id , cnt:" << (int)id <<","<<(int)cnt<< std::endl);
		}
	}
}

//void BodyPartClassifier::clearResult()
//{
//	memset(m_pLabelRaw, 0, 160*120*31*2);
//	memset(m_pMask, 0, 160*120);
//	 memset(pLabel, 0, 160*120);
//}


void BodyPartClassifier::maxClass( unsigned short * class_dist, int &max_cnt, int &max_class )
{
	max_class = -1;
	max_cnt = -1;
	for (int ci = 0; ci < 31; ci ++)
	{
		assert(class_dist[ci] <= 3*255);  //3����, 3X3=9������
		unsigned short cnt = class_dist[ci];

		if (cnt > 0)
		{
			if (max_cnt <= cnt)
			{
				max_cnt = cnt;
				max_class = ci;
			}
			else if (max_cnt == cnt)
			{
				// �������cntһ��???
				//assert(0);
			}
		}
	}
}



void BodyPartClassifier::classifyOnePixelClass( const unsigned short * pImage, int y, int x, const float player_scale, const int DepthUnusual, int * maxclass , unsigned short * pLabelRaw )
{
	//	clearResult();
	classifyOnePixel(pImage, y, x, player_scale, DepthUnusual,pLabelRaw);
	unsigned short class_dist[31];
	memcpy(class_dist, pLabelRaw + (y*160 + x) * 31, 31*sizeof(short));
	int maxcnt;
	maxClass(class_dist, maxcnt, *maxclass);
}

void BodyPartClassifier::PredictRawGpu( const float player_scale, const unsigned short * pImage , unsigned short * pLabelRaw )
{
//	throw std::exception("The method or operation is not implemented.");
}

void BodyPartClassifier::init()
{
	//	m_context = NULL;
	//cl_m_pfBodyPartProbility = NULL;
	//m_pvCoordWorldSpace   = new Vector4[120*160];
	//ASSERT(m_pvCoordWorldSpace);
	//m_pfBodyPartProbility = new float[6*31*120*160];
	//ASSERT(m_pfBodyPartProbility);
	m_iBodyPartProbilityStep = 120 * 160 * 31;

	//ƽ������Ĭ��ֵ
	m_defaultSmoothingParams.fSmoothing          = 0.5f;
	m_defaultSmoothingParams.fCorrection         = 0.5f;
	m_defaultSmoothingParams.fPrediction         = 0.5f;
	m_defaultSmoothingParams.fJitterRadius       = 0.05f;
	m_defaultSmoothingParams.fMaxDeviationRadius = 0.04f;

	//@xu-li:������
	filter = new Filter();
	memset(isfirstTime, 1,  20*6);
	

}

//uchar s_labelRawChar[120][160][32];
//extern int g_folder;
//extern int g_framecnt;
void BodyPartClassifier::rawToLabel( unsigned char * pLabel, const unsigned short * pImage, int labelConvMode, unsigned short * pLabelRaw )
{
	memset(pLabel, 0, 160*120);
//#ifdef _DEBUG
//	memset((char*)&s_labelRawChar[0][0][0], 0, 160 * 120 * 32);
//#endif
	for (int y = 1; y < 119; y ++)
	{
		for (int x = 1; x < 159; x ++)
		{
			// prepare
			const unsigned short * pdepth = pImage + (y*2) * 320;
			unsigned char  * plabel = pLabel + y * 160;
			unsigned short class_dist[31];


			unsigned short dx_with_player_index = pdepth[x*2];
			unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
			unsigned short player_index = dx_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
			assert(player_index < 8);
			// check if foreground
			if (!player_index)
			{
				plabel[x] = 0;
				continue;
			}

			//int same_player_pixel_count = GetClassDistWithNeighbor3X3(pImage, (unsigned short *)class_dist, y, x, labelConvMode, pLabelRaw);

			// post process of class_dist, make labelimage
			int max_class = -1;
			int max_cnt = -1;
			memcpy(class_dist, pLabelRaw + (y*160 + x) * 31, 31*sizeof(short));
			maxClass(class_dist, max_cnt, max_class);

			assert(max_cnt > 0 && max_class >= 0);
			plabel[x] = max_class + 1;

//#ifdef _DEBUG
//
//			for (int ci = 0; ci < 31; ci ++)
//			{
//				assert(class_dist[ci] <= 3*255);  //3����, 3X3=9������
//				s_labelRawChar[y][x][ci] = class_dist[ci] / 3;
//			}
//			s_labelRawChar[y][x][31] = player_index;
//#endif
		}
	}

//#ifdef _DEBUG
//	fstream file;
//	char filename[256];
//	sprintf(filename, "D:/workspace/microsoftData/SingleFile%d/labelraw%d.labelraw", g_folder, g_framecnt);
//	file.open(filename, ios_base::out|ios_base::binary);
//	file.write((char*)&s_labelRawChar[0][0][0], 160 * 120 * 32);
//	file.close();
//#endif

}

/**
*@xu-li:
*�������ƣ�initCentroidpoint
*�����������ҳ�ʼ���ĵ�λ�ã���ʼ����Ҫ�ı���
*���������
*		1.const unsigned short * pImage		���ͼ����������;
*		2.const unsigned short * pLabelRaw	��λ����������31����λͶƱ���;
*���������
*		1.NUI_CENTROIDS_STAGE_DATA * pData	���ĵ�ָ�룬�����ʼ���ĵ㣻
*		2.int iCount[6][31]					ÿ����ÿ����λ���������ص������ͳ��;
*		3.NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6];	�˵ĸ���״̬����һ�����ش�������Ϊp>0.14����Ϊ�����ҵ���
*		4.NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31];	��λ������״̬����һ�����ش�������Ϊp>0.14����Ϊ�˲�λ�ҵ���
*		5.Vector4 vCentroid[6][31]			���6��ǰ����31����λ�����ĵ����������;
*		6.Vector4 * m_pvCoordWorldSpace		�������������ȵ��������ص���������������
*		7.float * m_pfBodyPartProbility		���6��ǰ����ÿ�����ص��ÿ����λ�ĸ��ʣ�
*����ֵ��void��
*/
inline void BodyPartClassifier::initCentroidpoint( const unsigned short * pImage, const unsigned short * pLabelRaw, 
												  NUI_CENTROIDS_STAGE_DATA * pData , int iCount[6][31], NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6], 
												  NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31], Vector4 vCentroid[6][31] ,
												  Vector4 * m_pvCoordWorldSpace, float * m_pfBodyPartProbility)
{
	// 6���˵Ĳ�λ����״̬
	//NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6];
	// 6����31����λ�����ĵ�
	//Vector4 vCentroid[6][31];
	// 6����31����λ�����ظ���
	///int iCount[6][31];
	// 6����31����λ�ĸ���״̬
	//NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31];
	//Vector4  m_pvCoordWorldSpace[120][160]
	// float  m_pfBodyPartProbility[6][120][160][31]
	{
		MEASURE_TIME(Util tu("CentroidsStage_RunFrame: ��ʼ���ڴ�"));
		memset(m_pvCoordWorldSpace, 0, 120*160*sizeof(Vector4));
		if (!g_NoHugeArray)
		{
			memset(m_pfBodyPartProbility, 0, 6*31*120*160*sizeof(float));
		}

		memset(eBodyPartTrackingState, 0, 6*sizeof(NUI_BODY_PART_TRACKING_STATE));
		memset(vCentroid, 0, 6*31*sizeof(Vector4));
		memset(iCount, 0, 6*31*sizeof(int));
		memset(eBodyPartPositionTrackingState, 0, 6*31*sizeof(NUI_BODY_PART_POSITION_TRACKING_STATE));
		MEASURE_TIME(g_initCentroidMem = tu.time());
	}

	{
		MEASURE_TIME(Util tu("CentroidsStage_RunFrame: ��ʼ���ĵ�"));

//		#pragma omp parallel for if(parallelism_enabled) 
		for (int y = 0; y < 120; y ++)
		{
			for (int x = 0; x < 160; x ++)
			{
				// prepare
				const unsigned short * pdepth = pImage + (y * 2) * 320;
				unsigned short dx_with_player_index = pdepth[x * 2];
				unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
				unsigned short player_index = dx_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
				unsigned short pid = player_index - 1; // for access vCentroid & iCount & eTrackingState
				if (!player_index)						//@xu-li:��Ҫ������
				{
					continue;
				}

				// get p(c|u)
				const unsigned short * pClassDist = pLabelRaw + (y * 160 + x) * 31;//@xu-li:��ǰ���ص�31��ͶƱ���ݵ��׵�ַ

				// make label image
				// nothing

				// X(u) = [x(u), y(u), z(u)]
				// ��ǰ��������������ϵ������(XYZ)
				Vector4 X;
				ImageToWorldSpace(Vector4I(x, y, dx), X);//�ռ䵥λ����

				// push back 0.039m, save Z
				//X.z -= 0.039f;
				//Z[y * 160 + x] = X.z;
				m_pvCoordWorldSpace[y * 160 + x] = X;//@xu-li:���Ǵ˱�������ʼλ�ã����ｫ�����ϵ�����ȵĵ�ת��Ϊ��������ϵ������Ŵ���������

				// compute m_pfBodyPartProbility, W = m_pfBodyPartProbility / Z^2;
				int offset = y * 160 + x;
				for (int bpid = 0; bpid < 31; bpid ++)
				{
					float p;
					p = pClassDist[bpid] / (3.0f*255.0f);	//@xu-li:�������
					if (!g_NoHugeArray)
					{
						m_pfBodyPartProbility[pid * m_iBodyPartProbilityStep + bpid * 120 * 160 + offset] = p;
						//@xu-li:�ɴ˴��洢��ʽ����ÿһ���˵�ͼ�񵥶�����һ��ͼ���У�����ÿ�����ص��31����λ��ÿһ����λ�ĸ���ֵ����һ��ͼƬ��
						//@xu-li:Ҳ����˵�����ݿռ�m_pfBodyPartProbility�Ĵ�ŷ�ʽ�ǣ�1��ǰ��ÿһ�����ضԵ�1����λ�ĸ���ֵ������Ϊ0��+1��ǰ��ÿһ�����ضԵ�2����λ�ĸ���ֵ������Ϊ0��+����+1��ǰ��ÿһ�����ضԵ�31����λ�ĸ���ֵ������Ϊ0��+2��ǰ��ÿһ�����ضԵ�1����λ�ĸ���ֵ������Ϊ0��+����+6��ǰ��ÿһ�����ضԵ�31����λ�ĸ���ֵ������Ϊ0��
						//@xu-li:��31*6����СΪ160*120�ľ������һ��
					}
					// initialize centroid
					Vector4 * c = &vCentroid[pid][bpid];	//@xu-li:pidΪǰ����ʶ���ˣ���bpidΪ31����λ������
					if (p > 0.14)							//@xu-li:��Ϊ�õ��ڸò�λ�ĸ�����ֵ
					{
						c->x += (X.x * 1);					//@xu-li:��ͬһ��λ���������ص����ά�ռ������ۼӣ��Ա����������ĵ㸳��ֵʹ�ã�����ֵ�ô��ۼ�ֵ��ƽ��
						c->y += (X.y * 1);					//@xu-li:�������1����
						c->z += (X.z * 1);
						iCount[pid][bpid] += 1;				//@xu-li:���˸ò�λ�ϸ���������ֵ�����ص��ܸ�������
						eBodyPartPositionTrackingState[pid][bpid] = NUI_BODY_PART_POSITION_TRACKED;//@xu-li:�����õģ���������ص��ĳ���˵Ĵ˲�λ���ʴ���0.14����Ϊ�ò�λ��λ�ø������ˣ���ô�������֡ͼ�����������ضԸò�λ��С��0.14�������λ��λ��û�и�����
						eBodyPartTrackingState[pid] = NUI_BODY_PART_POSISION_ONLY;//@xu-li:����е����ɣ��κ�һ��������pid����˵�����31��λ�е��κ�һ���ϣ���ֵ�ͱ�����ΪNUI_BODY_PART_POSISION_ONLY������ǲ�֪������;��������
					}
					else
					{
						if (!g_NoHugeArray)
						{
							m_pfBodyPartProbility[pid * m_iBodyPartProbilityStep + bpid * 120 * 160 + offset] = 0.0f;//@xu-li:��������ֵ�ĸ���ֱ�Ӹ�ֵΪ0����ô�ں�����������������m_pfBodyPartProbility�����ضԲ�λ�ĸ���ֵ���˴���0.14�ģ�ʣ�µ�ȫΪ0
						}
					}
				}
			}

		}

		//�õ�����λ�ĳ�ʼ���ĵ�
		centroidReduction(pData, iCount, eBodyPartPositionTrackingState,  vCentroid);

		memcpy(g_vInitedCentroid_copy, vCentroid, 6 * 31 * sizeof(Vector4));
		MEASURE_TIME(g_initCentroid = tu.time());
	}
}

inline void BodyPartClassifier::centroidReduction( NUI_CENTROIDS_STAGE_DATA * pData, int iCount[6][31], 
												  NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31], Vector4 vCentroid[6][31] )
{
	for (int pid = 0; pid < 6; pid ++)
	{
		for (int bpid = 0; bpid < 31; bpid ++)
		{
			if (g_doMeanshift)
			{
				/**/
				if (pData->BodyPartCentroids[pid].eBodyPartPositionTrackingState[bpid] == NUI_BODY_PART_POSITION_TRACKED)//@xu-li:�ж�����ʲô���壿
				{
					vCentroid[pid][bpid] = pData->BodyPartCentroids[pid].Centroids[bpid];
				}
				else  if (eBodyPartPositionTrackingState[pid][bpid] == NUI_BODY_PART_POSITION_TRACKED)
				{
					ASSERT(iCount[pid][bpid] > 0);
					vCentroid[pid][bpid].x /= iCount[pid][bpid];	//@xu-li:��ͬһ��λ���е���������ľ�ֵ��Ϊ��ʼ���ĵ�
					vCentroid[pid][bpid].y /= iCount[pid][bpid];
					vCentroid[pid][bpid].z /= iCount[pid][bpid];
					assert(iCount[pid][bpid] > 0);
				}
				else
				{
					//TODO: û�����������ĳ�ʼ���ĵ�
					vCentroid[pid][bpid].x = 0;
					vCentroid[pid][bpid].y = 0;
					vCentroid[pid][bpid].z = 0;
				}
			} 
			else
			{
				if (eBodyPartPositionTrackingState[pid][bpid] == NUI_BODY_PART_POSITION_TRACKED)
				{
					ASSERT(iCount[pid][bpid] > 0);
					vCentroid[pid][bpid].x /= iCount[pid][bpid];
					vCentroid[pid][bpid].y /= iCount[pid][bpid];
					vCentroid[pid][bpid].z /= iCount[pid][bpid];
					assert(iCount[pid][bpid] > 0);
				}
				else
				{
					//TODO: û�����������ĳ�ʼ���ĵ�
					vCentroid[pid][bpid].x = 0;
					vCentroid[pid][bpid].y = 0;
					vCentroid[pid][bpid].z = 0;
				}
			}

		}
	}
}
/**
*@xu-li:
*�������ƣ�meanshift
*�����������Ҳ�λ���ĵ�λ�ã����������ֵ
*���������
*		1.Vector4 vCentroid[6][31]			���6��ǰ����31����λ�ĳ�ʼ���ĵ����������;
*		2.NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31];	��λ������״̬����һ�����ش�������Ϊp>0.14����Ϊ�˲�λ�ҵ���
*		3.NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6];						�˵ĸ���״̬����һ�����ش�������Ϊp>0.14����Ϊ�����ҵ���
*		4.int iCount[6][31]					����Ըò�λ�ĸ���p>0.14���������ص�����
*		5.Vector4 * m_pvCoordWorldSpace		�������������ȵ��������ص���������������
*		6.float * m_pfBodyPartProbility		���6��ǰ����ÿ�����ص��ÿ����λ�ĸ��ʣ�
*���������
*		1.Vector4 vCentroid[6][31]			6��31����λ���ĵ��������������
*		2.
*		3.
*		4.
*		5.
*		6.
*		7.
*����ֵ��void��
*/
void BodyPartClassifier::meanshift( Vector4 vCentroid[6][31],  NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31],
								   NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6], NUI_CENTROIDS_STAGE_DATA * pData, int iCount[6][31],
								   Vector4 * m_pvCoordWorldSpace, float * m_pfBodyPartProbility, const unsigned short * pLabelRaw, const unsigned short * pImage)
{
#define AVERAGE_BAND_WIDTH 0.065f

	{
		MEASURE_TIME(Util tu("CentroidsStage_RunFrame: �������ĵ�"));
		//////////////////////�˲�������//////////////////////////////////////////////
		//@xu-li:�Ը��ʽ���ƽ���˲��Ĳ���
		//float filter[] = {1.0,2.0,1.0,2.0,4.0,2.0,1.0,2.0,1.0};	 //�˲�������
		//float tmp[]    = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};   //�м�����
		//float *filtered;//zeros(120, 160);  //�˲����ͼ
		//float filter_out = 0;
		//filtered = (float*)malloc(sizeof(float)*120*160);
		//////////////////////�˲�������//////////////////////////////////////////////

		for (int pid = 0; pid < 6; pid ++)

			for (int bpid = 0; bpid < 31; bpid ++)

			{
				static float BC[16] = {8 * AVERAGE_BAND_WIDTH,//@xu-li:δʹ��BC[]
					4 * AVERAGE_BAND_WIDTH,
					3 * AVERAGE_BAND_WIDTH,
					2 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH,
					1 * AVERAGE_BAND_WIDTH};

				float * pP = NULL;
				if (!g_NoHugeArray)
				{
					//����һ�¸������ݵĴ洢�ṹ
					//m_pfBodyPartProbilityΪ�����׵�ַ
					//����ֳ�6�����ݣ�ÿ��Ϊһ���˵�����
					//��ÿһ���˵������У��ֳַ�31�죬����ÿһ��Ϊÿһ����λ�ĸ��ʣ����31���ÿһ����Ե���һ��ͼ�����ͼ��Ϊ����ͼ���������ص��ͬһ����λ�ĸ��ʷֲ�
					pP = m_pfBodyPartProbility + pid * m_iBodyPartProbilityStep + bpid * 120 * 160;//@xu-li:�������ضԵ�һ��λ������ͼ����׵�ַ//m_iBodyPartProbilityStep = 120 * 160 * 31;
					
				}

				////////////////////////�˲�/////////////////////////////////////////////////////////////////
				///���˲��ڴ˻�Ƚ��鷳����ΪҪ��31����λ������Ҳ����ÿһ֡ͼ������31������ͼ��Ŀռ�ƽ������������
				///���Կ�������ǰ�ƣ������ɴ˸��ʽ����ʱ������˲�
				//@xu-li:�Ը��ʽ���ƽ���˲�
			        
				
				//memset(filtered, 0, sizeof(float)*120*160);
				//for (int y = 1 ; y < 119 ; y++)
				//{       
				//	for (int x = 1; x < 159; x++)
				//	{
    //                    if( pP[( y )*160 + ( x ) ] <= 0)
				//			continue;
				//		else
				//		{
				//			tmp[1] = pP[(y-1)*160 + (x-1)] ;
				//			tmp[2] = pP[(y-1)*160 + ( x )] ;
				//			tmp[3] = pP[(y-1)*160 + (x+1)] ;
				//			tmp[4] = pP[( y )*160 + (x-1)] ;
				//			tmp[5] = pP[( y )*160 + ( x )] ;
				//			tmp[6] = pP[( y )*160 + (x+1)] ;
				//			tmp[7] = pP[(y+1)*160 + (x-1)] ;
				//			tmp[8] = pP[(y+1)*160 + ( x )] ;
				//			tmp[9] = pP[(y+1)*160 + (x+1)] ;

				//			for (int n = 0; n<9; n++)
				//			{
				//					filter_out +=  tmp[n]*filter[n];
				//			}
				//			filter_out = filter_out/16;
    //            
				//			filtered[y*160 + x] = filter_out;
				//		}

				//	}
				//}
				////////////////////////�˲�/////////////////////////////////////////////////////////////////



				Vector4 c_old, c = vCentroid[pid][bpid];
				NUI_BODY_PART_POSITION_TRACKING_STATE state = eBodyPartPositionTrackingState[pid][bpid];

				if (state == NUI_BODY_PART_POSITION_TRACKED)
				{
					const int initHalfLong = 10;
					int halfsideLong = initHalfLong;
					bool lastSumkZero = false;
					int loop = 0;
					do
					{
						double sum_k = 0;
						Vector4 sum_kX;

						// my improve imperiment
						Vector4I imageC;
						WorldToImageSpace(c, imageC);

						int ybegin = imageC.y - halfsideLong;//@xu-li:�Գ�ʼ���ĵ�Ϊ����ȡ2*halfsideLong�߳��Ŀ���Ϊ���㷶Χ
						int yend = imageC.y + halfsideLong;
						int xbegin = imageC.x - halfsideLong;
						int xend = imageC.x + halfsideLong;
						if (ybegin < 0)
						{
							ybegin = 0;
						}
						if (yend >= 120)
						{
							yend = 120;
						}
						if (xbegin < 0)
						{
							xbegin = 0;
						}
						if (xend >= 160)
						{
							xend = 160;
						}
						//���ܸĽ��Ĺؼ���������2��forѭ����Χ�ı仯��
						//for (int y = 0; y < 120; y ++)
						//{
						//    for (int x = 0; x < 160; x ++)
						//    {
						for (int y = ybegin; y < yend; y ++)		//@xu-li:���ٱ�Եɨ�跶Χ
						{
							for (int x = xbegin; x < xend; x ++)
							{
								
								float p;
								if (!g_NoHugeArray)
								{
									assert(pP);
									p= pP[y * 160 + x];//todo pLabelRaw[y * 160 + x]
									//assert(filtered);//@xu-li:�Ը��ʽ���ƽ���˲��Ľ��ʹ��
									//p = filtered[y*160+x];
								} 
								else
								{
									p =  getLabelRawAt(pid, bpid, x, y, pLabelRaw, pImage);
								}
								
								
								if (p < FLT_MIN && p > -FLT_MIN)		//@xu-li:������0�ļ���Сֵ��Χ
								{
									continue;
								}
								// X: ��ǰ��������������ϵ������
								Vector4 X = m_pvCoordWorldSpace[y * 160 + x];		//@xu-li:X�ǵ�ǰ���ص���������������c�ǵ�ǰ���ĵ�������������
								double distance_2_centroid = (X.x - c.x)*(X.x - c.x) + (X.y - c.y)*(X.y - c.y) + (X.z - c.z)*(X.z - c.z);//@xu-li:��Χ�ڵ��������ص��������ĵ��ŷ�Ͼ����ƽ��

								double k = p * X.z * X.z * exp(-1*distance_2_centroid/(64 * AVERAGE_BAND_WIDTH * AVERAGE_BAND_WIDTH/*BC[loop]*BC[loop]*/));//@xu-li:�˴�k�ĺ�Ϊ�ܶȹ��ƣ��˺���Ϊ��˹�˺���beta=1/(64 * AVERAGE_BAND_WIDTH * AVERAGE_BAND_WIDTH)��

								sum_k += k;								//@xu-li:k�ĺͣ���sum_kΪ�ܶȹ��ƣ�������õļ��������΢��������ʹ�õ�
								sum_kX.x += static_cast<float>(k * X.x);
								sum_kX.y += static_cast<float>(k * X.y);
								sum_kX.z += static_cast<float>(k * X.z);
							}
						}
						//assert(sum_k > 0.00001);
						if (sum_k < 0.00001)
						{
							if (lastSumkZero)
							{
								//assert(0);
								break;
							}
							lastSumkZero = true;
							halfsideLong = 160;
							//loop--;
							continue;
							//break;
						}
						if (halfsideLong == 160)
						{
							halfsideLong = initHalfLong;
						}
						c_old = c;
						c.x = static_cast<float>(sum_kX.x / sum_k);//@xu-li�������ĵ������ɲο�����������λ���㷨����������xc=sum(mi*xi)/sum(mi),yc=sum(mi*yi)/sum(mi),zc=sum(mi*zi)/sum(mi)
						c.y = static_cast<float>(sum_kX.y / sum_k);
						c.z = static_cast<float>(sum_kX.z / sum_k);

						double distance = (c.x-c_old.x)*(c.x-c_old.x) + (c.y-c_old.y)*(c.y-c_old.y) + (c.z-c_old.z)*(c.z-c_old.z);
						if (distance <= 0.001*0.001 )
						{
							break;
						}

					} while (++loop < 16);

					// push back 0.039m
					if (c.z != 0)
					{
						c.z -= 0.039f;
					}
					vCentroid[pid][bpid] = c;//@xu-li:���ϼ����֪vCentroid�д�ŵ������ĵ�λ����Ϣ����������ϵ�µ���Ϣ
					eBodyPartTrackingState[pid] = NUI_BODY_PART_POSISION_ONLY;
				}
				else
				{
					vCentroid[pid][bpid].x = 0.0f;
					vCentroid[pid][bpid].y = 0.0f;
					vCentroid[pid][bpid].z = 0.0f;
					vCentroid[pid][bpid].w = 0.0f;
				}
			}

			MEASURE_TIME(g_calcCentroid = tu.time());
			//free(filtered);//@xu-li:�Ը��ʽ���ƽ���˲��Ľ��ָ���ͷ�
	}
}
/*
**************���������ʼûʲô��Ҫ������뽫�������ͳһ����������ǰ������ʱ������pData���ָ�룬Ȼ���Ա�����ʱ�򵥶�����ָ��ָ����س�Ա���д��Ż������������ѭ��
*
*����������ת�ƣ����˵ĸ���״̬eBodyPartTrackingState����λ�ĸ���״̬eBodyPartPositionTrackingState�����ĵ����������������vCentroid����������p>0.14�����ص�����iCount������pData�µ�BodyPartCentroids�ṹ����
*���������
*		1.NUI_BODY_PART_TRACKING_STATE * eBodyPartTrackingState		�˵ĸ���״̬
*		2.Vector4 vCentroid[6][31]									��λ���ĵ����������������
*		3.int iCount[6][31]											����λ����p>0.14�����ص�����
*		4.NUI_BODY_PART_POSITION_TRACKING_STATE  eBodyPartPositionTrackingState[6][31]	��λ�ĸ���״̬
*���������
*		1.NUI_CENTROIDS_STAGE_DATA * pData							���岿λ���ĵ��������ͼ��ϣ������ĸ�����ͳһ����һ��ָ����
*
*/
inline void BodyPartClassifier::vCentroid2BodyPartCentroids
	( NUI_CENTROIDS_STAGE_DATA * pData, 
	NUI_BODY_PART_TRACKING_STATE * eBodyPartTrackingState, Vector4 vCentroid[6][31], 
	int iCount[6][31], NUI_BODY_PART_POSITION_TRACKING_STATE  eBodyPartPositionTrackingState[6][31] )
{
	for (int pid = 0; pid < 6; pid ++)
	{
		pData->BodyPartCentroids[pid].eBodyPartTrackingState = eBodyPartTrackingState[pid];
		for (int bpid = 0; bpid < 31; bpid ++)
		{
			pData->BodyPartCentroids[pid].Centroids[bpid]      = vCentroid[pid][bpid];
			pData->BodyPartCentroids[pid].PixelCount[bpid]     = iCount[pid][bpid];
			pData->BodyPartCentroids[pid].eBodyPartPositionTrackingState[bpid] = eBodyPartPositionTrackingState[pid][bpid];
		}
	}
}

float BodyPartClassifier::getLabelRawAt( int pid, int bpid, int x, int y, const unsigned short * pLabelRaw, const unsigned short * pImage )
{
	const unsigned short * pdepth = pImage + (y * 2) * 320;
	unsigned short dx_with_player_index = pdepth[x * 2];
//	unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index = dx_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
	//unsigned short pid = player_index - 1; // for access vCentroid & iCount & eTrackingState
	if (player_index - 1 != pid)
	{
		return 0.0f;
	}

	// get p(c|u)
	const unsigned short * pClassDist = pLabelRaw + (y * 160 + x) * 31;
	return pClassDist[bpid] / (3.0f*255.0f);;
}


