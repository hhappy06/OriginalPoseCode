

#define NUI_IMAGE_PLAYER_INDEX_SHIFT 3
#define NUI_IMAGE_PLAYER_INDEX_MASK  7
#define OUT_COLS     160
#define OUT_ROWS     120
#define DEPTH_COLS   320
#define DEPTH_ROWS   240
//#define TREECOUNT    3
//#define TREENODECNT  1023
#define CLASSCOUNT   31
#define DepthUnusual 8000.0f
#define DEPTH_FACTOR 2048.0f

#define TREECOUNT  3
#define TREE_DEPTH  18
#define TREENODECNT  ((1<<(TREE_DEPTH+1)) - 1)
#define VALUE_COUNT  835114

#define AVERAGE_BAND_WIDTH (0.065f * 8)
#define NUI_CAMERA_DEPTH_NOMINAL_INVERSE_FOCAL_LENGTH_IN_PIXELS (3.501e-3f) // (1/NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS)
#define NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 (NUI_CAMERA_DEPTH_NOMINAL_INVERSE_FOCAL_LENGTH_IN_PIXELS)
#define NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS         (285.63f)   // Based on 320x240 pixel size.
#define NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 (NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS)
#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */


#define m_iBodyPartProbilityStep 120 * 160 * 31


typedef struct _Vector4I
{
    int x;
    int y;
    int z;
    int w;
} Vector4I;
typedef struct _tagVector4
{
        float x;
        float y;
        float z;
        float w;
}Vector4;
// 决策树节点
typedef struct tagTreeNode
{
	int  left;
	int  right;
	signed char ux, uy;
	signed char vx, vy;
	signed short c;
}TreeNode;


// 叶子节点Value: 部位类别的概率
typedef struct tagClassDist
{
	unsigned char id;
	unsigned char cnt;
}ClassDist;


// 叶子节点Value, 5个最大概率最大的类别
typedef struct NodeValue
{
	ClassDist v[5];
}NodeValue;

typedef 
enum _NUI_BODY_PART_TRACKING_STATE
{
    NUI_BODY_PART_NOT_TRACKED = 0,
    NUI_BODY_PART_POSISION_ONLY,
    NUI_BODY_PART_TRACKED
} NUI_BODY_PART_TRACKING_STATE;
typedef
enum _NUI_BODY_PART_POSITION_TRACKING_STATE
{
    NUI_BODY_PART_POSITION_NOT_TRACKED = 0,
    NUI_BODY_PART_POSITION_INFERRED    = ( NUI_BODY_PART_POSITION_NOT_TRACKED + 1 ),
    NUI_BODY_PART_POSITION_TRACKED     = ( NUI_BODY_PART_POSITION_INFERRED + 1 ),
} NUI_BODY_PART_POSITION_TRACKING_STATE;

typedef struct _NUI_BODY_PART_CENTROIDS
{
    NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState;
    Vector4 Centroids[ 31 ];
    int PixelCount[ 31 ];
    NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[ 31 ];
} NUI_BODY_PART_CENTROIDS;

typedef struct _NUI_CENTROIDS_STAGE_DATA
{
    NUI_BODY_PART_CENTROIDS BodyPartCentroids[ 6 ];
} NUI_CENTROIDS_STAGE_DATA;


