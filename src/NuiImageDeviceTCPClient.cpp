#include "StdAfx.h"
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "NuiImageDeviceTCPClient.h"


NuiImageDeviceTCPClient::NuiImageDeviceTCPClient(void)
{
    m_pFuncGotDepthAlert = NULL;
    m_pTempDepthBuffer = NULL;
    m_pTCPReceiveBuffer = NULL;
    m_hThNuiProcessThread = NULL;
    m_hThTimerThread = NULL;
    m_hThTCPTransferThread = NULL;
    m_hEvNuiProcessThreadStop = NULL;
    m_hEvTimerThreadStop = NULL;
    m_hEvTCPTransferThreadStop = NULL;
    m_hNextDepthFrameEvent = NULL;
    m_hNextTCPTransferEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_DepthFramesTotal = 0;
    m_TimerEventCount = 0;
    m_bTCPTransferSuccess = FALSE;
}


NuiImageDeviceTCPClient::~NuiImageDeviceTCPClient(void)
{
    if (m_pTempDepthBuffer)
    {
        delete [] m_pTempDepthBuffer;
        m_pTempDepthBuffer = NULL;
    }
    if (m_pTCPReceiveBuffer)
    {
        delete [] m_pTCPReceiveBuffer;
        m_pTCPReceiveBuffer = NULL;
    }
}


int NuiImageDeviceTCPClient::InitDevice(DWORD lParam/* = 0*/, DWORD wParam/* = 0*/)
{
    DWORD dwIPAddr = (lParam != 0) ? lParam : cIPLocalHost;
    DWORD dwPort   = (wParam != 0) ? wParam : cDefaultPort;

    // TODO: check dwIPAddr, dwPort
    if (dwPort > 60000)
    {
        return dwPort;
    }
    m_dwIPAddr = dwIPAddr;
    m_dwPort   = dwPort;

    // 测试m_dwIPAddr格式是否正确
    //boost::asio::ip::address_v4 a(m_dwIPAddr);
    //std::string ipaddr = a.to_string();
    m_endpoint.address(boost::asio::ip::address_v4(m_dwIPAddr));
    m_endpoint.port((WORD)m_dwPort);

    // NextDepthFrameEvent
    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    
    // NextTCPTransferEvent
    m_hNextTCPTransferEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    // Start the Nui processing thread
    m_hEvNuiProcessThreadStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcessThread = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );
    
    // Start the timer thread
    m_hEvTimerThreadStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThTimerThread = CreateThread( NULL, 0, TimerThread, this, 0, NULL );
    
    // Start the TCP transfer thread
    m_hEvTCPTransferThreadStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThTCPTransferThread = CreateThread( NULL, 0, TCPTransferThread, this, 0, NULL );

    // 申请深度图缓存内存320*240, 算法用
    m_pTempDepthBuffer = new USHORT[cDepthHeight * cDepthWidth];
    ASSERT(m_pTempDepthBuffer);
    // 申请TCP接收深度图缓存, 640*480
    m_pTCPReceiveBuffer = (USHORT *)(new char[DEP_IMAGE_SIZE]);
    ASSERT(m_pTCPReceiveBuffer);

    return 0;
}


void NuiImageDeviceTCPClient::UnInit()
{
    // Stop the TCP Transfer thread
    if ( NULL != m_hEvTCPTransferThreadStop && m_hEvTCPTransferThreadStop != INVALID_HANDLE_VALUE)
    {
        // Wait for thread to stop
        if ( NULL != m_hThTCPTransferThread )
        {
            // Signal the thread
            SetEvent(m_hEvTCPTransferThreadStop);

            WaitForSingleObject( m_hThTCPTransferThread, INFINITE );
            CloseHandle( m_hThTCPTransferThread );
        }
        CloseHandle( m_hEvTCPTransferThreadStop );
        TRACE("UnInit: m_hEvTCPTransferThreadStop\n");
    }

    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessThreadStop && m_hEvNuiProcessThreadStop != INVALID_HANDLE_VALUE)
    {
        // Wait for thread to stop
        if ( NULL != m_hThNuiProcessThread )
        {
            // Signal the thread
            SetEvent(m_hEvNuiProcessThreadStop);

            WaitForSingleObject( m_hThNuiProcessThread, INFINITE );
            CloseHandle( m_hThNuiProcessThread );
        }
        CloseHandle( m_hEvNuiProcessThreadStop );
        TRACE("UnInit: m_hEvNuiProcessThreadStop\n");
    }

    // Stop the timer thread
    if ( NULL != m_hEvTimerThreadStop && m_hEvTimerThreadStop != INVALID_HANDLE_VALUE)
    {
        // Wait for thread to stop
        if ( NULL != m_hThTimerThread )
        {
            // Signal the thread
            SetEvent(m_hEvTimerThreadStop);

            WaitForSingleObject( m_hThTimerThread, INFINITE );
            CloseHandle( m_hThTimerThread );
        }
        CloseHandle( m_hEvTimerThreadStop );
        TRACE("UnInit: m_hEvTimerThreadStop\n");
    }

    if ( m_hNextTCPTransferEvent && ( m_hNextTCPTransferEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextTCPTransferEvent );
        m_hNextTCPTransferEvent = NULL;
        TRACE("UnInit: m_hNextTCPTransferEvent\n");
    }
    
    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
        TRACE("UnInit: m_hNextDepthFrameEvent\n");
    }
}


int NuiImageDeviceTCPClient::SetDepthCallBack(pCallBackFunc f)
{
    ASSERT(f);
    if (f)
    {
        m_pFuncGotDepthAlert = f;
    }
    return (m_pFuncGotDepthAlert == NULL);
}


