
#include "stdafx.h"
#include <cmath>
#include "Util.h"
#include <thread>
#include "NuiSinect.h"
#include "RandomForest.h"
#include "BodyPartClassifier.h"
#include "calcskeleton.h"
#include <mutex>
#include <QImage>
#include <chrono>

#define BST_UNCHECKED      0x0000
#define BST_CHECKED        0x0001
#define BST_INDETERMINATE  0x0002
#define BST_PUSHED         0x0004
#define BST_FOCUS          0x0008

// 控制: 是否生成并显示BodyPart
#define DRAW_BODY_PART

//lookups for color tinting based on player index
static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };


// D2D绘图用结构RGBX, 每像素4字节数据
const int g_BytesPerPixel = 4;
// D2D绘制骨骼时的线宽
static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;


calcSkeleton::calcSkeleton(INuiImageDevice * a_pNuiImageDevice, BodyPartClassifier * a_pclf)
	:m_bStopThread(false)
{
    // 初始化
    // 深度图设备
    m_pNuiImageDevice = a_pNuiImageDevice;
    m_pBPC = a_pclf;
    //    // D2D绘图线程用HANDLE
    //    m_hThDrawDepth    = NULL;
    //    m_hThDrawBodyPart = NULL;
    //    m_hThDrawSkeleton = NULL;
    //    m_hEvDrawDepth    = NULL;
    //    m_hEvDrawBodyPart = NULL;
    //    m_hEvDrawSkeleton = NULL;
    //    m_hEvDrawDepthStop    = NULL;
    //    m_hEvDrawBodyPartStop = NULL;
    //    m_hEvDrawSkeletonStop = NULL;

    // FPS
    m_LastDepthFPStime = 0;
    m_DepthFramesTotal = 0;
    m_LastDepthFramesTotal = 0;

    // 算法模块数据
    m_pLabel = NULL;
    m_pLabelRaw = NULL;
    m_pDepthFrame = NULL;

    // 设置默认数据连接方式
    //SetCaptureMode(CAPTURE_MODE_VIDEO_FILE);
    //m_pSelf = this;

    // 初始化算法模块数据
    m_pLabel    = new BYTE[160*120];
    m_pLabelRaw = new USHORT[160*120*31];
    m_pDepthFrame = new USHORT[320*240];
    assert(m_pLabel);
    assert(m_pLabelRaw);
    assert(m_pDepthFrame);
    m_qi = new QPixmap(160,120);

    //thread1 = new std::thread(&MainWindow::_)
}

calcSkeleton::~calcSkeleton()
{
    m_bStopThread = true;
    delete [] m_pLabel;
    delete [] m_pLabelRaw;
    delete [] m_pDepthFrame;
}

void calcSkeleton::DrawSkeleton(void * a_pSkeleton, void *a_pCentroids)
{
    Util tu("MainWindow: display skeleton");
    const NUI_SKELETON_FRAME * pSkeleton = (const NUI_SKELETON_FRAME * )a_pSkeleton;
    const NUI_CENTROIDS_STAGE_DATA * pCentroids = (const NUI_CENTROIDS_STAGE_DATA *)a_pCentroids;
    //QImage qi(640, 480, QImage::Format_RGB888);

    m_qi->fill(QColor(200,200,200));
    QPainter painter(m_qi);


//    D2D1_SIZE_U size = m_pDrawSkeleton->GetControlSize();
    int height = 120, width = 160;

//    ID2D1HwndRenderTarget * pRenderTarget = m_pDrawSkeleton->RenderTarget();
//    pRenderTarget->BeginDraw();
//    pRenderTarget->Clear( );

    for (int pid = 0; pid < 6; pid ++)
    {
        if (pSkeleton->SkeletonData[pid].eTrackingState == NUI_SKELETON_NOT_TRACKED)
        {
            continue;
        }
        // 身体部位中心点转换为图像坐标系
        Vector4I centroid[31];
        for (int bpid = 0; bpid < 31; bpid ++)
        {
            BodyPartClassifier::WorldToImageSpace(pCentroids->BodyPartCentroids[pid].Centroids[bpid],
                                                  centroid[bpid], height, width);
        }
//        // 关节转换为图像坐标系
//        Vector4I joint[20];
//        NUI_SKELETON_POSITION_TRACKING_STATE joint_state[20];
//        for (int jid = 0; jid < 20; jid ++)
//        {
//            BodyPartClassifier::WorldToImageSpace(pSkeleton->SkeletonData[pid].SkeletonPositions[jid], joint[jid], height, width);
//            joint_state[jid] = pSkeleton->SkeletonData[pid].eSkeletonPositionTrackingState[jid];
//        }

//        // 绘制骨骼
//        // Render Torso
////        painter.drawLine( joint[NUI_SKELETON_POSITION_HEAD].x, joint[NUI_SKELETON_POSITION_HEAD].y,
////                          joint[NUI_SKELETON_POSITION_HEAD].x, joint[NUI_SKELETON_POSITION_HEAD].y) ;
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_HEAD );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT );

