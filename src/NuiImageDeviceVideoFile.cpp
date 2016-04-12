
//#include <boost/asio/high_resolution_timer.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

#include <thread>
#include <string>
#include <QFile>
#include <QDataStream>
#include <chrono>
#include "NuiImageDeviceVideoFile.h"
#include "stdafx.h"
NuiImageDeviceVideoFile::NuiImageDeviceVideoFile(void)
{
    //m_pFuncGotDepthAlert = NULL;
    m_pTempDepthBuffer = NULL;
    m_pDepthFileBuffer = NULL;
    m_hThNuiProcessThread = NULL;
    m_hThTimerThread = NULL;
    m_hEvNuiProcessThreadStop = NULL;
    m_hEvTimerThreadStop = NULL;
    m_hNextDepthFrameEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_DepthFramesTotal = 1;
    m_TimerEventCount = 0;
    m_bufferReady = false;
}


NuiImageDeviceVideoFile::~NuiImageDeviceVideoFile(void)
{

}


int NuiImageDeviceVideoFile::InitDevice(DWORD lParam/* = 0*/, DWORD wParam/* = 0*/)
{
    // TODO: 检测文件列表
    /*
    char * pszFileList = (char *) lParam;
    if (!pszFileList)
    {
        return 0;
    }
*/
    // 申请深度图缓存内存320*240, 算法用
    m_pTempDepthBuffer = new USHORT[cDepthHeight * cDepthWidth];
    ASSERT(m_pTempDepthBuffer);
    // 申请TCP接收深度图缓存, 640*480
    m_pDepthFileBuffer = (USHORT *)(new char[DEP_IMAGE_SIZE]);
    ASSERT(m_pDepthFileBuffer);
    // NextDepthFrameEvent
    //m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );


    // Start the Nui processing thread
    //    m_hEvNuiProcessThreadStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    // m_hThNuiProcessThread = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );
    m_hThNuiProcessThread = new std::thread(&NuiImageDeviceVideoFile::Nui_ProcessThread,  (LPVOID)this);
    // Start the timer thread
    //  m_hEvTimerThreadStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    // m_hThTimerThread = CreateThread( NULL, 0, TimerThread, this, 0, NULL );
   // m_hThTimerThread = new std::thread(&NuiImageDeviceVideoFile::TimerThread,  (LPVOID)this);


    return 0;
}


void NuiImageDeviceVideoFile::UnInit()
{

    m_continueProcessing = false;
    m_getdepthCondVar.notify_all();
    if (((std::thread *)m_hThNuiProcessThread)->joinable())
    {
        ((std::thread *)m_hThNuiProcessThread)->join();
    }

//    if (((std::thread *)m_hThTimerThread)->joinable())
//    {
//        ((std::thread *)m_hThTimerThread)->join();
//    }

    delete m_hThNuiProcessThread;
 //   delete m_hThTimerThread;
    if (m_pTempDepthBuffer)
    {
        delete [] m_pTempDepthBuffer;
        m_pTempDepthBuffer = NULL;
    }
    if (m_pDepthFileBuffer)
    {
        delete [] m_pDepthFileBuffer;
        m_pDepthFileBuffer = NULL;
    }
    // Stop the timer thread
    //    if ( NULL != m_hEvTimerThreadStop && m_hEvTimerThreadStop != INVALID_HANDLE_VALUE)
    //    {
    //        // Wait for thread to stop
    //        if ( NULL != m_hThTimerThread )
    //        {
    //            // Signal the thread
    //           // SetEvent(m_hEvTimerThreadStop);

    //           // WaitForSingleObject( m_hThTimerThread, INFINITE );
    //           // CloseHandle( m_hThTimerThread );
    //        }
    //       // CloseHandle( m_hEvTimerThreadStop );
    //       // TRACE("UnInit: m_hEvTimerThreadStop\n");
    //    }

    //    // Stop the Nui processing thread
    //    if ( NULL != m_hEvNuiProcessThreadStop && m_hEvNuiProcessThreadStop != INVALID_HANDLE_VALUE)
    //    {
    //        // Wait for thread to stop
    //        if ( NULL != m_hThNuiProcessThread )
    //        {
    //            // Signal the thread
    //            SetEvent(m_hEvNuiProcessThreadStop);

    //            WaitForSingleObject( m_hThNuiProcessThread, INFINITE );
    //            CloseHandle( m_hThNuiProcessThread );
    //        }
    //        CloseHandle( m_hEvNuiProcessThreadStop );
    //        TRACE("UnInit: m_hEvNuiProcessThreadStop\n");
    //    }

    //    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    //    {
    //        CloseHandle( m_hNextDepthFrameEvent );
    //        m_hNextDepthFrameEvent = NULL;
    //        TRACE("UnInit: m_hNextDepthFrameEvent\n");
    //    }
}


int NuiImageDeviceVideoFile::SetDepthCallBack(pCallBackFunc f)
{
    ASSERT(f);
    if (f)
    {
        m_pFuncGotDepthAlert = f;
    }
    return (m_pFuncGotDepthAlert == NULL);
}


const USHORT * NuiImageDeviceVideoFile::GetDepthMap()
{
    return m_pTempDepthBuffer;
}


DWORD WINAPI NuiImageDeviceVideoFile::TimerThread(LPVOID pParam)
{
    NuiImageDeviceVideoFile *pthis = (NuiImageDeviceVideoFile *)pParam;
    return pthis->TimerThread1( );
}


