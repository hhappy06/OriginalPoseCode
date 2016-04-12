//NuiFrameTextureImpl.cpp
#include "stdafx.h"
#include "NuiSinect.h"
using namespace NuiSinect;

typedef unsigned char uchar;
typedef unsigned long ulong;


class NuiFrameTextureImpl
    : public INuiFrameTexture
{
public:
    ~NuiFrameTextureImpl();

    static long Create(INuiFrameTexture * *);

    static long Create(int,int,int,uchar *,bool,ulong,INuiFrameTexture * *);

    long CreateBuffer(int,int,int);

    static long CreateBuffer(int,int,int,INuiFrameTexture * *);

public:
    virtual int BufferLen();

    virtual int Pitch();

    virtual HRESULT LockRect(UINT Level, /* [ref] */ NUI_LOCKED_RECT *pLockedRect, /* [unique] */ RECT *pRect, DWORD Flags);

    virtual HRESULT UnlockRect( /* [in] */ UINT Level);
protected:
    NuiFrameTextureImpl();
private:
    BOOL    m_bBufferInit;
    int     m_Width;
    int     m_Height;
    int     m_BytesPerPixel;
    UCHAR * m_pBuffer;
    ULONG   m_TimeStamp;
    ULONG   m_ulCriticalSection;
/*
    INuiFrameTextureµÄ½á¹¹²Â²â:
    [esi+00h] = addr(const NuiFrameTexture::`vftable'{for `SimpleComObjectBase<NuiFrameTexture>'});
    [esi+04h] = addr(const NuiFrameTexture::`vftable'{for `INuiFrameTexture'});
    [esi+08h] = int refCount;
    [esi+0Ch] = addr(const NuiFrameTexture::`vftable'{for `NativeComObject<NuiFrameTexture,INuiFrameTexture>'});
    [esi+10h] = addr(const NuiFrameTexture::`vftable'{for `IDirect3DTexture9'});
    [esi+14h] = bool(short) bInit; // (0)1;
    [esi+18h] = int width;         // (0)320;
    [esi+1Ch] = int height;        // (0)240;
    [esi+20h] = int BytesPerPixel; // (0)2;
    [esi+24h] = UCHAR * pBuffer;   // (0)pBuffer;
    [esi+28h] = ULONG timeStamp;   // (0)0;
    [esi+2Ch] = ULONG ulCriticalSection; // (0)0;
*/
};

NuiFrameTextureImpl::NuiFrameTextureImpl()
{
    m_bBufferInit = FALSE;
    m_Width = 0;
    m_Height = 0;
    m_BytesPerPixel = 0;
    m_pBuffer = NULL;
    m_TimeStamp = 0;
    m_ulCriticalSection = 0;
}


NuiFrameTextureImpl::~NuiFrameTextureImpl()
{
    if (m_pBuffer)
    {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }
    m_bBufferInit = FALSE;
}


long NuiFrameTextureImpl::Create(INuiFrameTexture ** ppFrame)
{
    return NuiFrameTextureImpl::Create(0, 0, 0, NULL, FALSE, 0, ppFrame);
}


long NuiFrameTextureImpl::Create(int width, int height, int bytesPerPixel, uchar * pBuffer, bool bBufferInit, ulong timeStamp, INuiFrameTexture ** ppFrame)
{
    //width, height, 4, pBuffer, &pInputFrameTexture, timeStamp
    if (ppFrame == NULL)
    {
        return 1;
    }

    NuiFrameTextureImpl * frameTexture = new NuiFrameTextureImpl();
    if (frameTexture)
    {
        frameTexture->m_Width = width;
        frameTexture->m_Height = height;
        frameTexture->m_BytesPerPixel = bytesPerPixel;
        frameTexture->m_pBuffer = pBuffer;
        frameTexture->m_bBufferInit = bBufferInit;
        frameTexture->m_TimeStamp = timeStamp;
        *ppFrame = static_cast<INuiFrameTexture*>(frameTexture);
    }

    return 0;
}


long NuiFrameTextureImpl::CreateBuffer(int width, int height, int bytesPerPixel)
{
    int bufferSize = width * height * bytesPerPixel;
    UCHAR * pBuffer = new UCHAR [bufferSize];
    if (pBuffer)
    {
        m_pBuffer = pBuffer;
        m_Width = width;
        m_Height = height;
        m_BytesPerPixel = bytesPerPixel;
        m_bBufferInit = TRUE;
        m_TimeStamp = 0;
        return 0;
    }
    return 1;
}


long NuiFrameTextureImpl::CreateBuffer(int width, int height, int bytesPerPixel, INuiFrameTexture ** ppFrame)
{
    if (ppFrame == NULL)
    {
        return 1;
    }
    NuiFrameTextureImpl * frameTexture = new NuiFrameTextureImpl();
    if (frameTexture)
    {
        if (!frameTexture->CreateBuffer(width, height, bytesPerPixel))
        {
            *ppFrame = static_cast<INuiFrameTexture*>(frameTexture);
            return 0;
        }
        else
        {
            return 1;
        }
    }
    return 1;
}


int NuiFrameTextureImpl::BufferLen()
{
    return (m_BytesPerPixel * m_Width * m_Height);
}


int NuiFrameTextureImpl::Pitch()
{
    return (m_BytesPerPixel * m_Width);
}


HRESULT NuiFrameTextureImpl::LockRect(UINT Level, /* [ref] */ NUI_LOCKED_RECT *pLockedRect, /* [unique] */ RECT *pRect, DWORD Flags)
{
    if (pLockedRect == NULL || Level != 0 || m_pBuffer)
    {
        return S_FALSE;
    }

    pLockedRect->pBits = m_pBuffer;
    pLockedRect->Pitch = Pitch();
    pLockedRect->size  = BufferLen();

    return S_OK;
}


HRESULT NuiFrameTextureImpl::UnlockRect( /* [in] */ UINT Level)
{
    if (Level != 0)
    {
        return S_FALSE;
    }
    return S_OK;
}
