#include "StdAfx.h"
#include "../resource.h"
#include "NuiImageDeviceKinect.h"

#ifdef USE_DEVICE_KINECT

NuiImageDeviceKinect::NuiImageDeviceKinect()
{
    m_pNuiSensor = NULL;
    m_pFuncGotDepthAlert = NULL;
    m_pTempDepthBuffer = NULL;
    m_instanceId = NULL;
    m_hThNuiProcess = NULL;
    m_hEvNuiProcessStop = NULL;
    m_hNextDepthFrameEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_DepthFramesTotal = 0;
}


NuiImageDeviceKinect::~NuiImageDeviceKinect()
{
    if (m_pTempDepthBuffer)
    {
        delete [] m_pTempDepthBuffer;
        m_pTempDepthBuffer = NULL;
    }
}


int NuiImageDeviceKinect::InitDevice(DWORD lParam/* = 0*/, DWORD wParam/* = 0*/)
{
    HRESULT  hr;

    if ( !m_pNuiSensor )
    {
        hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

        if ( FAILED(hr) )
        {
            return hr;
        }

        SysFreeString(m_instanceId);

        m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
    }

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;

    hr = m_pNuiSensor->NuiInitialize(nuiFlags);
    if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
    {
        nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX; //NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
        hr = m_pNuiSensor->NuiInitialize( nuiFlags);
    }

    if ( FAILED( hr ) )
    {
        // IDS_ERROR_IN_USE, IDS_ERROR_NUIINIT
        return hr;
    }

    if ( !HasSkeletalEngine( m_pNuiSensor ) )
    {
        // TODO: 不能提供BGR, 可用V1.0替代
        // IDS_ERROR_SKELETONTRACKING
        return 1;
    }
    DWORD m_DepthStreamFlags = 0;
    hr = m_pNuiSensor->NuiImageStreamOpen(
        HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_320x240, //NUI_IMAGE_RESOLUTION_640x480
        m_DepthStreamFlags,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );

    if ( FAILED( hr ) )
    {
        // IDS_ERROR_DEPTHSTREAM
        return hr;
    }

    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

    // 申请深度图缓存内存
    m_pTempDepthBuffer = new USHORT[cDepthHeight * cDepthWidth];
    ASSERT(m_pTempDepthBuffer);

    return 0;
}


void NuiImageDeviceKinect::UnInit()
{
    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop && m_hEvNuiProcessStop != INVALID_HANDLE_VALUE)
    {
        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            // Signal the thread
            SetEvent(m_hEvNuiProcessStop);

            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
        TRACE("UnInit: m_hEvNuiProcessStop\n");
    }

    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
        TRACE("UnInit: m_hNextDepthFrameEvent\n");
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->NuiShutdown( );
    }
    TRACE("UnInit: m_pNuiSensor->NuiShutdown()\n");

    SafeRelease( m_pNuiSensor );
}


int NuiImageDeviceKinect::SetDepthCallBack(pCallBackFunc f)
{
    ASSERT(f);
    if (f)
    {
        m_pFuncGotDepthAlert = f;
    }
    return (m_pFuncGotDepthAlert == NULL);
}


const USHORT * NuiImageDeviceKinect::GetDepthMap()
{
    return m_pTempDepthBuffer;
}


DWORD WINAPI NuiImageDeviceKinect::Nui_ProcessThread( LPVOID pParam )
{
    NuiImageDeviceKinect *pthis = (NuiImageDeviceKinect *)pParam;
    return pthis->Nui_ProcessThread( );
}

/// <summary>
/// Thread to handle Kinect processing
/// </summary>
/// <returns>always 0</returns>
DWORD WINAPI NuiImageDeviceKinect::Nui_ProcessThread( )
{
    const int numEvents = 2;
    HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, m_hNextDepthFrameEvent };
    int    nEventIdx;

    // Main thread loop
    bool continueProcessing = true;
    while ( continueProcessing )
    {
        // Wait for any of the events to be signalled
        nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );

        // Timed out, continue
        if ( nEventIdx == WAIT_TIMEOUT )
        {
            continue;
        }

        // stop event was signalled 
        if ( WAIT_OBJECT_0 == nEventIdx )
        {
            continueProcessing = false;
            break;
        }

        // Wait for each object individually with a 0 timeout to make sure to
        // process all signalled objects if multiple objects were signalled
        // this loop iteration

        // In situations where perfect correspondance between color/depth/skeleton
        // is essential, a priority queue should be used to service the item
        // which has been updated the longest ago

        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextDepthFrameEvent, 0 ) )
        {
            HRESULT hr;
            NUI_IMAGE_FRAME imageFrame;

            // Attempt to get the color frame
            hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
            if (FAILED(hr))
            {
                return NULL;
            }

            INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
            NUI_LOCKED_RECT LockedRect;

            // Lock the frame data so the Kinect knows not to modify it while we're reading it
            pTexture->LockRect(0, &LockedRect, NULL, 0);

            // Make sure we've received valid data
            if (LockedRect.Pitch != 0 && cDepthWidth * cDepthHeight * 2 == LockedRect.size)
            {
                if (m_pTempDepthBuffer == NULL)
                {
                    m_pTempDepthBuffer = new USHORT[cDepthWidth * cDepthHeight];
                }

                // copy ...........
                for (int i = 0; i < cDepthWidth * cDepthHeight; ++i)
                {
                    USHORT intensity = reinterpret_cast<USHORT*>(LockedRect.pBits)[i]; // >> 8;
                    //intensity >>= 3;

                    m_pTempDepthBuffer[i] = intensity;
                }
            }

            // We're done with the texture so unlock it
            pTexture->UnlockRect(0);

            // Release the frame
            m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

            //only increment frame count if a frame was successfully drawn
            if ( m_pFuncGotDepthAlert() )
            {
                // 已经成功处理深度图
                ++m_DepthFramesTotal;
            }
            ResetEvent(m_hNextDepthFrameEvent);
        }
    }

    return 0;
}
#endif USE_DEVICE_KINECT