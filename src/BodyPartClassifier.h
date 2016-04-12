#ifndef _BODY_PART_CLASSIFIER_
#define _BODY_PART_CLASSIFIER_
#include <cassert>
#include "NuiSinect.h"
#include "RandomForest.h"
#include "Filter.h"
//using namespace NuiSinect;
//#define _Vector4I_
#ifndef _Vector4I_
#define _Vector4I_
typedef struct _Vector4I
{
    _Vector4I(int ax = 0, int ay = 0, int az = 0)
        : x(ax), y(ay), z(az), w(0)
    {
    }
    int x;
    int y;
    int z;
    int w;
}     Vector4I;
#endif// _Vector4I_

#define _DEPRECATED_
//////////////////////////////////////////////////////////////////////////
#ifndef _SMOOTH_FILTER_STATE_
#define _SMOOTH_FILTER_STATE_
typedef
    enum _SMOOTH_FILTER_STATE
{
    SMOOTH_FILTER_INITIAL  = 0,
    SMOOTH_FILTER_NTRACKED = SMOOTH_FILTER_INITIAL + 1,
    SMOOTH_FILTER_TRACKED  = SMOOTH_FILTER_NTRACKED + 1
}   SMOOTH_FILTER_STATE;
#endif// _SMOOTH_FILTER_STATE_

#ifndef _Joints_Smoothing_State_
#define _Joints_Smoothing_State_
typedef 
struct _Joints_Smoothing_State
{
    _Joints_Smoothing_State()
    {
        memset(&m_Bn, 0, 20 * sizeof(Vector4));
        memset(&m_Xn, 0, 20 * sizeof(Vector4));
        memset(&m_X, 0, 20 * sizeof(Vector4));
        memset(&m_FilterState, 0, 20 * sizeof(SMOOTH_FILTER_STATE));
    }
    Vector4 m_Bn[20];
    Vector4 m_Xn[20];
    Vector4 m_X[20];
    SMOOTH_FILTER_STATE m_FilterState[20];
}   Joints_Smoothing_State;
#endif// _Joints_Smoothing_State_
//////////////////////////////////////////////////////////////////////////



class  BodyPartClassifier
{
public:
    BodyPartClassifier();
	//BodyPartClassifier(cl_context a_context);

	void init();

	~BodyPartClassifier();

    // �������ɭ�ַ���ģ��
   // BOOL BuildForestFromFile(const  char * szFileName);
	bool BuildForestFromResource( );

    // ���ط���
    void ExemplarStage_RunFrame(const float player_scale, const unsigned short * pImage, unsigned short * pLabelRaw, int labelConvMode,unsigned char * pLabel, bool useGpu = false);

	void rawToLabel( unsigned char * pLabel, const unsigned short * pImage, int labelConvMode, unsigned short * pLabelRaw );


	void maxClass( unsigned short * class_dist, int &max_cnt, int &max_class );

    // ��ȡ���岿λ���ĵ�
    void CentroidsStage_RunFrame(const unsigned short * pImage, const unsigned short * pLabelRaw, NUI_CENTROIDS_STAGE_DATA * pData,Vector4 * m_pvCoordWorldSpace, float * m_pfBodyPartProbility);

	void vCentroid2BodyPartCentroids( NUI_CENTROIDS_STAGE_DATA * pData, 
		NUI_BODY_PART_TRACKING_STATE * eBodyPartTrackingState, Vector4  vCentroid[6][31], 
		int  iCount[6][31], 
		NUI_BODY_PART_POSITION_TRACKING_STATE  eBodyPartPositionTrackingState[6][31] );

