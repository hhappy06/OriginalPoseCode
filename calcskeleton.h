#ifndef CALCSKELETON_H
#define CALCSKELETON_H
#include "INuiImageDevice.h"
#include "BodyPartClassifier.h"
#include <QPainter>
#include <QPixmap>

typedef enum
{
    CAPTURE_MODE_KINECT = 1,
    CAPTURE_MODE_DEVICE,
    CAPTURE_MODE_DEVICE_PROXY,
    CAPTURE_MODE_VIDEO_FILE,
    CAPTURE_MODE_CAPTURE_DISABLE,
    CAPTURE_MODE_CAPTURE_ENABLE
} CAPTURE_MODE;
class calcSkeleton
{
public:
    calcSkeleton(INuiImageDevice * a_pNuiImageDevice, BodyPartClassifier *);
    ~calcSkeleton();
    void run(void * mainwindow);
    bool m_bStopThread ;
protected:
    void SetCaptureMode(CAPTURE_MODE eCaptureMode);
    bool Nui_GotDepthAlert(void * mainwindow);
    void DrawSkeleton(Vector4 * jointf, Vector4 * centroidf);
    void DrawSkeleton(const NUI_SKELETON_FRAME * pSkeleton, const NUI_CENTROIDS_STAGE_DATA * pCentroids);

    // �㷨����������(�ص�����)
    //static bool  _static_Nui_GotDepthAlert();

    void DrawSkeleton(void * a_pSkeleton, void *a_pCentroids);
    // Self
   // static MainWindow *  m_pSelf;
    // NUI Device
    INuiImageDevice * m_pNuiImageDevice;
protected:

    BYTE          m_depthRGBX[320*240*3];
    BYTE          m_BodyPartRGBX[160*120*3];
    BYTE          m_SkeletonRGBX[320*240*3];

    // ͳ��FPS��
    //HFONT m_hFontFPS;
    double m_LastDepthFPStime;
    double m_DepthFramesTotal;
    double m_LastDepthFramesTotal;

    // ���ƹ�����ʾ����
    DWORD m_LastSkeletonFoundTime;
    bool m_bScreenBlanked;

    // �㷨ģ��
    BodyPartClassifier * m_pBPC;

    // �㷨ģ������
    USHORT * m_pDepthFrame;
    unsigned char *  m_pLabel;
    unsigned short * m_pLabelRaw;
    //Vector4 m_centroid[31];
    //Vector4 m_joints[20];
    NUI_CENTROIDS_STAGE_DATA m_CentroidData;
    NUI_SKELETON_FRAME       m_SkeletonData;
    QPixmap  *m_qi;
   // QImage q;

};

#endif // CALCSKELETON_H
