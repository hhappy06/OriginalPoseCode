#include "stdafx.h"
#include "NuiSinect.h"
#include "RandomForest.h"
#include "BodyPartClassifier.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <thread>
#include "INuiImageDevice.h"

// D2D绘图用结构RGBX, 每像素4字节数据
const int g_BytesPerPixel = 4;
// D2D绘制骨骼时的线宽
static const float g_JointThickness = 6.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;

MainWindow *MainWindow::g_mainWindow = 0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)

{
    MainWindow::g_mainWindow = this;
    ui->setupUi(this);
    ui->depthLabel->setBackgroundRole(QPalette::Base);
    ui->depthLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->depthLabel->setScaledContents(true);

    ui->bodyPartLabel->setBackgroundRole(QPalette::Base);
    ui->bodyPartLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->bodyPartLabel->setScaledContents(true);

    ui->foregroundLabel->setBackgroundRole(QPalette::Base);
    ui->foregroundLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->foregroundLabel->setScaledContents(true);

    ui->skeletonLabel->setBackgroundRole(QPalette::Base);
    ui->skeletonLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->skeletonLabel->setScaledContents(true);
//    scrollArea = new QScrollArea;
//    scrollArea->setBackgroundRole(QPalette::Dark);
//    scrollArea->setWidget(imageLabel);
//    setCentralWidget(scrollArea);
    initPainter();
    m_pBPC = new BodyPartClassifier();
    if (!m_pBPC->BuildForestFromResource())
    {
        assert(0);
        //AfxMessageBox("模型文件加载失败!");
    }
    else
    {
//        SetCaptureMode(CAPTURE_MODE_CAPTURE_ENABLE);
//        SetCaptureMode(CAPTURE_MODE_KINECT);
    }

    // 初始化NUI设备
    int res;
    m_pNuiImageDevice = INuiImageDevice::CreateDevice(NUI_DEVICE_VIDEO_FILE);
    ASSERT(m_pNuiImageDevice);

    res = m_pNuiImageDevice->InitDevice();
    if (res != 0)
    {
        // 初始化错误
        std::string str;
        //str.Format("VideoFIle初始化失败! res = %08X", res);
       // AfxMessageBox(str);
        ASSERT(0);
        return;

    }

    m_calcSkeleton =  new calcSkeleton(m_pNuiImageDevice, m_pBPC);
   // assert(m_calcSkeleton.m_pNuiImageDevice != NULL);
    m_calcSkeletonThread1 = new std::thread (&calcSkeleton::run, m_calcSkeleton, this);
   // thread1.detach();

}

MainWindow::~MainWindow()
{
    m_calcSkeleton->m_bStopThread = true;
    if (m_calcSkeletonThread1->joinable())
    {
        m_calcSkeletonThread1->join();
    }
    delete m_calcSkeleton;
    delete m_calcSkeletonThread1;
    m_pNuiImageDevice->UnInit();
    INuiImageDevice::destroyDevice();

    delete m_pBPC;
    delete ui;
}

void MainWindow::showSkeleton(void * a_qimage)
{
   // QImage * pqi = (QImage*)a_qimage;
    //ui->depthLabel->setImage((QImage*)a_qimage);
    ui->skeletonLabel->setPixmap(*((QPixmap*)(a_qimage)));
  //  delete (QImage*)a_qimage;
}


void MainWindow::showDepthImage(void * a_qimage)
{
    QImage * pqi = (QImage*)a_qimage;
    //ui->depthLabel->setImage((QImage*)a_qimage);
    ui->depthLabel->setPixmap(QPixmap::fromImage(*(QImage*)a_qimage));
    delete (QImage*)a_qimage;
}

void MainWindow::showForegroundImage(void *a_qimage)
{
    QImage * pqi = (QImage*)a_qimage;
    //ui->depthLabel->setImage((QImage*)a_qimage);
    ui->foregroundLabel->setPixmap(QPixmap::fromImage(*(QImage*)a_qimage));
    delete (QImage*)a_qimage;
}

void MainWindow::showBodyPart(void * a_qimage)
{
    QImage * pqi = (QImage*)a_qimage;
    //ui->depthLabel->setImage((QImage*)a_qimage);
    ui->bodyPartLabel->setPixmap(QPixmap::fromImage(*(QImage*)a_qimage));
    delete (QImage*)a_qimage;
}

void MainWindow::initPainter()
{
//    QPen pen();
//    pen.setColor();
//    m_painter =  new QPainter();
   // m_painter->setPen();
}