const USHORT * NuiImageDeviceTCPClient::GetDepthMap()
{
    return m_pTempDepthBuffer;
}


DWORD WINAPI NuiImageDeviceTCPClient::TCPTransferThread(LPVOID pParam)
{
    NuiImageDeviceTCPClient *pthis = (NuiImageDeviceTCPClient *)pParam;
    return pthis->TCPTransferThread( );
}


DWORD WINAPI NuiImageDeviceTCPClient::TCPTransferThread()
{
    const int numEvents = 2;
    HANDLE hEvents[numEvents] = { m_hEvTCPTransferThreadStop, m_hNextTCPTransferEvent };
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
            TRACE("m_hEvTCPTransferThreadStop\n");
            break;
        }

        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextTCPTransferEvent, 0 ) )
        {
            // get data to m_pTCPReceiveBuffer;
            if (TCPGetDepthImage())
            {
                // 640X480 to 320X240
                //memset(m_pTempDepthBuffer, 0, cDepthHeight * cDepthWidth * sizeof(USHORT));
                for (int y = 0; y < cDepthHeight; y ++)
                {
                    for (int x = 0; x < cDepthWidth; x ++)
                    {
                        m_pTempDepthBuffer[y * cDepthWidth + x] = m_pTCPReceiveBuffer[2*y * (2*cDepthWidth) + 2*x];
                    }
                }
                m_bTCPTransferSuccess = TRUE;
            }
            else
            {
                TRACE("TCPGetDepthImage() == FALSE\n");
                m_bTCPTransferSuccess = FALSE;
            }
            // reset event
            ResetEvent(m_hNextTCPTransferEvent);
        }
    }
    return 0;
}


DWORD WINAPI NuiImageDeviceTCPClient::TimerThread(LPVOID pParam)
{
    NuiImageDeviceTCPClient *pthis = (NuiImageDeviceTCPClient *)pParam;
    return pthis->TimerThread( );
}


DWORD WINAPI NuiImageDeviceTCPClient::TimerThread()
{
    // Construct a timer without setting an expiry time.
    boost::asio::io_service io;

    // Main thread loop
    bool continueProcessing = true;
    while ( continueProcessing )
    {
        // stop event was signalled 
        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvTimerThreadStop, 0 ) )
        {
            //SetEvent(m_hEvNuiProcessThreadStop);
            continueProcessing = false;
            TRACE("m_hEvTimerThreadStop\n");
            break;
        }

        boost::asio::deadline_timer timer(io, boost::posix_time::millisec(16));// 32/2
        //boost::asio::deadline_timer timer(io, boost::posix_time::seconds(2));
        timer.wait();
        if (m_TimerEventCount++ % 2)
        {
            SetEvent(m_hNextDepthFrameEvent);
            //TRACE("SetEvent(m_hNextDepthFrameEvent)\n");
        }
        else
        {
            SetEvent(m_hNextTCPTransferEvent);
        }
    }
    return 0;
}



DWORD WINAPI NuiImageDeviceTCPClient::Nui_ProcessThread( LPVOID pParam )
{
    NuiImageDeviceTCPClient *pthis = (NuiImageDeviceTCPClient *)pParam;
    return pthis->Nui_ProcessThread( );
}

DWORD WINAPI NuiImageDeviceTCPClient::Nui_ProcessThread( )
{
    const int numEvents = 2;
    HANDLE hEvents[numEvents] = { m_hEvNuiProcessThreadStop, m_hNextDepthFrameEvent };
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
            TRACE("m_hEvNuiProcessThreadStop\n");
            break;
        }

        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextDepthFrameEvent, 0 ) )
        {
            TRACE("m_pFuncGotDepthAlert()\n");
            
            // get data to m_pTCPReceiveBuffer;
            if (m_bTCPTransferSuccess == TRUE)
            {
                if (m_bTCPTransferSuccess != FALSE && m_pFuncGotDepthAlert())
                {
                    // 已经成功处理深度图
                    ++m_DepthFramesTotal;
                }
            }
            // reset event
            ResetEvent(m_hNextDepthFrameEvent);
        }
    }

    return 0;
}


BOOL NuiImageDeviceTCPClient::TCPGetDepthImage()
{
    BOOL res = FALSE;

    size_t frame_pos = 0;
    try
    {
        //boost::asio::io_service io_service;
        //tcp::endpoint endpoint(boost::asio::ip::address::from_string(m_ip_addr), m_port); //"172.16.88.163"
        boost::asio::ip::tcp::socket socket(m_io_service);
        socket.connect(m_endpoint);

        // send request
        boost::system::error_code ignored_error;
        boost::asio::write(socket, boost::asio::buffer("DEP_IMAGE"), ignored_error);

        frame_pos = 0;
        // receive one deopth frame
        for (;;)
        {
            boost::system::error_code error;
            size_t len = boost::asio::read(socket, boost::asio::buffer(m_pTCPReceiveBuffer, DEP_IMAGE_SIZE), error);
            if (error == boost::asio::error::eof)
            {
                break; // Connection closed cleanly by peer. frame end
            }
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            frame_pos += len;
        }
    }
    catch (std::exception& e)
    {
        //std::cerr << e.what() << std::endl;
        TRACE("TCPGetDepthImage: %s\n", e.what());
        return res;
    }

    if (frame_pos == DEP_IMAGE_SIZE)
    {
        //TRACE("DEP[100] = {%d}\n", ((unsigned short *)aimage_buff)[100]);
        res = TRUE;
    }

    return res;
}
