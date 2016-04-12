#pragma once
#include "INuiImageDevice.h"
#include "IPipelineStage.h"
#include "NuiSinect.h"

namespace NuiSinect {

class BodyPartProbility
{
public:
    BodyPartProbility()
        : m_pProb(0)
    {
        USHORT * pProb = new USHORT[Rows() * Cols() * BodyPartCount()];
        if (pProb)
        {
            memset(pProb, 0, Rows() * Cols() * BodyPartCount() * sizeof(USHORT));
            m_pProb  = pProb;
        }
    }
    ~BodyPartProbility()
    {
        if (m_pProb)
        {
            delete m_pProb;
            m_pProb = 0;
        }
    }
    void Reset()
    {
        if (m_pProb)
        {
            memset(m_pProb, 0, Rows() * Cols() * BodyPartCount() * sizeof(USHORT));
        }
    }
    const USHORT * Get(int row, int col = 0) const
    {
        return &(m_pProb[row * IMAGE_WIDTH + col]);
    }
    const int & Rows() const
    {
        return IMAGE_HEIGHT;
    }
    const int & Cols() const
    {
        return IMAGE_WIDTH;
    }
    const int & BodyPartCount() const
    {
        return BODY_PART_COUNT;
    }
private:
    USHORT * m_pProb;
    static const int IMAGE_WIDTH = 160;
    static const int IMAGE_HEIGHT = 120;
    static const int BODY_PART_COUNT = 31;
};

class BodyPartLabeller
{
public:
    int LoadForest(const signed char *pszModelFilePath);
    int Predict(/*DepthFrame, LabelRaw, playerScale*/);
    int ConvertToLabel(/*LabelRaw, Label*/);
protected:
private:
    // m_randomForest
};

/*
    功能: 把原始深度图数据转换为_NUI_IMAGE_TYPE结构,
          分辨率调整为320*240,
          Set深度图Event.
    输入: _NUI_IMAGE_FRAME
    输出: 身体部位概率分布(BodyPartDistribution)
    成员: BodyPartLabeller
*/
class ExemplarStage :
    public IPipelineStage
{
public:
    ExemplarStage(void);
    ~ExemplarStage(void);

    /*
        功能: 初始化算法模块,
              加载随机森林模型数据.
        输入: szModelFileName, 随机森林模型数据文件路径.
        输出: 无
        返回: 0初始化成功, 1初始化失败.
    */
    BOOL Initialize(_IN_ const signed char *pszModelFilePath);


    BOOL ShutDown();

    /*
        功能: 运行算法模块处理一帧数据.
        输入: szModelFileName, 随机森林模型数据文件路径.
        输出: 
        返回: 0成功, 1失败.
    */
    int RunFrame(/*_IN_ const NUI_IMAGE_FRAME *pDepthFrame, _OUT_ BodyPartProbility *pBodyPartProbility*/);
private:
    BodyPartLabeller * m_pBodyPartLabeller;
};


}
