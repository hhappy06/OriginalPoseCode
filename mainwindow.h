#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once


#include <QMainWindow>
#include "calcskeleton.h"
#include <QPainter>
#include <thread>
namespace Ui {
class MainWindow;
}




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow * g_mainWindow ;
public slots:
    void showDepthImage(void * a_qimage);
    void showForegroundImage(void * a_qimage);
    //void showSkeleton(void * a_qimage);
    void showBodyPart(void * a_qimage);
    void DrawSkeleton1(Vector4 * jointf, Vector4 * centroidf);
    //void DrawSkeleton(const NUI_SKELETON_FRAME * pSkeleton, const NUI_CENTROIDS_STAGE_DATA * pCentroids);
    //void DrawSkeleton(void * a_pSkeleton, void *a_pCentroids);
    void showSkeleton(void * a_qimage);

    void time_bodyPartClassification(double a);
    void time_centroid(double a);
    void time_skeleton(double a);
    void time_smooth(double a);
    void time_imageLoad(double a);
    void time_fps(double a);
    void time_totalCalculation(double a);

private:

    void initPainter();
    Ui::MainWindow *ui;
    calcSkeleton * m_calcSkeleton;
    INuiImageDevice * m_pNuiImageDevice;
    // Ëã·¨Ä£¿é
    BodyPartClassifier * m_pBPC;
    QPainter * m_painter;

    // Skeletal drawing
    QBrush *   m_pBrushJointTracked;
    QBrush *   m_pBrushJointInferred;
    QBrush *   m_pBrushBodyPartCentroidTracked;
    QBrush *   m_pBrushBodyPartCentroidInferred;
    QBrush *   m_pBrushBoneTracked;
    QBrush *   m_pBrushBoneInferred;
    QPointF    m_Points[NUI_SKELETON_POSITION_COUNT];

    std::thread * m_calcSkeletonThread1;

};

#endif // MAINWINDOW_H
