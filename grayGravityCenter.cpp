// gray_gravity_center.cpp : Defines the entry point for the console application.
//

//

#include <fstream>
#include <windows.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <cstring>
#include "Util.h"
#include "NuiSinect.h"
#include "RandomForest.h"
#include "BodyPartClassifier.h"
using namespace std;

#define MEASURE_TIME(X) X

#define NUI_IMAGE_PLAYER_INDEX_SHIFT 3
#define NUI_IMAGE_PLAYER_INDEX_MASK  ((USHORT)0x0007)
//#define NUI_BODY_PART_POSITION_TRACKED 1

static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef float          FLOAT;

extern double g_initCentroid ;
extern double g_calcCentroid ;
extern double g_initCentroidMem ;
//typedef struct _Vector4
//{
//	_Vector4()
//		: x(0), y(0), z(0), w(0)
//	{}
//	FLOAT x;
//	FLOAT y;
//	FLOAT z;
//	FLOAT w;
//} Vector4;

typedef struct _Label
{
	int num;	//label号：1～186
	float weight;	//该label号的权重系数
} Label;		//部位分类结果图中，每个像素点的数据结构

typedef struct _Gcenter
{
	float x;		//重心点的x坐标
	float y;		//重心点的y坐标
	float z;		//重心点的z坐标
	float sum_wx;	//x与weight的乘积的累加和
	float sum_wy;	//y与weight的乘积的累加和
	float sum_wz;	//y与weight的乘积的累加和
	float sum_w;	//属于该部位的点的weight的累加和
} Gcenter;		//每个部位的重心

/*
 *	坐标转换，图像坐标系转换为世界坐标系
 */
void ImageToWorldSpace1(int x, int y, int z, Vector4 & dst, int height = 120, int width = 160);
/*
 *	坐标转换，世界坐标系转换为图像坐标系
 */
void WorldToImageSpace(const Vector4 & src, int & x, int & y, int & z, int height = 120, int width = 160);

void ImageToWorldSpace1(int x, int y, int z, Vector4 & dst, int height/* = 120*/, int width/* = 160*/)
{
	//float const  NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 = 3.501f/1000;
	assert(height >= 120);
	assert(width >= 160);

	dst.z = z * 0.001f;
	dst.x = (x - width/2.0f) * (320.0f/width) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * dst.z;
	dst.y = - (y - height/2.0f) * (240.0f/height) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * dst.z;
	dst.w = 0;
}


void WorldToImageSpace(const Vector4 & src, int & x, int & y, int & z, int height/* = 120*/, int width/* = 160*/)
{
	//static const float NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 = 285.63f;
	assert(height >= 120);
	assert(width >= 160);

	if(src.z > FLT_EPSILON)
	{
		x = (int)(width/2 + src.x * (width/320.0) * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / src.z);
		y = (int)(height/2 - src.y * (height/240.0) * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / src.z);
		z = (int)(src.z * 1000);
	}
	else
	{
		x = 0;
		y = 0;
		z = 0;
	}
}