//#define MY_DrawBone(A, B) pRenderTarget->DrawLine( D2D1::Point2F(static_cast<float>(joint[A].x), static_cast<float>(joint[A].y)), D2D1::Point2F(static_cast<float>(joint[B].x), static_cast<float>(joint[B].y)), m_pDrawSkeleton->m_pBrushBoneTracked, g_TrackedBoneThickness/2 );
//#define MY_DrawBone_With_Check(A, B) if(joint[A].w&&joint[B].w){pRenderTarget->DrawLine( D2D1::Point2F(static_cast<float>(joint[A].x), \
//    static_cast<float>(joint[A].y)), D2D1::Point2F(static_cast<float>(joint[B].x), static_cast<float>(joint[B].y)), \
//    m_pDrawSkeleton->m_pBrushBoneTracked, g_TrackedBoneThickness/2 );}

#define MY_DrawBone_With_Check(A, B) if(joint[A].w&&joint[B].w){painter.drawLine( joint[A].x, \
    joint[A].y, joint[B].x, joint[B].y) ;}

void MainWindow::DrawSkeleton1(Vector4 * jointf, Vector4 * centroidf)
{
    //QImage qi(320, 240, QImage::Format_RGB888);
    QPixmap qi(320, 240);
    QPainter painter(&qi);
    // we found a skeleton, re-start the skeletal timer
//    m_bScreenBlanked = false;
//    m_LastSkeletonFoundTime = timeGetTime( );

//    RECT rct;
//    GetDlgItem(IDC_SKELETALVIEW)->GetClientRect(&rct);
//    int width = rct.right;
//    int height = rct.bottom;

//    // Endure Direct2D is ready to draw
//    HRESULT hr = m_pDrawSkeleton->EnsureResources( );
//    if ( FAILED( hr ) )
//    {
//        return;
//    }

    Vector4I centroid[31];
    for (int bpid = 0; bpid < 31; bpid ++)
    {
        BodyPartClassifier::WorldToImageSpace(centroidf[bpid], centroid[bpid], 240, 320);
    }
    Vector4I joint[20];
    for (int jid = 0; jid < 20; jid ++)
    {
        BodyPartClassifier::WorldToImageSpace(jointf[jid], joint[jid], 240, 320);
    }

//    ID2D1HwndRenderTarget * pRenderTarget = m_pDrawSkeleton->RenderTarget();
//    pRenderTarget->BeginDraw();
//    pRenderTarget->Clear( );

    // 绘制身体部位中心点
    for ( int i = 0 ; i < 31; i++ )
    {
//        // draw body part centroids
//        D2D1_ELLIPSE ellipse = D2D1::Ellipse(
//            D2D1::Point2F(static_cast<float>(centroid[i].x), static_cast<float>(centroid[i].y)),
//            //SkeletonToScreen( SkeletonFrame.SkeletonData[i].Position, width, height ),
//            g_JointThickness/2,
//            g_JointThickness/2
//            );
//        pRenderTarget->DrawEllipse(ellipse,m_pDrawSkeleton->m_pBrushBodyPartCentroidTracked);//drawEllipse
        painter.drawEllipse(centroid[i].x, centroid[i].y, g_JointThickness/2, g_JointThickness/2);
    }

    // 绘制骨骼
    // Render Torso
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT );

    // Left Arm
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT );

    // Right Arm
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT );

    // Left Leg
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT );

    // Right Leg
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT );
    MY_DrawBone_With_Check(NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT );

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
    ui->skeletonLabel->setPixmap(qi);
    //delete (QImage*)a_qimage;
}



#define MY_DrawBone_With_Tracking_Check(A, B) {\
    if(joint_state[A]!=NUI_SKELETON_POSITION_NOT_TRACKED&&joint_state[B]!=NUI_SKELETON_POSITION_NOT_TRACKED)\
    {\
    painter.drawLine( joint[A].x, joint[A].y, joint[B].x, joint[B].y) ;\
    }\
    else {;}\
}



void MainWindow::time_bodyPartClassification(double a)
{
    ui->label_bodyPartClassification->setText(QString::number(a));
}

void MainWindow::time_centroid(double a)
{
    ui->label_centroid->setText(QString::number(a));
}

void MainWindow::time_skeleton(double a)
{
    ui->label_skeleton->setText(QString::number(a));
}

void MainWindow::time_smooth(double a)
{
    ui->label_smooth->setText(QString::number(a));
}

void MainWindow::time_imageLoad(double a)
{
    ui->label_imageLoad->setText(QString::number(a));
}

void MainWindow::time_fps(double a)
{
    ui->label_fraps->setText(QString::number(a));
}

void MainWindow::time_totalCalculation(double a)
{
    ui->label_totalCaculation->setText(QString::number(a));
    ui->label_fraps->setText(QString::number(1000.0 / a));
}