//        // Left Arm
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT );

//        // Right Arm
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT );

//        // Left Leg
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT );

//        // Right Leg
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT );
//        MY_DrawBone_With_Tracking_Check(NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT );

        // 绘制身体部位中心点
        for ( int i = 0 ; i < 31; i++ )
        {
            if (centroid[i].x == 0 && centroid[i].y == 0 && centroid[i].z == 0)
            {
                continue;
            }
            // draw body part centroids
//            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
//                D2D1::Point2F(static_cast<float>(centroid[i].x), static_cast<float>(centroid[i].y)),
//                //SkeletonToScreen( SkeletonFrame.SkeletonData[i].Position, width, height ),
//                g_JointThickness,
//                g_JointThickness
//                );
//            pRenderTarget->DrawEllipse(ellipse,m_pDrawSkeleton->m_pBrushBodyPartCentroidTracked);
            painter.drawEllipse(centroid[i].x, centroid[i].y, g_JointThickness, g_JointThickness);
        }

//        // 绘制关节
//        for ( int i = 0 ; i < 20; i++ )
//        {
//            if (joint_state[i] == NUI_SKELETON_POSITION_NOT_TRACKED)
//            {
//                continue;
//            }
////            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
////                D2D1::Point2F(static_cast<float>(joint[i].x), static_cast<float>(joint[i].y)),
////                g_JointThickness,
////                g_JointThickness
////                );
////            pRenderTarget->DrawEllipse(ellipse,m_pDrawSkeleton->m_pBrushJointTracked);
//            painter.drawEllipse(joint[i].x, joint[i].y, g_JointThickness/2, g_JointThickness/2);
//        }
    }

    //////////////////////////////////////////////////////////////////////////
//    hr = pRenderTarget->EndDraw();

//    // Device lost, need to recreate the render target
//    // We'll dispose it now and retry drawing
//    if ( hr == D2DERR_RECREATE_TARGET )
//    {
//        hr = S_OK;
//        m_pDrawSkeleton->DiscardResources();
//        return;
//    }
   // ui->skeletonLabel->setPixmap(m_qi);
}

