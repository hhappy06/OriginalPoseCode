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
    ����: ��ԭʼ���ͼ����ת��Ϊ_NUI_IMAGE_TYPE�ṹ,
          �ֱ��ʵ���Ϊ320*240,
          Set���ͼEvent.
    ����: _NUI_IMAGE_FRAME
    ���: ���岿λ���ʷֲ�(BodyPartDistribution)
    ��Ա: BodyPartLabeller
*/
class ExemplarStage :
    public IPipelineStage
{
public:
    ExemplarStage(void);
    ~ExemplarStage(void);

    /*
        ����: ��ʼ���㷨ģ��,
              �������ɭ��ģ������.
        ����: szModelFileName, ���ɭ��ģ�������ļ�·��.
        ���: ��
        ����: 0��ʼ���ɹ�, 1��ʼ��ʧ��.
    */
    BOOL Initialize(_IN_ const signed char *pszModelFilePath);


    BOOL ShutDown();

    /*
        ����: �����㷨ģ�鴦��һ֡����.
        ����: szModelFileName, ���ɭ��ģ�������ļ�·��.
        ���: 
        ����: 0�ɹ�, 1ʧ��.
    */
    int RunFrame(/*_IN_ const NUI_IMAGE_FRAME *pDepthFrame, _OUT_ BodyPartProbility *pBodyPartProbility*/);
private:
    BodyPartLabeller * m_pBodyPartLabeller;
};


}
