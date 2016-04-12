#pragma once

#define _IN_
#define _OUT_
#define _INOUT_

#define _In_
#define _Out_
#define _Inout_

namespace NuiSinect
{

#ifndef _Vector4_
#define _Vector4_
typedef struct _Vector4
    {
        _Vector4(FLOAT ax = 0, FLOAT ay = 0, FLOAT az = 0)
            : x(ax), y(ay), z(az), w(0.0f)
        {
        }
        _Vector4 operator + (const _Vector4 & src) const
        {
            _Vector4 dst;
            dst.x = this->x + src.x;
            dst.y = this->y + src.y;
            dst.z = this->z + src.z;
            dst.w = this->w + src.w;
            return dst;
        }
        /*_Vector4 & operator + (const _Vector4 & src)
        {
            this->x += src.x;
            this->y += src.y;
            this->z += src.z;
            this->w += src.w;
            return *this;
        }*/
        _Vector4 operator - (const _Vector4 & src) const
        {
            _Vector4 dst;
            dst.x = this->x - src.x;
            dst.y = this->y - src.y;
            dst.z = this->z - src.z;
            dst.w = this->w - src.w;
            return dst;
        }
        /*_Vector4 & operator - (const _Vector4 & src)
        {
            this->x -= src.x;
            this->y -= src.y;
            this->z -= src.z;
            this->w -= src.w;
            return *this;
        }*/
        _Vector4 operator * (const float k) const
        {
            _Vector4 dst;
            dst.x = this->x * k;
            dst.y = this->y * k;
            dst.z = this->z * k;
            dst.w = this->w * k;
            return dst;
        }
        /*_Vector4 & operator * (const float k)
        {
            this->x *= k;
            this->y *= k;
            this->z *= k;
            this->w *= k;
            return *this;
        }*/
        FLOAT x;
        FLOAT y;
        FLOAT z;
        FLOAT w;
    }     Vector4;

//?????
//_bstr_t operator+(const char* s1, const _bstr_t& s2) ;



#endif// _Vector4_

#ifndef _NUI_BODY_PART_INDEX_
#define _NUI_BODY_PART_INDEX_
typedef 
enum _NUI_BODY_PART_INDEX
    {
        BODY_PART_LU_HEAD = 0,  //0
        BODY_PART_RU_HEAD,      //1
        BODY_PART_NECK,         //2
        BODY_PART_LU_ARM,       //3
        BODY_PART_LU_TORSO,     //4
        BODY_PART_RU_TORSO,     //5
        BODY_PART_RU_ARM,       //6
        BODY_PART_LW_ARM,       //7
        BODY_PART_LW_TORSO,     //8
        BODY_PART_RW_TORSO,     //9
        BODY_PART_RW_ARM,       //10
        BODY_PART_L_HAND,       //11
        BODY_PART_LU_LEG,       //12
        BODY_PART_RU_LEG,       //13
        BODY_PART_R_HAND,       //14
        BODY_PART_LW_LEG,       //15
        BODY_PART_RW_LEG,       //16
        BODY_PART_L_FOOT,       //17
        BODY_PART_R_FOOT,       //18
        BODY_PART_LW_HEAD,      //19
        BODY_PART_RW_HEAD,      //20
        BODY_PART_L_SHOULDER,   //21
        BODY_PART_R_SHOULDER,   //22
        BODY_PART_L_ELBOW,      //23
        BODY_PART_R_ELBOW,      //24
        BODY_PART_L_KNEE,       //25
        BODY_PART_R_KNEE,       //26
        BODY_PART_L_WRIST,      //27
        BODY_PART_R_WRIST,      //28
        BODY_PART_L_ANKLE,      //29
        BODY_PART_R_ANKLE,      //30
        BODY_PART_COUNT         //31
    } NUI_BODY_PART_INDEX;
#endif// _NUI_BODY_PART_INDEX_


#ifndef _NUI_SKELETON_POSITION_INDEX_
#define _NUI_SKELETON_POSITION_INDEX_
typedef 
enum _NUI_SKELETON_POSITION_INDEX
    {    NUI_SKELETON_POSITION_HIP_CENTER    = 0,
    NUI_SKELETON_POSITION_SPINE    = ( NUI_SKELETON_POSITION_HIP_CENTER + 1 ) ,
    NUI_SKELETON_POSITION_SHOULDER_CENTER    = ( NUI_SKELETON_POSITION_SPINE + 1 ) ,
    NUI_SKELETON_POSITION_HEAD    = ( NUI_SKELETON_POSITION_SHOULDER_CENTER + 1 ) ,
    NUI_SKELETON_POSITION_SHOULDER_LEFT    = ( NUI_SKELETON_POSITION_HEAD + 1 ) ,
    NUI_SKELETON_POSITION_ELBOW_LEFT    = ( NUI_SKELETON_POSITION_SHOULDER_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_WRIST_LEFT    = ( NUI_SKELETON_POSITION_ELBOW_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_HAND_LEFT    = ( NUI_SKELETON_POSITION_WRIST_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_SHOULDER_RIGHT    = ( NUI_SKELETON_POSITION_HAND_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_ELBOW_RIGHT    = ( NUI_SKELETON_POSITION_SHOULDER_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_WRIST_RIGHT    = ( NUI_SKELETON_POSITION_ELBOW_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_HAND_RIGHT    = ( NUI_SKELETON_POSITION_WRIST_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_HIP_LEFT    = ( NUI_SKELETON_POSITION_HAND_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_KNEE_LEFT    = ( NUI_SKELETON_POSITION_HIP_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_ANKLE_LEFT    = ( NUI_SKELETON_POSITION_KNEE_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_FOOT_LEFT    = ( NUI_SKELETON_POSITION_ANKLE_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_HIP_RIGHT    = ( NUI_SKELETON_POSITION_FOOT_LEFT + 1 ) ,
    NUI_SKELETON_POSITION_KNEE_RIGHT    = ( NUI_SKELETON_POSITION_HIP_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_ANKLE_RIGHT    = ( NUI_SKELETON_POSITION_KNEE_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_FOOT_RIGHT    = ( NUI_SKELETON_POSITION_ANKLE_RIGHT + 1 ) ,
    NUI_SKELETON_POSITION_COUNT    = ( NUI_SKELETON_POSITION_FOOT_RIGHT + 1 ) 
    }     NUI_SKELETON_POSITION_INDEX;

#endif// _NUI_SKELETON_POSITION_INDEX_

#ifndef _NUI_IMAGE_TYPE_
#define _NUI_IMAGE_TYPE_
typedef 
enum _NUI_IMAGE_TYPE
    {    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX    = 0,
    NUI_IMAGE_TYPE_COLOR    = ( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX + 1 ) ,
    NUI_IMAGE_TYPE_COLOR_YUV    = ( NUI_IMAGE_TYPE_COLOR + 1 ) ,
    NUI_IMAGE_TYPE_COLOR_RAW_YUV    = ( NUI_IMAGE_TYPE_COLOR_YUV + 1 ) ,
    NUI_IMAGE_TYPE_DEPTH    = ( NUI_IMAGE_TYPE_COLOR_RAW_YUV + 1 ) ,
    NUI_IMAGE_TYPE_COLOR_INFRARED    = ( NUI_IMAGE_TYPE_DEPTH + 1 ) ,
    NUI_IMAGE_TYPE_COLOR_RAW_BAYER    = ( NUI_IMAGE_TYPE_COLOR_INFRARED + 1 ) 
    }     NUI_IMAGE_TYPE;

#endif// _NUI_IMAGE_TYPE_
#ifndef _NUI_IMAGE_RESOLUTION_
#define _NUI_IMAGE_RESOLUTION_
typedef 
enum _NUI_IMAGE_RESOLUTION
    {    NUI_IMAGE_RESOLUTION_INVALID    = -1,
    NUI_IMAGE_RESOLUTION_80x60    = 0,
    NUI_IMAGE_RESOLUTION_320x240    = ( NUI_IMAGE_RESOLUTION_80x60 + 1 ) ,
    NUI_IMAGE_RESOLUTION_640x480    = ( NUI_IMAGE_RESOLUTION_320x240 + 1 ) ,
    NUI_IMAGE_RESOLUTION_1280x960    = ( NUI_IMAGE_RESOLUTION_640x480 + 1 ) 
    }     NUI_IMAGE_RESOLUTION;

#endif// _NUI_IMAGE_RESOLUTION_


#ifndef _NUI_IMAGE_VIEW_AREA_
#define _NUI_IMAGE_VIEW_AREA_
typedef struct _NUI_IMAGE_VIEW_AREA
    {
    int eDigitalZoom;
    LONG lCenterX;
    LONG lCenterY;
    }     NUI_IMAGE_VIEW_AREA;

#endif// _NUI_IMAGE_VIEW_AREA_


#ifndef _NUI_TRANSFORM_SMOOTH_PARAMETERS_
#define _NUI_TRANSFORM_SMOOTH_PARAMETERS_
typedef struct _NUI_TRANSFORM_SMOOTH_PARAMETERS
    {
        _NUI_TRANSFORM_SMOOTH_PARAMETERS()
        {
            fSmoothing          = 0.5f;
            fCorrection         = 0.5f;
            fPrediction         = 0.5f;
            fJitterRadius       = 0.05f;
            fMaxDeviationRadius = 0.04f;
        }
        FLOAT fSmoothing;
        FLOAT fCorrection;
        FLOAT fPrediction;
        FLOAT fJitterRadius;
        FLOAT fMaxDeviationRadius;
    }     NUI_TRANSFORM_SMOOTH_PARAMETERS;
#endif// _NUI_TRANSFORM_SMOOTH_PARAMETERS_


#ifndef _NUI_SURFACE_DESC_
#define _NUI_SURFACE_DESC_
typedef struct _NUI_SURFACE_DESC
    {
    UINT Width;
    UINT Height;
    }     NUI_SURFACE_DESC;

#endif// _NUI_SURFACE_DESC_


#ifndef _NUI_SKELETON_POSITION_TRACKING_STATE_
#define _NUI_SKELETON_POSITION_TRACKING_STATE_
typedef 
enum _NUI_SKELETON_POSITION_TRACKING_STATE
    {    NUI_SKELETON_POSITION_NOT_TRACKED    = 0,
    NUI_SKELETON_POSITION_INFERRED    = ( NUI_SKELETON_POSITION_NOT_TRACKED + 1 ) ,
    NUI_SKELETON_POSITION_TRACKED    = ( NUI_SKELETON_POSITION_INFERRED + 1 ) 
    }     NUI_SKELETON_POSITION_TRACKING_STATE;
#endif// _NUI_SKELETON_POSITION_TRACKING_STATE_

#ifndef _NUI_BODY_PART_POSITION_TRACKING_STATE_
#define _NUI_BODY_PART_POSITION_TRACKING_STATE_
typedef
enum _NUI_BODY_PART_POSITION_TRACKING_STATE
    {
        NUI_BODY_PART_POSITION_NOT_TRACKED = 0,
        NUI_BODY_PART_POSITION_INFERRED    = ( NUI_BODY_PART_POSITION_NOT_TRACKED + 1 ),
        NUI_BODY_PART_POSITION_TRACKED     = ( NUI_BODY_PART_POSITION_INFERRED + 1 ),
    } NUI_BODY_PART_POSITION_TRACKING_STATE;
#endif// _NUI_BODY_PART_POSITION_TRACKING_STATE_

#ifndef _NUI_SKELETON_TRACKING_STATE_
#define _NUI_SKELETON_TRACKING_STATE_
typedef 
enum _NUI_SKELETON_TRACKING_STATE
    {    NUI_SKELETON_NOT_TRACKED    = 0,
    NUI_SKELETON_POSITION_ONLY    = ( NUI_SKELETON_NOT_TRACKED + 1 ) ,
    NUI_SKELETON_TRACKED    = ( NUI_SKELETON_POSITION_ONLY + 1 ) 
    }     NUI_SKELETON_TRACKING_STATE;

#endif// _NUI_SKELETON_TRACKING_STATE_

#ifndef _NUI_BODY_PART_TRACKING_STATE_
#define _NUI_BODY_PART_TRACKING_STATE_
typedef 
enum _NUI_BODY_PART_TRACKING_STATE
{
    NUI_BODY_PART_NOT_TRACKED = 0,
    NUI_BODY_PART_POSISION_ONLY,
    NUI_BODY_PART_TRACKED
} NUI_BODY_PART_TRACKING_STATE;
#endif// _NUI_BODY_PART_TRACKING_STATE_

#ifndef _NUI_SKELETON_DATA_
#define _NUI_SKELETON_DATA_
typedef struct _NUI_SKELETON_DATA
    {
    NUI_SKELETON_TRACKING_STATE eTrackingState;
    DWORD dwTrackingID;
    DWORD dwEnrollmentIndex;
    DWORD dwUserIndex;
    Vector4 Position;
    Vector4 SkeletonPositions[ 20 ];
    NUI_SKELETON_POSITION_TRACKING_STATE eSkeletonPositionTrackingState[ 20 ];
    DWORD dwQualityFlags;
    }     NUI_SKELETON_DATA;

#endif// _NUI_SKELETON_TRACKING_STATE_
#ifndef _NUI_SKELETON_FRAME_
#define _NUI_SKELETON_FRAME_

#pragma pack(push, 16)
typedef struct _NUI_SKELETON_FRAME
    {
    LARGE_INTEGER liTimeStamp;
    DWORD dwFrameNumber;
    DWORD dwFlags;
    Vector4 vFloorClipPlane;
    Vector4 vNormalToGravity;
    NUI_SKELETON_DATA SkeletonData[ 6 ];
    }     NUI_SKELETON_FRAME;


#pragma pack(pop)
#endif// _NUI_SKELETON_FRAME_

#ifndef _NUI_BODY_PART_CENTROIDS_
#define _NUI_BODY_PART_CENTROIDS_
typedef struct _NUI_BODY_PART_CENTROIDS
{
    NUI_BODY_PART_TRACKING_STATE eBodyPartTrackingState;
    Vector4 Centroids[ 31 ];
    int PixelCount[ 31 ];
    NUI_BODY_PART_POSITION_TRACKING_STATE eBodyPartPositionTrackingState[ 31 ];
} NUI_BODY_PART_CENTROIDS;
#endif// _NUI_BODY_PART_CENTROIDS_

#ifndef _NUI_CENTROIDS_STAGE_DATA_
#define _NUI_CENTROIDS_STAGE_DATA_
typedef struct _NUI_CENTROIDS_STAGE_DATA
{
    NUI_BODY_PART_CENTROIDS BodyPartCentroids[ 6 ];
} NUI_CENTROIDS_STAGE_DATA;
#endif// _NUI_CENTROIDS_STAGE_DATA_


#ifndef _NUI_LOCKED_RECT_
#define _NUI_LOCKED_RECT_
#ifdef _MIDL_OUTPUT_
typedef struct _NUI_LOCKED_RECT
{
    INT Pitch;
    INT size;
    void *pBits;
}     NUI_LOCKED_RECT;

#else
typedef struct _NUI_LOCKED_RECT
{
    INT                         Pitch;
    INT                         size;
    byte                        *pBits;
} NUI_LOCKED_RECT;
#endif// _MIDL_OUTPUT_
#endif// _NUI_LOCKED_RECT_

#ifndef _NUI_IMAGE_FRAME_
#define _NUI_IMAGE_FRAME_
class INuiFrameTexture;
typedef struct _NUI_IMAGE_FRAME
{
    LARGE_INTEGER liTimeStamp;
    DWORD dwFrameNumber;
    NUI_IMAGE_TYPE eImageType;
    NUI_IMAGE_RESOLUTION eResolution;
    INuiFrameTexture *pFrameTexture;
    DWORD dwFrameFlags;
    NUI_IMAGE_VIEW_AREA ViewArea;
}     NUI_IMAGE_FRAME;

#endif// _NUI_IMAGE_FRAME_

#define RECT void
class INuiFrameTexture
{
public:
    virtual int /*STDMETHODCALLTYPE*/ BufferLen( void) = 0;

    virtual int /*STDMETHODCALLTYPE*/ Pitch( void) = 0;

    virtual HRESULT /*STDMETHODCALLTYPE*/ LockRect( 
        UINT Level,
        /* [ref] */ NUI_LOCKED_RECT *pLockedRect,
        /* [unique] */ RECT *pRect,
        DWORD Flags) = 0;

    virtual HRESULT /*STDMETHODCALLTYPE*/ UnlockRect( 
        /* [in] */ UINT Level) = 0;        
};


/*
NUI_SKELETON_POSITION_HIP_CENTER   = 0,  //0, 
NUI_SKELETON_POSITION_SPINE           ,  //1, 
NUI_SKELETON_POSITION_SHOULDER_CENTER ,  //2, BODY_PART_NECK
NUI_SKELETON_POSITION_HEAD            ,  //3, (BODY_PART_LU_HEAD + BODY_PART_RU_HEAD) * 0.5f
NUI_SKELETON_POSITION_SHOULDER_LEFT   ,  //4, BODY_PART_L_SHOULDER
NUI_SKELETON_POSITION_ELBOW_LEFT      ,  //5, BODY_PART_L_ELBOW
NUI_SKELETON_POSITION_WRIST_LEFT      ,  //6, BODY_PART_L_WRIST/BODY_PART_L_HAND/0(需修正)
NUI_SKELETON_POSITION_HAND_LEFT       ,  //7, BODY_PART_L_HAND/BODY_PART_L_WRIST/0(需修正)
NUI_SKELETON_POSITION_SHOULDER_RIGHT  ,  //8, BODY_PART_R_SHOULDER
NUI_SKELETON_POSITION_ELBOW_RIGHT     ,  //9, BODY_PART_R_ELBOW
NUI_SKELETON_POSITION_WRIST_RIGHT     ,  //10, BODY_PART_R_WRIST/BODY_PART_R_HAND/0(需修正)
NUI_SKELETON_POSITION_HAND_RIGHT      ,  //11, BODY_PART_R_HAND/BODY_PART_R_WRIST/0(需修正)
NUI_SKELETON_POSITION_HIP_LEFT        ,  //12, BODY_PART_LW_TORSO
NUI_SKELETON_POSITION_KNEE_LEFT       ,  //13, BODY_PART_L_KNEE
NUI_SKELETON_POSITION_ANKLE_LEFT      ,  //14, BODY_PART_L_ANKLE
NUI_SKELETON_POSITION_FOOT_LEFT       ,  //15, BODY_PART_L_FOOT
NUI_SKELETON_POSITION_HIP_RIGHT       ,  //16, BODY_PART_RW_TORSO
NUI_SKELETON_POSITION_KNEE_RIGHT      ,  //17, BODY_PART_R_KNEE
NUI_SKELETON_POSITION_ANKLE_RIGHT     ,  //18, BODY_PART_R_ANKLE
NUI_SKELETON_POSITION_FOOT_RIGHT      ,  //19, BODY_PART_R_FOOT
*/

class INuiSensor
{
public:
    virtual HRESULT STDMETHODCALLTYPE NuiInitialize( 
        /* [in] */ DWORD dwFlags) = 0;

    virtual void STDMETHODCALLTYPE NuiShutdown( void) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamOpen( 
        /* [in] */ NUI_IMAGE_TYPE eImageType,
        /* [in] */ NUI_IMAGE_RESOLUTION eResolution,
        /* [in] */ DWORD dwImageFrameFlags,
        /* [in] */ DWORD dwFrameLimit,
        /* [in] */ HANDLE hNextFrameEvent,
        /* [out] */ HANDLE *phStreamHandle) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamSetImageFrameFlags( 
        /* [in] */ HANDLE hStream,
        /* [in] */ DWORD dwImageFrameFlags) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamGetNextFrame( 
        /* [in] */ HANDLE hStream,
        /* [in] */ DWORD dwMillisecondsToWait,
        /* [retval][out] */ NUI_IMAGE_FRAME *pImageFrame) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamReleaseFrame( 
        /* [in] */ HANDLE hStream,
        /* [in] */ NUI_IMAGE_FRAME *pImageFrame) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingEnable( 
        /* [in] */ HANDLE hNextFrameEvent,
        /* [in] */ DWORD dwFlags) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingDisable( void) = 0;


    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonSetTrackedSkeletons( 
        /* [size_is][in] */ DWORD *TrackingIDs) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonGetNextFrame( 
        /* [in] */ DWORD dwMillisecondsToWait,
        /* [out][in] */ NUI_SKELETON_FRAME *pSkeletonFrame) = 0;

    // 骨骼平滑
    virtual HRESULT STDMETHODCALLTYPE NuiTransformSmooth( 
        NUI_SKELETON_FRAME *pSkeletonFrame,
        const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams = NULL) = 0;

#if 0

    virtual HRESULT STDMETHODCALLTYPE NuiSetFrameEndEvent( 
        /* [in] */ HANDLE hEvent,
        /* [in] */ DWORD dwFrameEventFlag) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamGetImageFrameFlags( 
        /* [in] */ HANDLE hStream,
        /* [retval][out] */ DWORD *pdwImageFrameFlags) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageGetColorPixelCoordinatesFromDepthPixel( 
        /* [in] */ NUI_IMAGE_RESOLUTION eColorResolution,
        /* [in] */ const NUI_IMAGE_VIEW_AREA *pcViewArea,
        /* [in] */ LONG lDepthX,
        /* [in] */ LONG lDepthY,
        /* [in] */ USHORT usDepthValue,
        /* [out] */ LONG *plColorX,
        /* [out] */ LONG *plColorY) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution( 
        /* [in] */ NUI_IMAGE_RESOLUTION eColorResolution,
        /* [in] */ NUI_IMAGE_RESOLUTION eDepthResolution,
        /* [in] */ const NUI_IMAGE_VIEW_AREA *pcViewArea,
        /* [in] */ LONG lDepthX,
        /* [in] */ LONG lDepthY,
        /* [in] */ USHORT usDepthValue,
        /* [out] */ LONG *plColorX,
        /* [out] */ LONG *plColorY) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution( 
        /* [in] */ NUI_IMAGE_RESOLUTION eColorResolution,
        /* [in] */ NUI_IMAGE_RESOLUTION eDepthResolution,
        /* [in] */ DWORD cDepthValues,
        /* [size_is][in] */ USHORT *pDepthValues,
        /* [in] */ DWORD cColorCoordinates,
        /* [size_is][out][in] */ LONG *pColorCoordinates) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiCameraElevationSetAngle( 
        /* [in] */ LONG lAngleDegrees) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiCameraElevationGetAngle( 
        /* [retval][out] */ LONG *plAngleDegrees) = 0;

#ifdef NUI_AUDIO
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NuiGetAudioSource( 
        /* [out] */ INuiAudioBeam **ppDmo) = 0;
#endif//

    virtual int STDMETHODCALLTYPE NuiInstanceIndex( void) = 0;

    virtual BSTR STDMETHODCALLTYPE NuiDeviceConnectionId( void) = 0;

    virtual BSTR STDMETHODCALLTYPE NuiUniqueId( void) = 0;

    virtual BSTR STDMETHODCALLTYPE NuiAudioArrayId( void) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiStatus( void) = 0;

    virtual DWORD STDMETHODCALLTYPE NuiInitializationFlags( void) = 0;

#ifdef NUI_COLOR_CAMERA
    virtual HRESULT STDMETHODCALLTYPE NuiGetCoordinateMapper( 
        /* [retval][out] */ INuiCoordinateMapper **pMapping) = 0;
#endif//

    virtual HRESULT STDMETHODCALLTYPE NuiImageFrameGetDepthImagePixelFrameTexture( 
        /* [in] */ HANDLE hStream,
        /* [in] */ NUI_IMAGE_FRAME *pImageFrame,
        /* [out] */ BOOL *pNearMode,
        /* [out] */ INuiFrameTexture **ppFrameTexture) = 0;

#ifdef NUI_COLOR_CAMERA
    virtual HRESULT STDMETHODCALLTYPE NuiGetColorCameraSettings( 
        /* [retval][out] */ INuiColorCameraSettings **pCameraSettings) = 0;
#endif//

    virtual BOOL STDMETHODCALLTYPE NuiGetForceInfraredEmitterOff( void) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiSetForceInfraredEmitterOff( 
        /* [in] */ BOOL fForceInfraredEmitterOff) = 0;

    virtual HRESULT STDMETHODCALLTYPE NuiAccelerometerGetCurrentReading( 
        /* [retval][out] */ Vector4 *pReading) = 0;

#endif// 0
};

#ifndef _INTERFACE_DEFINED_
#define _INTERFACE_DEFINED_
#define NUIAPI

/*_Check_return_*/ HRESULT NUIAPI NuiCreateSensorByIndex( _In_ int index, _Out_ INuiSensor ** ppNuiSensor );

#if 0
HRESULT NUIAPI NuiInitialize( _In_ DWORD dwFlags );

VOID NUIAPI NuiShutdown();

/*_Check_return_*/ HRESULT NUIAPI NuiGetSensorCount( _In_ int * pCount );

/*_Check_return_*/ HRESULT NUIAPI NuiCreateSensorByIndex( _In_ int index, _Out_ INuiSensor ** ppNuiSensor );

/*_Check_return_*/ HRESULT NUIAPI NuiCreateSensorById( _In_ const OLECHAR *strInstanceId, _Out_ INuiSensor ** ppNuiSensor );

#ifdef NUI_AUDIO
/*_Check_return_*/ HRESULT NUIAPI NuiGetAudioSource( _Out_ INuiAudioBeam ** ppDmo );
#endif//

typedef void (CALLBACK* NuiStatusProc)( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* pUserData );

void NUIAPI NuiSetDeviceStatusCallback( NuiStatusProc callback, void* pUserData );

/*_Check_return_*/ inline bool HasSkeletalEngine(_In_ INuiSensor *pNuiSensor)
{
    if (!pNuiSensor) return false;
    //return (pNuiSensor->NuiInitializationFlags() & NUI_INITIALIZE_FLAG_USES_SKELETON) || (pNuiSensor->NuiInitializationFlags() & NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX);
    return true;
};

#ifdef NUI_AUDIO
#define MAX_DEV_STR_LEN 512
typedef struct
{
    wchar_t szDeviceName[MAX_DEV_STR_LEN];
    wchar_t szDeviceID[MAX_DEV_STR_LEN];
    int iDeviceIndex;
} NUI_MICROPHONE_ARRAY_DEVICE, *PNUI_MICROPHONE_ARRAY_DEVICE;

/*_Check_return_*/ HRESULT NuiGetMicrophoneArrayDevices( _Out_opt_cap_post_count_(size, *piDeviceCount) PNUI_MICROPHONE_ARRAY_DEVICE pDeviceInfo, _In_ int size,  _Out_ int *piDeviceCount);

typedef struct
{
    wchar_t szDeviceName[MAX_DEV_STR_LEN];
    int iDeviceIndex;
    bool fDefault;
} NUI_SPEAKER_DEVICE, *PNUI_SPEAKER_DEVICE;

/*_Check_return_*/ HRESULT NuiGetSpeakerDevices( _Out_opt_cap_post_count_(size, *piDeviceCount) PNUI_SPEAKER_DEVICE pDeviceInfo, _In_ int size,  _Out_ int *piDeviceCount);
#endif//
#ifdef NUI_COLOR_CAMERA
/*_Check_return_*/ HRESULT NUIAPI NuiCreateCoordinateMapperFromParameters(
    _In_ ULONG dataByteCount, 
    _In_ void* pData,
    _Out_ INuiCoordinateMapper **ppCoordinateMapper);
#endif//

#endif// 0

#endif// _INTERFACE_DEFINED_

} //namespace NuiSinect


