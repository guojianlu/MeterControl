
// ����ͨ��

#include "stdafx.h"
#include <windows.h>
#include "global.h"


// �������ƿ��ƴ���
int OpenComMeter()
{
	char strComName[100];
	WCHAR wstrComName[256];
	COMMTIMEOUTS Timeouts ;
	DCB dcb ;

	sprintf_s(strComName, sizeof(strComName), "COM%d", ComPortMeter);
	memset(wstrComName, 0, sizeof(wstrComName));
	MultiByteToWideChar(CP_ACP, 0, strComName, strlen(strComName), wstrComName, sizeof(wstrComName) / sizeof(wstrComName[0]));

	hComMeter = CreateFile(wstrComName, 
		GENERIC_READ|GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hComMeter == INVALID_HANDLE_VALUE)
	{
		return 1 ;
	}

	//SetCommMask(hCom, EV_RXCHAR) ;//����ʱ������������ָ��ͨ�Ŷ˿��ϵ��¼�
	SetupComm(hComMeter, 100, 100) ;

	//��ʱ����
	Timeouts.ReadIntervalTimeout = 100;
	Timeouts.ReadTotalTimeoutConstant = 300;
	Timeouts.ReadTotalTimeoutMultiplier = 1000;
	Timeouts.WriteTotalTimeoutConstant = 100;
	Timeouts.WriteTotalTimeoutMultiplier = 300;
	SetCommTimeouts(hComMeter, &Timeouts) ;

	GetCommState(hComMeter, &dcb) ;
	dcb.BaudRate = BaudRateMeter;		// ������
	dcb.ByteSize = 8;					// �����ʽ8bit
	dcb.Parity = NOPARITY;				// ����żУ��λ
	dcb.StopBits = ONESTOPBIT;			// һ��ֹͣλ
	SetCommState(hComMeter, &dcb);

	PurgeComm(hComMeter, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);		// ��ջ�����

	return 0;
}

// �ر��Ѿ��򿪵Ĵ���
void CloseComPorts()
{
	if (hComMeter != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hComMeter);
	}
}

// ������
int ReadCom(HANDLE hCom, unsigned char data[], int number)
{
	DWORD realnumber;
	BOOL bReadState;

	bReadState = ReadFile(hCom, data, number, &realnumber, NULL);
	if (! bReadState)
	{
		return -1;
	}
	else
	{
		return realnumber;
	}
}

// д����
int WriteCom(HANDLE hCom, unsigned char data[], int number)
{
	DWORD realnumber;
	BOOL bWriteState;

	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	bWriteState = WriteFile(hCom, data, number, &realnumber, NULL) ;
	if (! bWriteState)
	{
		return -1;
	}
	else
	{
		return realnumber;
	}
}