void calcSkeleton::run(void * a_mainwindow)
{
    int cnt = 1;
    assert(m_pNuiImageDevice != NULL);
    // std::lock_guard<std::mutex> lg(m_pNuiImageDevice->m_loadImage_sync);
    while (m_pNuiImageDevice->m_loadState == INuiImageDevice::LOADSTATE_NOTINTHREAD)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(300));
    }

    while(!m_bStopThread)
    {
        Util k("run:");


        {
            Util k("get&copy DepthMap:");

            //   std::lock_guard<std::mutex> lk(m_pNuiImageDevice->m_depthMapMutex);

            std::unique_lock<std::mutex> ul(m_pNuiImageDevice->m_bufferMutex);
            m_pNuiImageDevice->m_calcState = INuiImageDevice::CALCSTATE_WAITING;
            INuiImageDevice * pNuiImageDevice = m_pNuiImageDevice;
            m_pNuiImageDevice->m_getdepthCondVar.wait(ul,
                                                      [pNuiImageDevice]
            {
                return pNuiImageDevice->m_bufferReady.load();
            }
            );


           // m_pNuiImageDevice->m_calcState = INuiImageDevice::CALCSTATE_GETING;
            //std::lock_guard<std::mutex> lg(m_pNuiImageDevice->m_depthMapMutex);
            const USHORT * pDepthFrameTmp = m_pNuiImageDevice->GetDepthMap();
            memcpy(m_pDepthFrame, pDepthFrameTmp, 320*240*2);
            m_pNuiImageDevice->m_bufferReady.store(false);
        }
       // m_pNuiImageDevice->m_calcState = INuiImageDevice::CALCSTATE_CALCING;
        //while (m_pNuiImageDevice->m_loadState != INuiImageDevice::LOADSTATE_WAITING) {std::this_thread::yield();}
        // assert(m_pNuiImageDevice->m_loadState == INuiImageDevice::LOADSTATE_WAITING);
        // m_pNuiImageDevice->m_getdepthCondVar.notify_one();

        this->Nui_GotDepthAlert(a_mainwindow);
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        QMetaObject::invokeMethod((QObject*)a_mainwindow, "time_totalCalculation", Qt::AutoConnection,
                                  Q_ARG(double, k.time()) );
        //        QMetaObject::invokeMethod((QObject*)a_mainwindow, "time_fps", Qt::AutoConnection,
        //                                  Q_ARG(double, k.time()) );
        cnt++;
    }
    m_pNuiImageDevice->m_calcState = INuiImageDevice::CALCSTATE_NOTINTHREAD;
}


QImage * convert2qimage(BYTE * rgbrun, int cols, int rows, int step)
{
    QImage* m_pimage2Draw_qt = new QImage(rgbrun,
                                          cols,
                                          rows,
                                          step,
                                          QImage::Format_RGB888);
    // cout << "a_pimage2Draw_mat->step" << a_pimage2Draw_mat->step<< endl;
    //    }
    //    CvMat c_img = *a_pimage2Draw_mat;
    //    cvConvertImage(&c_img, &c_img, CV_CVTIMG_SWAP_RB);
    return m_pimage2Draw_qt;
}

#ifdef DRAW_BODY_PART
#define LABEL_DATA m_pLabel
#else
#define LABEL_DATA 0
#endif