__kernel 
	void rfpredict(
	__global TreeNode * atree,
	__global NodeValue * avalue,
	__global unsigned short * aimage, 
	__global unsigned short  * aLabelRaw,
	__global unsigned char  * alabel
	)
{

	__global unsigned short * class_dist = aLabelRaw +  get_global_id(0) * OUT_COLS * CLASSCOUNT + get_global_id(1)* CLASSCOUNT;

	//unsigned short dx_with_player_index = aimage[p.y * DEPTH_COLS + p.x];
	unsigned short dx_with_player_index = aimage[get_global_id(0) * 2 * DEPTH_COLS + get_global_id(1) * 2];
	unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index = dx_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
	int vi ;
	int i;
        alabel[ get_global_id(0) * OUT_COLS + get_global_id(1)] = 0;
        for (i = 0;i < CLASSCOUNT; i ++)
        {
            class_dist[i] = 0;
        }
	if (player_index)
	{
		int node_id;
		int px, py;


		unsigned short dp_with_player_index;
		unsigned short player_index_p;

		float dxinv = DEPTH_FACTOR / (float) dx;
		for (i = 0; i < 3; i ++)
		{
			node_id = 0;
			do
			{
				int dp1 = -1, dp2 = -1;
				px = (int)(((float)atree[node_id].ux * dxinv) + get_global_id(1) * 2);
				py = (int)(((float)atree[node_id].uy * dxinv) + get_global_id(0) * 2);
				dp1 = DepthUnusual;
				if (px >= 0 && px < DEPTH_COLS && py >= 0 && py < DEPTH_ROWS) 
				{
					dp_with_player_index = aimage[py * DEPTH_COLS + px];
					player_index_p = dp_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
					if (player_index_p == player_index)
					{
						dp1 = (dp_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT);// / DEPTH_FACTOR;
						// dp.x = tmp;
					}
				}

				if (atree[node_id].vx  == 0 && atree[node_id].vy == 0)
				{
					dp2 = dx;
				}
				else
				{
					dp2 = DepthUnusual;
					px = (int)(((float)atree[node_id].vx * dxinv) + get_global_id(1) * 2);
					py = (int)(((float)atree[node_id].vy * dxinv) + get_global_id(0) * 2);
					if (px >= 0 && px < DEPTH_COLS && py >= 0 && py < DEPTH_ROWS) 
					{
						dp_with_player_index = aimage[py * DEPTH_COLS + px];
						player_index_p = dp_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
						if (player_index_p == player_index)
						{
							dp2 = (dp_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT) ;
							// dp.y = tmp;
						}
					}
				}
				//node_id = ((dp1 - dp2) < n.c) ? n.left :  n.right;
			//	node_id = ((dp1 - dp2) < atree[node_id].c) ? atree[node_id].left :  atree[node_id].right;
				if ((dp1 - dp2) < (int)atree[node_id].c)
				{
					node_id = atree[node_id].left;
				}
				else
				{
					node_id = atree[node_id].right;
				}
			}
			while (node_id > 0);
			// class_dist[-(node_id + 1)] ++;

			int value_id = -(node_id + 1);
			__global NodeValue * v = avalue + value_id;
			for (vi = 0; vi < 5; vi++)
			{
				unsigned char id  = v->v[vi].id;		//该像素点所属部位的id
					
				unsigned char cnt = v->v[vi].cnt;		//对应于该像素点的投票数

				class_dist[id] += cnt;

			}
			atree += TREENODECNT;

		}
		int max_class = -1;
		int max_cnt = -1;
		for (int ci = 0; ci < 31; ci ++)
		{
			unsigned short cnt = class_dist[ci];
			if (cnt > 0)
			{
				if (max_cnt <= cnt)
				{
					max_cnt = cnt;
					max_class = ci;
				}
			}
		}
		alabel[ get_global_id(0) * OUT_COLS + get_global_id(1)] = max_class + 1;
	}
	//calc alabel

}
void WorldToImageSpace(Vector4 *src, Vector4I * dst, int height/* = 120*/, int width/* = 160*/)
{
	//  static const float NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 = 285.63f;


	if(src->z > FLT_EPSILON)
	{
		dst->x = floor(width/2 + src->x * (width/320.0) * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / src->z + 0.5f);
		dst->y = floor(height/2 - src->y * (height/240.0) * NUI_CAMERA_SKELETON_TO_DEPTH_IMAGE_MULTIPLIER_320x240 / src->z + 0.5f);
		dst->z = floor(src->z * 1000 +0.5f);
		dst->w = 1;
	}
	else
	{
		dst->x = 0;
		dst->y = 0;
		dst->z = 0;
		dst->w = 0;
	}
	//assert(dst->y >= 0 && dst->y < height);
	//assert(dst->x >= 0 && dst->x < width);
}

