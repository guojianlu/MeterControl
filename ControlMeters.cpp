
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "global.h"

using namespace std;
using namespace cv;

//引入别的模块定义的函数
extern int CatchImage(int save);
extern void CreateImageShowMeter();
extern int ReadCom(HANDLE hCom, unsigned char data[], int number);
extern int WriteCom(HANDLE hCom, unsigned char data[], int number);
extern void CreateImageWithLine(int linelocation);

// 发送控制命令，旋松流量计
// 返回：0正常；1通信失败
int SendCommandUnscrew()
{
	unsigned char senddata, recvdata;
	int errortimes;
	int retval;

	senddata = 0x11;

	errortimes = 0;
	while (errortimes < 3)
	{
		retval = WriteCom(hComMeter, &senddata, 1);
		if (retval != 1)
		{
			errortimes ++;
			continue;
		}
		retval = ReadCom(hComMeter, &recvdata, 1);
		if ( (retval != 1) || (recvdata != (senddata | 0xF0)) )
		{
			errortimes ++;
			continue;
		}
		else
		{
			break;		// 成功
		}
	}

	if (errortimes == 3)
	{
		return 1;
	}

	return 0;
}

// 发送控制命令，旋紧流量计
// 返回：0正常；1通信失败
int SendCommandScrew()
{
	unsigned char senddata, recvdata;
	int errortimes;
	int retval;
	
	senddata = 0x00;

	errortimes = 0;
	while (errortimes < 3)
	{
		retval = WriteCom(hComMeter, &senddata, 1);
		if (retval != 1)
		{
			errortimes ++;
			continue;
		}
		retval = ReadCom(hComMeter, &recvdata, 1);
		if ( (retval != 1) || (recvdata != (senddata | 0xF0)) )
		{
			errortimes ++;
			continue;
		}
		else
		{
			break;		// 成功
		}
	}

	if (errortimes == 3)
	{
		return 1;
	}

	return 0;
}

// 控制流量计
// 返回：0正常；1：通信失败；2：获取图像失败
int ControlMeters()
{
	int startx, endx;
	int i, j, ii, jj, x, y;
	int offset;
	int sumbright, minbright, minj;

	if (CatchImage(0) != 0)		// 获取图像
	{
		return 1;
	}
	CreateImageShowMeter();				// 生成显示图像
	
	// 当前设置的刻度图像，只显示一次
	if (FirstImage == 0)
	{
		CreateImageWithLine(LocationExpected);
		//显示流量计设置图片
		::PostMessage(hWndMain, WM_SHOWPICLINEEXPECTED, 0, 0);
		FirstImage = 1;
	}
	
	// 界面的显示图像显示
	::PostMessage(hWndMain, WM_SHOWPICMETER, 0, 0);
	
	// 检测浮球位置，发送电机控制指令
	// 流量计图像图像：ImgMeterCorrectedShow, 图像尺寸：ImageSizeXMeterShow * ImageSizeXMeterShow
	// 目标位置：LocationExpected

	startx = 40; 
	endx = 100;

	minbright = ImageSizeXMeterShow * ImageSizeYMeterShow * 255 + 1;
	for (i = startx; i < endx; i++ )
	{
		for (j = FloatBallSize; j < ImageSizeYMeterShow; j++)
		{
			sumbright = 0;
			for (ii = 0; ii < FloatBallSize; ii++)
			{
				for (jj = 0; jj < FloatBallSize; jj++)
				{
					x = i + ii;
					y = j - jj;
					offset = y * ImgMeterCorrectedShow->widthStep + x;
					sumbright += (unsigned char) ImgMeterCorrectedShow->imageData[offset];
				}
			}
			if (sumbright < minbright)
			{
				minbright = sumbright;
				minj = j;
			}
		}
	}

	// control meters
	if (StartStates==1)
	{
		if (minj > LocationExpected + 10)
		{
			return SendCommandUnscrew();		// lower, unscrew
		}
		if (minj < LocationExpected - 10)
		{
			return SendCommandScrew();			// higher, screw
		}

	}
	return 0;
}

// 测试流量控制，正确返回0，错误返回1
int TestMeterCom()
{
	unsigned char senddata, recvdata;
	int errortimes;
	int retval;

	errortimes = 0;
	senddata = 0x5A;
	while (errortimes < 3)
	{
		retval = WriteCom(hComMeter, &senddata, 1);
		if (retval != 1)
		{
			errortimes ++;
			continue;
		}
		retval = ReadCom(hComMeter, &recvdata, 1);
		if ( (retval != 1) || (recvdata != 0x0A) )
		{
			errortimes ++;
			continue;
		}
		else
		{
			break;		// 成功
		}
	}

	if (errortimes == 3)
	{
		return 1;
	}

	return 0;
}
