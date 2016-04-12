#ifndef _NUIIMAGEDEVICETCPCLIENT_H
#define _NUIIMAGEDEVICETCPCLIENT_H
#include "StdAfx.h"
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "INuiImageDevice.h"

class NuiImageDeviceTCPClient
    : public INuiImageDevice
{
    static const int   cDepthWidth  = 320;
    static const int   cDepthHeight = 240;
    static const DWORD cIPLocalHost = 0x7F000001; // 127.0.0.1
    static const DWORD cDefaultPort = 808; //12001;
public:
    NuiImageDeviceTCPClient();
    virtual ~NuiImageDeviceTCPClient();

    int InitDevice(DWORD lParam = 0, DWORD wParam = 0);
    void UnInit();
    int SetDepthCallBack(pCallBackFunc);
    const USHORT * GetDepthMap();
private:
    BOOL TCPGetDepthImage();

    // TimerThread
    static DWORD WINAPI     TimerThread(LPVOID pParam);
    DWORD WINAPI            TimerThread();

    // TCPTransferThread
    static DWORD WINAPI     TCPTransferThread(LPVOID pParam);
    DWORD WINAPI            TCPTransferThread();

    // Nui_ProcessThread
    static DWORD WINAPI     Nui_ProcessThread( LPVOID pParam );
    DWORD WINAPI            Nui_ProcessThread( );

    // IP:PORT
    DWORD m_dwIPAddr;
    DWORD m_dwPort;
    // io_service, tcp
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::endpoint m_endpoint;

    // thread handling
    HANDLE        m_hThTimerThread;
    HANDLE        m_hEvTimerThreadStop;
    HANDLE        m_hThTCPTransferThread;
    HANDLE        m_hEvTCPTransferThreadStop;
    HANDLE        m_hThNuiProcessThread;
    HANDLE        m_hEvNuiProcessThreadStop;

    HANDLE        m_hNextTCPTransferEvent;
    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_pDepthStreamHandle;

    USHORT *      m_pTempDepthBuffer;
    USHORT *      m_pTCPReceiveBuffer;
    int           m_DepthFramesTotal;
    int           m_TimerEventCount;
    volatile BOOL m_bTCPTransferSuccess;

    pCallBackFunc m_pFuncGotDepthAlert;
};

#endif _NUIIMAGEDEVICETCPCLIENT_H