void ImageToWorldSpace(const Vector4I * src, Vector4 * dst, int height/* = 120*/, int width/* = 160*/)
{
	//  static const float NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 = 3.501f/1000;

	dst->z = src->z * 0.001f;
	dst->x = (src->x - width/2.0f) * (320.0f/width) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * dst->z;
	dst->y = - (src->y - height/2.0f) * (240.0f/height) * NUI_CAMERA_DEPTH_IMAGE_TO_SKELETON_MULTIPLIER_320x240 * dst->z;
	dst->w = 0;
}

__kernel 
	void initCentroid(
	__global unsigned short * pImage, 
	__global unsigned short  * pLabelRaw,
	__global int   iCount[6][31],
	__global  NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6],
	__global NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31],
	__global Vector4 vCentroid[6][31],
	__global  Vector4 * m_pvCoordWorldSpace, 
	__global  float * m_pfBodyPartProbility
	)
{
	int x = get_global_id(1);
	int y = get_global_id(0);
			int kss;
		int sss;
	__global unsigned short * pdepth = pImage + (y * 2) * 320;
	unsigned short dx_with_player_index = pdepth[x * 2];
	unsigned short dx = dx_with_player_index >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
	unsigned short player_index = dx_with_player_index & NUI_IMAGE_PLAYER_INDEX_MASK;
	unsigned short pid = player_index - 1; // for access vCentroid & iCount & eTrackingState
	if (player_index == 0)
	{
		return;
	}

	// get p(c|u)
	__global unsigned short * pClassDist = pLabelRaw + (y * 160 + x) * 31;

	// make label image
	// nothing

	// X(u) = [x(u), y(u), z(u)]
	// 当前像素在世界坐标系的坐标(XYZ)
	Vector4 X;
	Vector4I xTemp;
	xTemp.x = x;
	xTemp.y = y;
	xTemp.z = dx;
	xTemp.w = 0;
	ImageToWorldSpace(&xTemp, &X, 120, 160);

	// push back 0.039m, save Z
	//X.z -= 0.039f;
	//Z[y * 160 + x] = X.z;
	m_pvCoordWorldSpace[y * 160 + x] = X;

	// compute m_pfBodyPartProbility, W = m_pfBodyPartProbility / Z^2;
	int offset = y * 160 + x;
	for (int bpid = 0; bpid < 31; bpid ++)
	{
		float p = pClassDist[bpid] / (3.0f*255.0f);
		m_pfBodyPartProbility[pid * m_iBodyPartProbilityStep + bpid * 120 * 160 + offset] = p;
		// initialize centroid
		__global Vector4 * c = &vCentroid[pid][bpid];
		if (p > 0.14)
		{

			c->x += (X.x * 1.0f);
			c->y += (X.y * 1.0f);
			c->z += (X.z * 1.0f);
			iCount[pid][bpid] += 1;

			kss = iCount[pid][bpid];
			eBodyPartPositionTrackingState[pid][bpid] = NUI_BODY_PART_POSITION_TRACKED;
			sss = eBodyPartPositionTrackingState[pid][bpid] ;
		}
		else
		{
			m_pfBodyPartProbility[pid * m_iBodyPartProbilityStep + bpid * 120 * 160 + offset] = 0.0f;
		}
	}
	
}

__kernel 
 void centroidReduction(
 __global   NUI_CENTROIDS_STAGE_DATA * pData, 
 __global  int iCount[6][31], 
 __global  NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31], 
 __global  Vector4 vCentroid[6][31] )
{
	//for (int pid = 0; pid < 6; pid ++)
	//{
	//	for (int bpid = 0; bpid < 31; bpid ++)
	//	{
			/**/
	int pid = get_global_id(0) ;
	int bpid = get_global_id(1) ;
	if (pData->BodyPartCentroids[pid].eBodyPartPositionTrackingState[bpid] == NUI_BODY_PART_POSITION_TRACKED)
	{
		vCentroid[pid][bpid] = pData->BodyPartCentroids[pid].Centroids[bpid];
	}
	else  if (eBodyPartPositionTrackingState[pid][bpid] == NUI_BODY_PART_POSITION_TRACKED)
	{
		int kks = iCount[pid][bpid];
		vCentroid[pid][bpid].x /= iCount[pid][bpid];
		vCentroid[pid][bpid].y /= iCount[pid][bpid];
		vCentroid[pid][bpid].z /= iCount[pid][bpid];

	}
	else
	{
		//TODO: 没有满足条件的初始中心点
		vCentroid[pid][bpid].x = 0;
		vCentroid[pid][bpid].y = 0;
		vCentroid[pid][bpid].z = 0;
	}
		//}
	//}
}


