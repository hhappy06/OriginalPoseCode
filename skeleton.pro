#-------------------------------------------------
#
# Project created by QtCreator 2014-03-27T08:58:32
#
#-------------------------------------------------

QT       -= gui

TARGET = skeleton
TEMPLATE = lib
QMAKE_CXXFLAGS += -std=c++11
DEFINES += SKELETON_LIBRARY
INCLUDEPATH +=./src \
        ./zlib \
         /usr/local/cuda-5.5/include
android{
    INCLUDEPATH += "/home/yl/workspace/adrenosdk/adrenosdk-3-4-linux/Development/Inc/CL"
    INCLUDEPATH += "D:/AdrenoSDK/Development/Inc"
   # LIBS += -L/home/yl/workspace/adrenosdk -lOpenCL
    LIBS += -L"D:/AdrenoSDK/Development/Lib/Android/" -lOpenCL
}
Release
{
#    QMAKE_CXXFLAGS += -O3
}

SOURCES += stdafx.cpp \
    skeletonObj.h \
    skeletonInterface.cpp \
zlib/adler32.c \
zlib/crc32.c \
zlib/deflate.c \
zlib/inffast.c \
zlib/inflate.c \
zlib/inftrees.c \
zlib/trees.c \
zlib/zutil.c \
src/BodyPartClassifier.cpp \
src/buildForest.cpp \
src/clUtil.cpp \
src/opencl.cpp \
src/RandomForest.cpp \
src/Util.cpp

HEADERS += stdafx.h \
    skeletonObj.h \
    skeletonInterface.h \
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
src/BodyPartClassifier.h \
src/buildForest.h \
src/clUtil.h \
src/NuiSinect.h \
src/opencl.h \
src/RandomForest.h \
src/Util.h

RESOURCES += \
    resource_qt.qrc

