
// $Id: Serial.h,v 1.1 2009/12/14 19:57:28 rafael Dias Exp $

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <windows.h>

namespace network {

  //! Dado recebido pela porta serial
  //struct SERIAL_DATA
  //{
  //  BYTE *  pData;
  //  DWORD   dwLen;
  //};

  typedef void(*SERIAL_PORT_CALLBACK)( BYTE*, DWORD );

  // Enum para controle do Handshake
  enum EnumSerialHandshake
  {
	  HAND_SHAKE_OFF			=	0,			// NO HANDSHAKING
	  HAND_SHAKE_HARDWARE	=	1,			// HARDWARE HANDSHAKING (RTS/CTS)
	  HAND_SHAKE_SOFTWARE	=	2,			// SOFTWARE HANDSHAKING (XON/XOFF)
  };

    class CSerial
    {
    private:
      
        volatile BOOL bQuit;

        /**
         * \brief Handle para a porta serial
         */
        HANDLE hPort;

        //! handle to control the read event
        HANDLE hListenerThread;
        //! iocompletion port handle
        //int iIOCP;

        //! name of device
        char   cDevice[128];

        //! structure to configure every all parameters of the serial port
        DCB dcb;

        //! configure tge default value to write and read timeout
        void SetTimeouts();
      
        //! overlapped structure for use with the 
        OVERLAPPED ov;
      
        //! pointer to function of type SERIAL_PORT_CALLBACK that is 
        //!   perform the processing of the  data received by the serial port
        SERIAL_PORT_CALLBACK  process;

        DWORD SerialPortListener( void );
        static DWORD WINAPI ThreadStartSerialPortListener( LPVOID lpParam );

    public:
        /**
         *  \brief Constructor
         */
        CSerial() throw( ... );

        /**
         *  \brief Destructor
         */
        virtual ~CSerial( );

        /**
         *  \brief Opens a serial device
         */
            int Open(const char *device);

        /**
         *  \brief Inform that the serial port is open
         */
        BYTE IsOpen( void );

        /**
         *  \brief Close the serial comunications
         */
        void Close();

        /**
         *  \brief  Configure the baudrate of the serial link
         *  \param  baud_rate link baudrate
         *  \return ok or error
         */
        DWORD SetBaudRate(int baud_rate);

        /**
         *  \brief configures the stop bit
         *  \param stop_bits Number of stop bits in the data link
         *  \return status of operation
         */
        DWORD SetStopBits(int stop_bits);

        /**
         *  \brief Configura as opções de paridade para o canal de comunicação
         *  \param parity Paridade do canal de comunicação. Olhe em Winbase.h.
         *  \return Código de Erro. 
         */
        DWORD SetParity(int parity);
       
        /**
         *  \brief Configura o comprimento de dados usado na comunicação serial
         *  \param parity Paridade do canal de comunicação. Olhe em Winbase.h.
         *  \return Código de Erro. 
         */
        DWORD SetByteSize(int byte_size);

        /**
         *  \brief Configura o tipo de handshake entre o host e o cliente
         *  \param SerialHandshake Tipo de protocolo de handshake utilizado. Olhe a estrutura EnumSerialHandshake.
         *  \return Código de Erro. 
         */
        DWORD SetHandshaking(EnumSerialHandshake SerialHandshake);	

        ///**
        // *  \brief Le dados da porta COMM, retornando o tamanho do dado lido, se houver timeout ou nenhum dado retorna 0
        // */
        //int Read(char *s, int len);

      
        /**
         *  \brief Escreve dados na porta COM, bloqueando ate que todos os dados sejam escritos, retorna o numero de dados
         *         escritos ou 0 se ocorrer timeout
         */
        int Write(char *s, int len);

        /**
         *  \brief  Escreve dados na porta COM, bloqueando ate que todos os dados sejam escritos, retorna o 
         *          numero de dados escritos ou 0 se ocorrer timeout, coloca um delay em milisegundos entre 
         *          cada caractere a ser transmitido    
         */
        int Write(char *s, int len, int delay);


        /**
         *  \brief  perform the action of sei the listenner function
         *  \param  func pointer to a function of type SERIAL_PORT_CALLBACK
         *  \return status of operation
         */
        DWORD RegisterListenner( SERIAL_PORT_CALLBACK func);
  };

};

#endif