#if 0
INT X_old, Y_old;
#else
INT dx_old, dy_old;
#endif
DWORD dwMouseEventTimeOld;
DWORD dwFrameNumber = 0;
#define MEASURE_TIME(X) X
//#define MEASURE_TIME(X)
bool calcSkeleton::Nui_GotDepthAlert(void * a_mainwindow)
{
    //MEASURE_TIME(Util tu("Nui_GotDepthAlert: total"));
    //const int frameWidth = 320;
    // const int frameHeight = 240;
    //    {
    //        std::lock_guard<std::mutex> lk(m_pNuiImageDevice->m_depthMapMutex);
    //        const USHORT * pDepthFrameTmp = m_pNuiImageDevice->GetDepthMap();
    //        memcpy(m_pDepthFrame, pDepthFrameTmp, 320*240*2);

    //    }

    QImage * bgrQimage;
    const USHORT * pBufferRun = m_pDepthFrame;
    const USHORT * pBufferEnd = pBufferRun + (320 * 240);
    dwFrameNumber++;
    std::cout <<  ("Nui_GotDepthAlert begin dwFrameNumber =%d", dwFrameNumber);
    if (1)
    {
        //  MEASURE_TIME(Util tu("Nui_GotDepthAlert: depth to color"));

        // draw the bits to the bitmap
        BYTE * rgbrun = m_depthRGBX;

        //   ASSERT( frameWidth * frameHeight * g_BytesPerPixel <= ARRAYSIZE(m_depthRGBX) );

        //STEP1: 获取深度图
        int ftCnt = 0;
        while ( pBufferRun < pBufferEnd )
        {
            USHORT depth     = *pBufferRun;
            USHORT realDepth = (depth >> 3);
            USHORT player    = (depth & 0x0007);
            if (realDepth > 0)
            {
                ftCnt ++;
            }
            // transform 13-bit depth information into an 8-bit intensity appropriate
            // for display (we disregard information in most significant bit)
            BYTE intensity = static_cast<BYTE>(~(realDepth >> 4));
            //BYTE intensity = static_cast<BYTE>(realDepth >> 5);

            // tint the intensity by dividing by per-player values
            *(rgbrun++) = intensity >> g_IntensityShiftByPlayerB[player];
            *(rgbrun++) = intensity >> g_IntensityShiftByPlayerG[player];
            *(rgbrun++) = intensity >> g_IntensityShiftByPlayerR[player];

            // no alpha information, skip the last byte
            // ++rgbrun;

            ++pBufferRun;
        }
        assert(ftCnt > 0);
    }

    if (1)
    {
        // MEASURE_TIME(Util tu("Nui_GotDepthAlert: display depth"));

        // 显示深度图
        //m_pDrawDepth->Draw( m_depthRGBX, frameWidth * frameHeight * g_BytesPerPixel );
        //PostMessage(WM_DRAW_DEPTH);
        //SendMessage(WM_DRAW_DEPTH);
        // SetEvent(m_hEvDrawDepth);
        bgrQimage = convert2qimage(m_depthRGBX, 320, 240, 320 * 3);
        QMetaObject::invokeMethod((QObject*)a_mainwindow,
                                  "showForegroundImage",
                                  Qt::AutoConnection,
                                  Q_ARG(void *, bgrQimage)
                                  );

    }

    // STEP2: 检查微软深度图有无PlayerIndex, 最终应该是BGR模块检测
    BOOL bHavePlayer = FALSE;
    pBufferRun = m_pDepthFrame;
    while ( pBufferRun < pBufferEnd )
    {
        USHORT depth     = *pBufferRun;
        if(depth & 0x0007)
        {
            bHavePlayer = TRUE;
            break;
        }
        ++pBufferRun;
    }
    // 耗时估计:
    // STEP1之前: 12% ~ 13%
    // STEP3之前: 12% ~ 13%
    // STEP4之前: 23% ~ 25%
    // STEP5之前: 66% ~ 70%, STEP4消耗资源太多

    if (bHavePlayer)
    {

        if (0)
        {
            char szFileName[100];

            sprintf(szFileName, "./depth/Depth_Two_%05d.depth", dwFrameNumber);

            //TRACE("%s\n", szFileName);
            FILE * fp = NULL;
#ifdef WIN32
            fopen_s(&fp, szFileName, "wb");
#else
            fp = fopen(szFileName, "wb");
#endif
            if (fp)
            {
                fwrite(m_pDepthFrame, 1, 320*240*2, fp);
                fclose(fp);
            }
            else
            {
                assert(0);
            }
        }

        {
            // STEP3: 人体部位分类
            MEASURE_TIME(Util tu("Nui_GotDepthAlert: bodyPartClassification"));
            m_pBPC->ExemplarStage_RunFrame(1.0f, m_pDepthFrame, m_pLabelRaw, 1, LABEL_DATA, false);
            QMetaObject::invokeMethod((QObject*)a_mainwindow, "time_bodyPartClassification", Qt::AutoConnection,
                                      Q_ARG(double, tu.time()) );
        }

        if (0)
        {
            char szFileName[100];
#ifdef ANDROID
            sprintf(szFileName, "/storage/extSdCard/ubuntu/label/Depth_Two_%05d.label_raw", dwFrameNumber);
#else
            sprintf(szFileName, "./Depth_Two_%05d.label_raw", dwFrameNumber);
#endif
            //TRACE("%s\n", szFileName);
            FILE * fp = NULL;
#ifdef WIN32
            fopen_s(&fp, szFileName, "wb");
#else
            fp = fopen(szFileName, "wb");
#endif
            if (fp)
            {
                fwrite(m_pLabelRaw, 1, 160*120*31*2, fp);
                fclose(fp);
            }
            else
            {
                assert(0);
            }
        }

        {
            // STEP4: 提取部位中心
            MEASURE_TIME(Util tu("Nui_GotDepthAlert: centroid"));
            //m_pBPC->CentroidsStage_RunFrame(pDepthFrame, m_pLabelRaw, m_centroid);
            m_pBPC->CentroidsStage_RunFrame(m_pDepthFrame, m_pLabelRaw, &m_CentroidData, a);
            QMetaObject::invokeMethod((QObject*)a_mainwindow, "time_centroid", Qt::AutoConnection,
                                      Q_ARG(double, tu.time()) );
        }

        {
            // Vector4 v = m_CentroidData.BodyPartCentroids[0].Centroids[24];
            //TRACE("BODY_PART_R_ELBOW, %5d, %6.3f, %6.3f, %6.3f\n", dwFrameNumber, v.x, v.y, v.z);
        }

        {
            // STEP5: 提取人体骨骼
            MEASURE_TIME(Util tu("Nui_GotDepthAlert: skeleton"));
            //m_pBPC->ModelFittingStage_RunFrame(m_centroid, m_joints);
            m_pBPC->ModelFittingStage_RunFrame(&m_CentroidData, &m_SkeletonData);
            QMetaObject::invokeMethod((QObject*)a_mainwindow, "time_skeleton", Qt::AutoConnection,
                                      Q_ARG(double, tu.time()) );
        }

        if (1)
        {
            // STEP6: 平滑人体骨骼
            MEASURE_TIME(Util tu("Nui_GotDepthAlert: smooth"));
            NUI_TRANSFORM_SMOOTH_PARAMETERS SmoothingParams;
            SmoothingParams.fSmoothing          = 0.8f; //0.5f
            SmoothingParams.fCorrection         = 0.5f; //0.5f
            SmoothingParams.fPrediction         = 0.5f; //0.5f
            SmoothingParams.fJitterRadius       = 0.05f; //0.05f
            SmoothingParams.fMaxDeviationRadius = 0.04f; //0.04f
            if (S_OK != m_pBPC->NuiTransformSmooth(&m_SkeletonData, &SmoothingParams))
            {
                assert(0);
            }
            QMetaObject::invokeMethod((QObject*)a_mainwindow, "time_smooth", Qt::AutoConnection,
                                      Q_ARG(double, tu.time()) );
        }
        //////////////////////////////////////////////////////////////////////////
#if 1 //游戏控制
        //GameControlFruitNinja(&m_SkeletonData, 1440, 900);
#endif
        //////////////////////////////////////////////////////////////////////////
#ifdef DRAW_BODY_PART
        if (1)
        {
            // STEP7: 显示人体部位
            // MEASURE_TIME(Util tu("Nui_GotDepthAlert: display bodypart"));
            if (1/*不绘制BP图*/)
            {
                for (int pid = 0; pid < 6; pid ++)
                {
                    for (int bpid=0; bpid<31; bpid++)
                    {
                        if (m_CentroidData.BodyPartCentroids[pid].eBodyPartPositionTrackingState[bpid] != NUI_BODY_PART_POSITION_TRACKED)
                        {
                            continue;
                        }
                        Vector4I centroidI;
                        //BodyPartClassifier::WorldToImageSpace(m_centroid[bpid], centroidI);
                        BodyPartClassifier::WorldToImageSpace(m_CentroidData.BodyPartCentroids[pid].Centroids[bpid], centroidI);
                        int cx = centroidI.x;
                        int cy = centroidI.y;
                        if (cx >=0 && cx < 160 && cy >=0 && cy < 120)
                        {
                            m_pLabel[160*cy + cx] = 31; //31: centroid color id
                        }
                        else
                        {
                            // 中心点接近或已经离开画面，不画了
                            // assert(0);
                        }
                    }
                }
                //// 绘制BP图
                //m_pBPC->GetLabelImage(m_pLabel, m_BodyPartRGBX);
                ////m_pDrawBodyPart->Draw( m_BodyPartRGBX, 160*120*4 );
                ////PostMessage(WM_DRAW_BODY_PART);
                ////SendMessage(WM_DRAW_BODY_PART);
                //bgrQimage = convert2qimage(m_BodyPartRGBX, 160, 120, 160 * 3);
                //QMetaObject::invokeMethod((QObject*)a_mainwindow,
                //                          "showBodyPart",
                //                          Qt::AutoConnection,
                //                          Q_ARG(void *, bgrQimage)
                //                          );
                //                SetEvent(m_hEvDrawBodyPart);
            }
        }
#endif

        {
            // STEP8: 显示人体骨架
            //MEASURE_TIME(Util tu("Nui_GotDepthAlert: display skeleton"));
            //DrawSkeleton(joints, centroid);
            //PostMessage(WM_DRAW_SKELETON);
            //SendMessage(WM_DRAW_SKELETON);
            // SetEvent(m_hEvDrawSkeleton);

            // bgrQimage = convert2qimage(m_BodyPartRGBX, 320, 240, 320 * 3);
//#ifndef ANDROID
            DrawSkeleton((void*)&m_SkeletonData, (void*)&m_CentroidData);
            QMetaObject::invokeMethod((QObject*)a_mainwindow,
                                      "showSkeleton",
                                      Qt::AutoConnection,
                                      Q_ARG(void *, m_qi)

                                      );
//#endif
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Once per second, display the depth FPS
    m_DepthFramesTotal ++;
    //    double t = gettimeus();
    //    if ( (t - m_LastDepthFPStime) > 1000 )
    //    {
    //        //int fps = ((m_DepthFramesTotal - m_LastDepthFramesTotal) * 1000 + 500) / (t - m_LastDepthFPStime);
    //        //CWnd *wndFPS = GetDlgItem(IDC_FPS);
    //        //GetDlgItem(IDC_FPS)->SetDlgItemInt(static_cast<int>(IDC_FPS), static_cast<int>(fps), FALSE);
    //        //  SetDlgItemInt(IDC_FPS, fps);
    //        m_LastDepthFramesTotal = m_DepthFramesTotal;
    //        m_LastDepthFPStime = t;
    //    }

    //    // Blank the skeleton panel if we haven't found a skeleton recently
    //    if ( (t - m_LastSkeletonFoundTime) > 300 )
    //    {
    //        //        if ( !m_bScreenBlanked )
    //        //        {
    //        //            // 骨骼显示黑屏
    //        //            ID2D1HwndRenderTarget * pRenderTarget;
    //        //            pRenderTarget = m_pDrawSkeleton->RenderTarget();
    //        //            if ( NULL != pRenderTarget )
    //        //            {
    //        //                pRenderTarget->BeginDraw();
    //        //                pRenderTarget->Clear( );
    //        //                pRenderTarget->EndDraw( );
    //        //            }
    //        //            pRenderTarget = m_pDrawSkeletonYZ->RenderTarget();
    //        //            if ( NULL != pRenderTarget )
    //        //            {
    //        //                pRenderTarget->BeginDraw();
    //        //                pRenderTarget->Clear( );
    //        //                pRenderTarget->EndDraw( );
    //        //            }
    //        //            pRenderTarget = m_pDrawSkeletonXZ->RenderTarget();
    //        //            if ( NULL != pRenderTarget )
    //        //            {
    //        //                pRenderTarget->BeginDraw();
    //        //                pRenderTarget->Clear( );
    //        //                pRenderTarget->EndDraw( );
    //        //            }
    //        //            // BodyPart显示也黑屏
    //        //            pRenderTarget = m_pDrawBodyPart->RenderTarget();
    //        //            if ( NULL != pRenderTarget )
    //        //            {
    //        //                pRenderTarget->BeginDraw();
    //        //                pRenderTarget->Clear( );
    //        //                pRenderTarget->EndDraw( );
    //        //            }
    //        //            m_bScreenBlanked = true;
    //        //        }
    //    }
    cout <<  ("go alert done : dwFrameNumber =%d ", dwFrameNumber);
    return true;
}
