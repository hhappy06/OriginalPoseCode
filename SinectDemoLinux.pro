#-------------------------------------------------
#
# Project created by QtCreator 2013-11-24T15:47:27
#
#-------------------------------------------------

QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SinectDemoLinux
TEMPLATE = app
CONFIG += c++11

QMAKE_CXXFLAGS += -pthread
#LIBS+= -stdlib=libc++
INCLUDEPATH += zlib/ src/

SOURCES += main.cpp\
        mainwindow.cpp \
    zlib/adler32.c \
    zlib/crc32.c \
    zlib/deflate.c \
    zlib/inffast.c \
    zlib/inflate.c \
    zlib/inftrees.c \
    zlib/trees.c \
    zlib/zutil.c \
    src/BGRStage.cpp \
    src/BodyPartClassifier.cpp \
    src/CentroidsStage.cpp \
    src/ExemplarStage.cpp \
    src/INuiImageDevice.cpp \
    src/IPipelineStage.cpp \
    src/ModelFittingStage.cpp \
    src/NuiImageDeviceVideoFile.cpp \
    src/NuiSensorKinect.cpp \
    src/PipelineOutputStage.cpp \
    src/PipelineSourceStage.cpp \
    src/RandomForest.cpp \
    src/SkeletalTracker.cpp \
    src/Util.cpp \
    stdafx.cpp \
    calcskeleton.cpp

HEADERS  += mainwindow.h \
    zlib/crc32.h \
    zlib/deflate.h \
    zlib/gzguts.h \
    zlib/inffast.h \
    zlib/inffixed.h \
    zlib/inflate.h \
    zlib/inftrees.h \
    zlib/trees.h \
    zlib/zconf.h \
    zlib/zlib.h \
    zlib/zutil.h \
    src/BGRStage.h \
    src/BodyPartClassifier.h \
    src/CentroidsStage.h \
    src/ExemplarStage.h \
    src/INuiImageDevice.h \
    src/IPipelineStage.h \
    src/ModelFittingStage.h \
    src/NuiImageDeviceVideoFile.h \
    src/NuiSensorKinect.h \
    src/NuiSinect.h \
    src/PipelineOutputStage.h \
    src/PipelineSourceStage.h \
    src/RandomForest.h \
    src/SkeletalTracker.h \
    src/Util.h \
    stdafx.h \
    targetver.h \
    calcskeleton.h



RESOURCES += \
    resource.qrc