DWORD WINAPI NuiImageDeviceVideoFile::TimerThread1()
{
    //    // Construct a timer without setting an expiry time.
    //    boost::asio::io_service io;

    //    // Main thread loop
    //    bool continueProcessing = true;
    //    while ( continueProcessing )
    //    {
    //        // stop event was signalled
    //        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvTimerThreadStop, 0 ) )
    //        {
    //            //SetEvent(m_hEvNuiProcessThreadStop);
    //            continueProcessing = false;
    //            TRACE("m_hEvTimerThreadStop\n");
    //            break;
    //        }

    //        boost::asio::deadline_timer timer(io, boost::posix_time::millisec(100));// 32/2
    //        //boost::asio::deadline_timer timer(io, boost::posix_time::seconds(2));
    //        timer.wait();
    //        m_TimerEventCount ++;
    //        SetEvent(m_hNextDepthFrameEvent);
    //    }
    return 0;
}


DWORD WINAPI NuiImageDeviceVideoFile::Nui_ProcessThread( LPVOID pParam )
{
    NuiImageDeviceVideoFile *pthis = (NuiImageDeviceVideoFile *)pParam;
    return pthis->Nui_ProcessThread1( );
}

DWORD WINAPI NuiImageDeviceVideoFile::Nui_ProcessThread1( )
{
//    const int numEvents = 2;
//    HANDLE hEvents[numEvents] = { m_hEvNuiProcessThreadStop, m_hNextDepthFrameEvent };
//    int    nEventIdx;

    // Main thread loop
  // bool continueProcessing = true;
   // LoadDepthImage(m_DepthFramesTotal);
    while ( m_continueProcessing )
    {
        // Wait for any of the events to be signalled
        // nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );

        //        // Timed out, continue
        //        if ( nEventIdx == WAIT_TIMEOUT )
        //        {
        //            continue;
        //        }

        //        // stop event was signalled
        //        if ( WAIT_OBJECT_0 == nEventIdx )
        //        {
        //            continueProcessing = false;
        //            TRACE("m_hEvNuiProcessThreadStop\n");
        //            break;
        //        }

        //        if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextDepthFrameEvent, 0 ) )
        //        {
        //            TRACE("m_pFuncGotDepthAlert()\n");



        {

        {

;
        }



            // get data to m_pDepthFileBuffer;
            LoadDepthImage(m_DepthFramesTotal);
            // 已经成功处理深度图
            ++m_DepthFramesTotal;

            if (!m_continueProcessing)
            {
                break;
            }
            {



            }

        }

        if (m_DepthFramesTotal > 3)
        {
            m_DepthFramesTotal = 1;
        }
        // reset event
        //ResetEvent(m_hNextDepthFrameEvent);
        //        }
    }

    return 0;
}


BOOL NuiImageDeviceVideoFile::LoadDepthImage(int a_i)
{
    m_loadState = LOADSTATE_LOADING;
    BOOL res = FALSE;
    Util ul("LoadDepthImage time include copy lock seep");
    char pszFileName[100];
    sprintf(pszFileName, ":/depth/depth/Depth_Two_%05d.depth", a_i);
    //sprintf_s(pszFileName, 100, "D:\\_SVN\TRACE_INFO2\\Depth_Two_%05d.depth", (m_TimerEventCount % 509) + 1);
    cout <<  ("loading DepthImage:%s ", pszFileName);

    FILE * fp = 0;
#ifdef WIN32
    fopen_s(&fp, pszFileName, "rb");
#else
    fp = fopen(pszFileName, "rb");
#endif

    {
        Util ul("readImage time");
        QFile file(pszFileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            //std::cout <<"resource open error!" <<std::endl;
            assert(0);
        };
        bool exist = file.exists();
        assert(exist);
        QDataStream in(&file);

        const int cbSize = in.readRawData((char*)m_pDepthFileBuffer, 320 * 240 * 2);
        assert(cbSize > 0);
        file.close();
    }
    while (m_calcState.load() == CALCSTATE_NOTINTHREAD)
    {
         {std::this_thread::sleep_for(std::chrono::milliseconds(1));}
    }
//    std::unique_lock<std::mutex> ul(m_loadImage_sync);
//    if (!m_firstLoad)
//    {
//        m_loadState = LOADSTATE_WAITING;
//        m_getdepthCondVar.wait(ul);
//    }
    // 320*240时, 无需转换
    if (!m_firstLoad)
    {
        while (m_bufferReady.load() == true && m_continueProcessing)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    else
    {
        m_firstLoad = false;
        while(m_calcState.load() != CALCSTATE_WAITING)
        {
             {std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        }
    }


    {
        std::lock_guard<std::mutex> guard(m_bufferMutex);
        m_loadState = LOADSTATE_BUFFERING;
        //Util ul("Image Guard time");
        // std::lock_guard<std::mutex> guard(m_depthMapMutex);
        memcpy(m_pTempDepthBuffer, m_pDepthFileBuffer, 320*240*2);
        m_bufferReady = true;
        m_getdepthCondVar.notify_one();
    }

    // while (!m_bufferReady) {std::this_thread::yield();}
     //assert(m_calcState == CALCSTATE_WAITING);

 //   std::this_thread::sleep_for(std::chrono::milliseconds(3));
    cout <<  ("image %s loaded",pszFileName);
    return TRUE;
}