__constant float BC[16] = {
	8 * AVERAGE_BAND_WIDTH,
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
	1 * AVERAGE_BAND_WIDTH
};

__kernel 
void meanshift(
			__global NUI_CENTROIDS_STAGE_DATA * pData, 
			__global int iCount[6][31],
			__global NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31],
			__global Vector4 vCentroid[6][31], 
			__global NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6],
			__global Vector4 * m_pvCoordWorldSpace, 
			__global float * m_pfBodyPartProbility)
{

	int pid = get_global_id(0) ;
	int bpid = get_global_id(1) ;

	__global float * pP = m_pfBodyPartProbility + pid * m_iBodyPartProbilityStep + bpid * 120 * 160;
	Vector4 c_old;
	Vector4 c = vCentroid[pid][bpid];
	NUI_BODY_PART_POSITION_TRACKING_STATE state = eBodyPartPositionTrackingState[pid][bpid];

	if (state == NUI_BODY_PART_POSITION_TRACKED)
	{
		const int initHalfLong = 10;
		int halfsideLong = initHalfLong;
		bool lastSumkZero = false;
		int loop = 0;
		do
		{
			float sum_k = 0;
			Vector4 sum_kX;

			// my improve imperiment
			Vector4I imageC;
			WorldToImageSpace(&c, &imageC, 120, 160);

			int ybegin = imageC.y - halfsideLong;
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

			for (int y = ybegin; y < yend; y ++)
			{
				for (int x = xbegin; x < xend; x ++)
				{
					float p = pP[y * 160 + x];
					if (p < FLT_MIN && p > -FLT_MIN)
					{
						continue;
					}
					// X: 当前像素在世界坐标系的坐标
					Vector4 X = m_pvCoordWorldSpace[y * 160 + x];
					float distance_2_centroid = (X.x - c.x)*(X.x - c.x) + (X.y - c.y)*(X.y - c.y) + (X.z - c.z)*(X.z - c.z);
					float k = p * X.z * X.z * exp(-1*distance_2_centroid/(AVERAGE_BAND_WIDTH * AVERAGE_BAND_WIDTH));
					float fsfs = FLT_MIN;
					sum_k += k;
					sum_kX.x += (k * X.x);
					sum_kX.y += (k * X.y);
					sum_kX.z += (k * X.z);
				}
			}
			//assert(sum_k > 0.00001);
			if (sum_k < 0.00001)
			{
				if (lastSumkZero)
				{
					break;
				}
				lastSumkZero = true;
				halfsideLong = 160;
				//loop--;
				continue;
			}
			if (halfsideLong == 160)
			{
				halfsideLong = initHalfLong;
			}
			c_old = c;
			c.x = (sum_kX.x / sum_k);
			c.y = (sum_kX.y / sum_k);
			c.z = (sum_kX.z / sum_k);

			float distance = (c.x-c_old.x)*(c.x-c_old.x) + (c.y-c_old.y)*(c.y-c_old.y) + (c.z-c_old.z)*(c.z-c_old.z);
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
		vCentroid[pid][bpid] = c;
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


__kernel
void meanshiftReduction(
 __global   NUI_CENTROIDS_STAGE_DATA * pData, 
  __global  int iCount[6][31], 
 __global  NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31], 
  __global  Vector4 vCentroid[6][31],
   __global NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6]
)
{
	int pid = get_global_id(0) ;
	int bpid = get_global_id(1) ;
	pData->BodyPartCentroids[pid].Centroids[bpid]      = vCentroid[pid][bpid];
	pData->BodyPartCentroids[pid].PixelCount[bpid]     = iCount[pid][bpid];
	pData->BodyPartCentroids[pid].eBodyPartPositionTrackingState[bpid] = eBodyPartPositionTrackingState[pid][bpid];
	//if (bpid == 0)
	{
		pData->BodyPartCentroids[pid].eBodyPartTrackingState = eBodyPartTrackingState[pid];
	}
}
