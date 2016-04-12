#ifndef SKELETONINTERFACE_H
#define SKELETONINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#ifdef EXPORT_SKELETON
#define BASEAPI __declspec(dllexport)
#else
#define  BASEAPI __declspec(dllimport)
#endif
#else
#define BASEAPI

#endif

	bool out_to_file(const char * afilename, unsigned short * aout);


BASEAPI	void get_human_part(void *aathis,  unsigned char * out_part);//@xu-liÌí¼Óº¯Êý

BASEAPI void initSkeletonObj(void ** a_this, bool UseGpu = false);
BASEAPI void releaseSkeletonObj(void * aathis);
BASEAPI int calcSkeletonFromForeground(void * a_this,unsigned short * depthDataWithIndex, NUI_SKELETON_FRAME *SinectSkeletons, int file_id );
BASEAPI int calcSkeletonFromForeground_sf(void * a_this,unsigned short * depthDataWithIndex, NUI_SKELETON_FRAME *SinectSkeletons, NUI_SKELETON_FRAME *SinectSkeletons_sf, int file_id );//@xu-li
BASEAPI void ClassifyOnePixelInterface(void * a_this, const unsigned short * pImage, int y, int x, const float player_scale, const int DepthUnusual, int * maxclass);
BASEAPI void setSkeletonUseGpu(void * a_this,bool useGpu);
BASEAPI void getvCentroidInterface(void * a_this, Vector4 Centroid[6][31]);
BASEAPI void rawToLabelinterface(void * a_this, unsigned char * pLabel, const unsigned short * pImage, int labelConvMode, unsigned short * pLabelRaw );
BASEAPI void clearLabelRaw( void * a_pSkObj );
BASEAPI void setSkeletonUseHugeArray(void * aa, bool useHugeArray);
BASEAPI void setDoMeanshift(void * aa, bool doMeanshift);
BASEAPI void setGrayGravityCentroid(void * aa, bool GrayGravityCentroid);
BASEAPI void setFolderFrame(int folder, int framecnt);
//BASEAPI void skeletonSetOpenmp(bool enabled);
//BASEAPI void skeletonSetNoBodyPartProbility(bool enabled);
#ifdef __cplusplus
}
#endif

#endif
// SKELETONINTERFACE_H
