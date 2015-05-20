// SerialExample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Serial.h"

using namespace network;

int _tmain(int argc, _TCHAR* argv[])
{
    CSerial* pSerial;

    pSerial = new CSerial();
    pSerial->Open("\\\\.\\COM1");
    pSerial->SetBaudRate(CBR_9600);
    pSerial->SetParity(EVENPARITY);
    pSerial->SetStopBits(ONESTOPBIT);
    pSerial->SetHandshaking(HAND_SHAKE_OFF);


	return 0;
}

