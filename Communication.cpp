
// 串口通信

#include "stdafx.h"
#include <windows.h>
#include "global.h"


// 打开流量计控制串口
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

	//SetCommMask(hCom, EV_RXCHAR) ;//设置时间掩码来监视指定通信端口上的事件
	SetupComm(hComMeter, 100, 100) ;

	//超时设置
	Timeouts.ReadIntervalTimeout = 100;
	Timeouts.ReadTotalTimeoutConstant = 300;
	Timeouts.ReadTotalTimeoutMultiplier = 1000;
	Timeouts.WriteTotalTimeoutConstant = 100;
	Timeouts.WriteTotalTimeoutMultiplier = 300;
	SetCommTimeouts(hComMeter, &Timeouts) ;

	GetCommState(hComMeter, &dcb) ;
	dcb.BaudRate = BaudRateMeter;		// 波特率
	dcb.ByteSize = 8;					// 传输格式8bit
	dcb.Parity = NOPARITY;				// 无奇偶校验位
	dcb.StopBits = ONESTOPBIT;			// 一个停止位
	SetCommState(hComMeter, &dcb);

	PurgeComm(hComMeter, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);		// 清空缓冲区

	return 0;
}

// 关闭已经打开的串口
void CloseComPorts()
{
	if (hComMeter != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hComMeter);
	}
}

// 读串口
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

// 写串口
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

