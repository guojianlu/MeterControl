#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;

CString ErrInfo;

// 主窗口句柄
HWND hWndMain;	

// 线程控制
int StartStates;
int ThreadClosed;
int StopThread;

// 相机参数
int CameraOn;			// 相机是否连接

// 相机基本参数
int CameraSerialNumber;       //相机序列号
int CameraExposureTimeAbs;    //相机曝光时间的绝对值
int ImageStartX;
int ImageStartY;
int ImageWidth;
int ImageHeight;

// 从流量计的图像中分离出显示图像
int ImageSizeXMeterShow;            //ROL图像的宽
int ImageSizeYMeterShow;			//ROL图像的高
int SuggestMetersLocationTopY;		// 保存在ini文件中的流量计顶端Y坐标，将在流量计标定时被写入数据文件
int SuggestMetersLocationLeftX;

int LocationExpected;				// 期望的刻度像素
int TempLocationExpected;			// 临时保存设置的刻度像素
int FloatBallSize;					// 浮球在图像中的像素尺寸
int FirstImage;						// 没获得流量计图像前为0， 之后为1

// 全局图像
IplImage* ImgMeterRough = NULL;									// for meter
IplImage* ImgMeterCorrectedShow = NULL;	
IplImage* ImgMeterShowWithLine =  NULL;	


// 通信参数
int ComPortMeter;
int BaudRateMeter;
HANDLE hComMeter = INVALID_HANDLE_VALUE;

//启动标志