void BodyPartClassifier::GrayGravityCentroid(const USHORT * pImage, const unsigned short * pLabelRaw, Vector4 vCentroid[6][31],
											 NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31],
											 NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6])
{
	// 时间统计用
	double t0, t1, t2, t3;

	// 6个人31个部位的中心点
	//Vector4 vCentroid[6][31];
	memset(eBodyPartTrackingState, 0, 6*sizeof(NUI_BODY_PART_TRACKING_STATE));
	memset(vCentroid, 0, 6*31*sizeof(Vector4));

	memset(eBodyPartPositionTrackingState, 0, 6*31*sizeof(NUI_BODY_PART_POSITION_TRACKING_STATE));
	//部位分类图结果
	Label * m_plLabelImage = new Label[120*160];
	//重心点的计算结果和中间存储
	Gcenter * m_pgTempCenter = new Gcenter[6*31];
	//深度图的低3位
	unsigned short * m_pusDepthLow3 = new unsigned short[240*320];
	//深度图的低3位的抽点图
	unsigned short * m_pusDepthLow3Quarter = new unsigned short[120*160];

	{
		MEASURE_TIME(Util tu("CentroidsStage_RunFrame: 初始化内存"));
		//t0 = GetTimeStampUS();
		memset(m_plLabelImage, 0, 120*160*sizeof(Label));
		memset(m_pgTempCenter, 0, 6*31*sizeof(Gcenter));        
		memset(m_pusDepthLow3, 0, 240*320*sizeof(unsigned short));
		memset(m_pusDepthLow3Quarter, 0, 120*160*sizeof(unsigned short));      
		//memset(&vCentroid, 0, 6*31*sizeof(Vector4));
		MEASURE_TIME(g_initCentroidMem = tu.time());
	}

	{
		MEASURE_TIME(Util tu("CentroidsStage_RunFrame: 初始中心点"));
		//t1 = GetTimeStampUS();
		//提取深度图
		for (int y = 0; y < 240; y ++)
		{
			for (int x = 0; x < 320; x ++)
			{
				*(m_pusDepthLow3 + y * 320 + x) = ((*(pImage + y * 320 + x)) & 0x7);
				if ((y%2) == 0 && (x%2) == 0)
					*(m_pusDepthLow3Quarter + (y/2 * 160) + x/2) = *(m_pusDepthLow3 + y * 320 + x);
			}
		}

		//遍历部位分类图
		for (int y = 0; y < 120; y ++)
		{
			for (int x = 0; x < 160; x ++)
			{
				// prepare
				const unsigned short * pdepth = pImage + (y * 2) * 320;
				unsigned short dx_with_player_index = pdepth[x * 2];
				unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

				// 当前像素在世界坐标系的坐标(XYZ)
				Vector4 X;
				ImageToWorldSpace1(x, y, dx, X);

				// 当前坐标每个部位点的概率
				//const unsigned char * pClassDist = pLabelRaw + (y * 160 + x) * 32;
				const unsigned short * pClassDist = pLabelRaw + (y * 160 + x) * 31;
				// 部位的初始参数清零
				((m_plLabelImage + (y * 160 + x))->weight) = 0;
				((m_plLabelImage + (y * 160 + x))->num) = 0;

				// 找出每个像素点的最大概率部位号，并记录其概率
				for (int i = 0; i < 31; i ++)
				{
					assert(*(pClassDist + i) / (3.0f*255.0f) <= 1.0f);
					if (((m_plLabelImage + (y * 160 + x))->weight) < (*(pClassDist + i) / (3.0f*255.0f)))
					{
						((m_plLabelImage + (y * 160 + x))->weight) = (*(pClassDist + i) / (3.0f*255.0f));
						((m_plLabelImage + (y * 160 + x))->num) = i;
					}
				}
				//
				unsigned short h_num = *(m_pusDepthLow3Quarter + y * 160 + x);	//表示第几个人
				int l_num = (m_plLabelImage + (y * 160 + x))->num;	//部位号
				float l_weight = (m_plLabelImage + (y * 160 + x))->weight;	//该像素点的权重
				assert(l_weight <= 1.0f);
				if (h_num > 0)	//计算累加和
				{
					(m_pgTempCenter + (h_num-1)*31 + l_num)->sum_wx += l_weight * X.x;
					(m_pgTempCenter + (h_num-1)*31 + l_num)->sum_wy += l_weight * X.y;
					(m_pgTempCenter + (h_num-1)*31 + l_num)->sum_wz += l_weight * X.z;
					(m_pgTempCenter + (h_num-1)*31 + l_num)->sum_w += l_weight;
					eBodyPartPositionTrackingState[h_num - 1][l_num] = NUI_BODY_PART_POSITION_TRACKED;
					eBodyPartTrackingState[h_num - 1] = NUI_BODY_PART_POSISION_ONLY;
				}
			}
		}

		//t2 = GetTimeStampUS();

		//计算重心点
		for (int j = 0; j < 31 * 6; j ++)
		{
			if ((m_pgTempCenter + j)->sum_w != 0)
			{
				(m_pgTempCenter + j)->x = ((m_pgTempCenter + j)->sum_wx)/((m_pgTempCenter + j)->sum_w);
				(m_pgTempCenter + j)->y = ((m_pgTempCenter + j)->sum_wy)/((m_pgTempCenter + j)->sum_w);
				(m_pgTempCenter + j)->z = ((m_pgTempCenter + j)->sum_wz)/((m_pgTempCenter + j)->sum_w);
			}

			if ((m_pgTempCenter + j)->sum_w != 0)
			{
				int r, c;
				r = floor(j/31);
				c = j%31;
				vCentroid[r][c].x = ((m_pgTempCenter + j)->sum_wx)/((m_pgTempCenter + j)->sum_w);
				vCentroid[r][c].y = ((m_pgTempCenter + j)->sum_wy)/((m_pgTempCenter + j)->sum_w);
				vCentroid[r][c].z = ((m_pgTempCenter + j)->sum_wz)/((m_pgTempCenter + j)->sum_w);			
			}
		}
		MEASURE_TIME(g_initCentroid = tu.time());
	}

	//t3 = GetTimeStampUS();
	//printf("GrayGravityCentroid: Time = %8.3f ms, %8.3f ms, %8.3f ms\n", (t1-t0)*0.001, (t2-t1)*0.001, (t3-t2)*0.001);

//	memcpy_s(pCentroid, 6*31*sizeof(Vector4), &vCentroid, 6*31*sizeof(Vector4));

	delete [] m_plLabelImage;
	delete [] m_pgTempCenter;
	delete [] m_pusDepthLow3;
	delete [] m_pusDepthLow3Quarter;
}