    void meanshift( Vector4 vCentroid[6][31],  NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31],
		NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6], NUI_CENTROIDS_STAGE_DATA * pData, int iCount[6][31],
		Vector4 * m_pvCoordWorldSpace, float * m_pfBodyPartProbility, const unsigned short * pLabelRaw, const unsigned short * pImage);

	void initCentroidpoint( const unsigned short * pImage, const unsigned short * pLabelRaw, NUI_CENTROIDS_STAGE_DATA * pData ,int iCount[6][31],
		NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6], NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31], Vector4 vCentroid[6][31],
		Vector4 * m_pvCoordWorldSpace, float * m_pfBodyPartProbility);

    void centroidReduction( NUI_CENTROIDS_STAGE_DATA * pData, int iCount[6][31],
		NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31], Vector4 vCentroid[6][31] );

    // ��ȡ����
    void ModelFittingStage_RunFrame(const NUI_CENTROIDS_STAGE_DATA * pCentroidData, NUI_SKELETON_FRAME * pSkeletonFrame);
	void ModelFittingStage_RunFrame(const NUI_CENTROIDS_STAGE_DATA * pCentroidData, NUI_SKELETON_FRAME * pSkeletonFrame,  NUI_SKELETON_FRAME * pSkeletonFrame_sf);
    //void SetupSkeleton(const Vector4 * centroid, Vector4 * joints);

	//@xu-li:�Ǽܽڵ����ʵ�ԣ������������������ж�
	void isTracked(NUI_SKELETON_DATA * pSkeletonData);

    // ����ƽ��
    int NuiTransformSmooth(NUI_SKELETON_FRAME *pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS *apSmoothingParams = NULL);

    // ͼ�����������ϵת��
    static void ImageToWorldSpace(const Vector4I & src, Vector4 & dst, int height = 120, int width = 160);
    static void WorldToImageSpace(const Vector4 & src, Vector4I & dst, int height = 120, int width = 160);

    //// ����Labelͼ��ת��ΪRGBXģʽ, 3�ֱַ���:120/160, 240/320, 480/640
    //void GetLabelImage(const unsigned char * pLabel, unsigned char * pLabelImage);
    //void GetLabelImage2X(const unsigned char * pLabel, unsigned char * pLabelImage);
    //void GetLabelImage4X(const unsigned char * pLabel, unsigned char * pLabelImage);
	void classifyOnePixelClass( const unsigned short * pImage, int y, int x, const float player_scale, const int DepthUnusual, int * maxclass , unsigned short * pLabelRaw);
	void GrayGravityCentroid(const USHORT * pImage, const unsigned short * pLabelRaw, Vector4 vCentroid[6][31],
		NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[6][31],
		NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState[6]);
//	void clearResult();
//protected:
    // ���ɭ��predict
    void PredictRawParallel(const float player_scale, const unsigned short * pImage , unsigned short * pLabelRaw);
	void classifyOnePixel( const unsigned short * pImage, int y, int x, const float player_scale, const int DepthUnusual,  unsigned short * pLabelRaw);


	// ����class_dist����labelͼ
	int GetClassDistWithNeighbor2X2(const unsigned short * pImage, unsigned short * class_dist, int y, int x, int method, unsigned short * pLabelRaw);
	int GetClassDistWithNeighbor3X3(const unsigned short * pImage, unsigned short * class_dist, int y, int x, int method, unsigned short * pLabelRaw);
	void PredictRawGpu( const float player_scale, const unsigned short * pImage , unsigned short * pLabelRaw);
	inline float getLabelRawAt( int pid, int bpid, int x, int y, const unsigned short * pLabelRaw, const unsigned short * pImage );
	//private:
    RandomForest     m_forest;
    //unsigned short * m_pLabelRaw;
    //unsigned char  * m_pMask;
    // CentroidsStage_RunFrame��
    // ��ǰ֡�������ص���������ϵ����(x, y, z(depth))
   // Vector4 * m_pvCoordWorldSpace;   //[120][160]
    // 6����31����λ�ĸ���
   // float   * m_pfBodyPartProbility; // [6][120][160][31]
    int  m_iBodyPartProbilityStep;
    // smooth��
    NUI_TRANSFORM_SMOOTH_PARAMETERS m_defaultSmoothingParams;
    Joints_Smoothing_State          m_msSmoothingState[6];


	//cl_mem cl_m_pfBodyPartProbility;
	//cl_context m_context;

	//@xu-li:������
	Filter *filter;
	Vector4 pretmp1[ 20*6 ];
	Vector4 pretmp2[ 20*6 ];
	Vector4 btmp1[ 20*6 ];
	Vector4 btmp2[ 20*6 ];
	unsigned char isfirstTime[ 20*6 ];
};

#endif//
