

#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;

extern CString ErrInfo;



// 窗口句柄
extern HWND hWndMain;	

// 自定义消息
#define WM_SHOWPICMETER (WM_USER + 102)
#define WM_SHOWPICLINEEXPECTED (WM_USER + 103)

// 线程控制
extern int ThreadClosed;
extern int StopThread;
extern int StartStates;

// 相机参数
extern int CameraOn;			// 相机是否连接

// 相机基本参数
extern int CameraSerialNumber;
extern int CameraExposureTimeAbs;
extern int ImageStartX;
extern int ImageStartY;
extern int ImageWidth;
extern int ImageHeight;

// 从流量计的图像中分离出显示图像
extern int ImageSizeXMeterShow;
extern int ImageSizeYMeterShow;

extern int SuggestMetersLocationTopY;		// 保存在ini文件中的流量计顶端Y坐标，将在流量计标定时被写入数据文件
extern int SuggestMetersLocationLeftX;

extern int LocationExpected;				// 期望的刻度像素
extern int TempLocationExpected;
extern int FloatBallSize;				// 浮球在图像中的像素尺寸
extern int FirstImage;	

// 全局图像
extern IplImage* ImgMeterRough;									// for meter
extern IplImage* ImgMeterCorrectedShow;	
extern IplImage* ImgMeterShowWithLine;	

// 通信参数
extern int ComPortMeter;
extern int BaudRateMeter;
extern HANDLE hComMeter;