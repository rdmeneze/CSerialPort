
// $Id: Serial.cpp,v 1.1 2009/12/14 19:57:28 Antonio Exp $

#include "stdafx.h"
#include "Serial.h"
//#include "st_iocp.h"
#include "Win32Error.h"

using namespace network;



//-----------------------------------------------------------------------------------------------------------------------------------------------------

CSerial::CSerial()
{
    hPort = NULL;
    bQuit = FALSE;

    //iIOCP = IOCP::Create();
    //if (iIOCP == -1)
    //{
    //    CWin32Error e;
    //    throw e.ErrorCode();
    //}

    process = NULL;

    hListenerThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CSerial::ThreadStartSerialPortListener, this, 0, NULL);
    if (hListenerThread == NULL)
    {
        CWin32Error e;
        throw e.ErrorCode();
    }

    return;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

CSerial::~CSerial( ) 
{
    try
    {
        Close();
        bQuit = TRUE;
    }
    catch (...)
    {
        return;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

int CSerial::Open(const char *device)
{
    hPort = CreateFile((LPCTSTR)device,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if (hPort == INVALID_HANDLE_VALUE)
    {
        CWin32Error e;
        hPort = NULL;
        return e.ErrorCode();
    }

    _snprintf(cDevice, sizeof(cDevice) - 1, device);

    // Seta os valores default para a porta
    SetBaudRate(9600);
    SetStopBits(1);
    SetByteSize(8);
    SetParity(NOPARITY);
    SetHandshaking(HAND_SHAKE_OFF);
    SetTimeouts();

    return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

BYTE CSerial::IsOpen( void )
{
    return (hPort != NULL);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

void CSerial::Close()
{
    if (hPort != NULL)
    {
        CloseHandle(hPort);
        hPort = NULL;
    }

    cDevice[0] = '\0';

    return;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::SetHandshaking(EnumSerialHandshake SerialHandshake)
{
    // Obtem os parametros default da porta serial aberta
    SecureZeroMemory(&dcb, sizeof(DCB));

    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    switch (SerialHandshake)
    {
            case HAND_SHAKE_OFF:
            {
                dcb.fOutxCtsFlow = false;									// Disable CTS monitoring
                dcb.fOutxDsrFlow = false;									// Disable DSR monitoring
                dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR monitoring
                dcb.fOutX = false;									// Disable XON/XOFF for transmission
                dcb.fInX = false;									// Disable XON/XOFF for receiving
                dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
            }
            break;

            case HAND_SHAKE_HARDWARE:
            {
                dcb.fOutxCtsFlow = true;										// Enable CTS monitoring
                dcb.fOutxDsrFlow = true;										// Enable DSR monitoring
                dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;	// Enable DTR handshaking
                dcb.fOutX = false;									// Disable XON/XOFF for transmission
                dcb.fInX = false;									// Disable XON/XOFF for receiving
                dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;	// Enable RTS handshaking
            }
            break;

            case HAND_SHAKE_SOFTWARE:
            {
                dcb.fOutxCtsFlow = false;									// Disable CTS (Clear To Send)
                dcb.fOutxDsrFlow = false;									// Disable DSR (Data Set Ready)
                dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR (Data Terminal Ready)
                dcb.fOutX = true;										// Enable XON/XOFF for transmission
                dcb.fInX = true;										// Enable XON/XOFF for receiving
                dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
            }
            break;
    }

    if (!SetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::SetBaudRate(int baud_rate)
{
    // Obtem os parametros default da porta serial aberta
    SecureZeroMemory(&dcb, sizeof(DCB));

    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hPort, &dcb))
        return GetLastError();

    switch (baud_rate)
    {
        case CBR_110:
        case CBR_300:
        case CBR_600:
        case CBR_1200:
        case CBR_2400:
        case CBR_4800:
        case CBR_9600:
        case CBR_14400:
        case CBR_19200:
        case CBR_38400:
        case CBR_56000:
        case CBR_57600:
        case CBR_115200:
        case CBR_128000:
        case CBR_256000:
            dcb.BaudRate = DWORD(baud_rate);
            break;

        default:
            return ERROR_BAD_COMMAND;
    }

    if (!SetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::SetStopBits(int stop_bits)
{
    // Obtem os parametros default da porta serial aberta
    SecureZeroMemory(&dcb, sizeof(DCB));

    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hPort, &dcb))
        return ::GetLastError();

    switch (stop_bits)
    {
        case ONESTOPBIT:		// 1 stopbit (default)
        case ONE5STOPBITS:	// 1.5 stopbit
        case TWOSTOPBITS:		// 2 stopbits
            dcb.StopBits = BYTE(stop_bits);
            break;

        default:
            return ERROR_BAD_COMMAND;
    }

    if (!SetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::SetParity(int parity)
{
    // Obtem os parametros default da porta serial aberta
    SecureZeroMemory(&dcb, sizeof(DCB));

    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    switch (parity)
    {
        case NOPARITY:		// No parity (default)
        case ODDPARITY:		// Odd parity
        case EVENPARITY:	// Even parity
        case MARKPARITY:	// Mark parity
        case SPACEPARITY:	// Space parity
            dcb.Parity = BYTE(parity);
            break;

        default:
            return ERROR_BAD_COMMAND;
    }

    if (!SetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::SetByteSize(int byte_size)
{
    // Obtem os parametros default da porta serial aberta
    SecureZeroMemory(&dcb, sizeof(DCB));

    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    switch (byte_size)
    {
        case 5:	// 5 bits per byte
        case 6:	// 6 bits per byte
        case 7:	// 7 bits per byte
        case 8:	// 8 bits per byte (default)
            dcb.ByteSize = BYTE(byte_size);
            break;

        default:
            return ERROR_BAD_COMMAND;
    }

    if (!SetCommState(hPort, &dcb))
    {
        CWin32Error e;
        return e.ErrorCode();
    }

    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

void CSerial::SetTimeouts()
{
    COMMTIMEOUTS cto;

    cto.ReadIntervalTimeout = MAXWORD;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant = 1;
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant = 0;

    SetCommTimeouts(hPort, &cto);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

//int CSerial::Read(char *s, int len)
//{
//	DWORD read = 0;
//
//	if( !ReadFile(hPort, (char *)s, len, &read, 0) )
//	{
//		return 0;
//	}
//	
//	return read;
//}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

int CSerial::Write(char *s, int len)
{
    DWORD wrote = 0;

    if (!WriteFile(hPort, (char *)s, len, &wrote, 0))
    {
        return 0;
    }

    return wrote;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

int CSerial::Write(char *s, int len, int delay)
{
    DWORD wrote = 0;
    char temp[2];
    int i;

    for (i = 0; i < len; i++)
    {
        Sleep(delay);

        temp[0] = s[i];
        temp[1] = 0;

        if (!WriteFile(hPort, (char *)temp, 1, &wrote, 0))
        {
            return 0;
        }
    }

    return len;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::RegisterListenner( SERIAL_PORT_CALLBACK func_process )
{
    process = func_process;
    if (process == NULL)
    {
        return (-1);
    }
    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD CSerial::SerialPortListener( void )
{
  DWORD rxEvnt;
  DWORD dwRet;
  DWORD dwBytesRead;
  BYTE * pBuffer;
  const int bufferLen = (1024);
  //struct SERIAL_DATA * serialData;

  ov.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
  ov.Internal = 0;
  ov.InternalHigh = 0;
  ov.Offset = 0;
  ov.OffsetHigh = 0;
  
  if ( ov.hEvent == NULL )
  {
    CWin32Error e;
    return e.ErrorCode();
  }

  // configura o evento a ser recebido
  if ( SetCommMask( hPort, (DWORD)EV_RXCHAR ) )
  {
    CWin32Error e;
    return e.ErrorCode();
  }

  do
  {
    if ( bQuit == TRUE )
    {
      break;
    }

    if ( WaitCommEvent( hPort, &rxEvnt, &ov ) == 0 )
    {
      dwRet = GetLastError();
      if( dwRet == ERROR_IO_PENDING)
      {
        continue;
      }
    }

    if ( rxEvnt & EV_RXCHAR )
    {
      pBuffer = new BYTE [ bufferLen + 1 ];
      if ( pBuffer == NULL )
      {
        continue;
      }
      
      dwRet = ReadFile( hPort, (LPVOID)pBuffer, bufferLen, &dwBytesRead, &ov );

      if ( !dwRet )
      {
        dwRet = ::GetLastError();
        delete pBuffer;
        continue;
      }

      //serialData = new SERIAL_DATA;
      //serialData->dwLen = dwBytesRead;
      //serialData->pData = new BYTE[ serialData->dwLen ];
      //memcpy( (BYTE*)serialData->pData, (BYTE*)pBuffer, serialData->dwLen );

      if (process != NULL)
      {
          process(pBuffer, dwBytesRead);
      }

      //IOCP::Send(iIOCP, serialData);

      ///PostQueuedCompletionStatus( hCompletionEvnt, sizeof( SERIAL_DATA ), (ULONG_PTR)serialData, NULL );

      delete pBuffer;
    }

  }while( 1 );

  return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

DWORD WINAPI CSerial::ThreadStartSerialPortListener( LPVOID lpParam )
{
  CSerial * serial;
  
  serial = (CSerial*)lpParam;

  serial->SerialPortListener( );

  return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------

