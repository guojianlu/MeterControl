

#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;

extern CString ErrInfo;



// ���ھ��
extern HWND hWndMain;	

// �Զ�����Ϣ
#define WM_SHOWPICMETER (WM_USER + 102)
#define WM_SHOWPICLINEEXPECTED (WM_USER + 103)

// �߳̿���
extern int ThreadClosed;
extern int StopThread;
extern int StartStates;

// �������
extern int CameraOn;			// ����Ƿ�����

// �����������
extern int CameraSerialNumber;
extern int CameraExposureTimeAbs;
extern int ImageStartX;
extern int ImageStartY;
extern int ImageWidth;
extern int ImageHeight;

// �������Ƶ�ͼ���з������ʾͼ��
extern int ImageSizeXMeterShow;
extern int ImageSizeYMeterShow;

extern int SuggestMetersLocationTopY;		// ������ini�ļ��е������ƶ���Y���꣬���������Ʊ궨ʱ��д�������ļ�
extern int SuggestMetersLocationLeftX;

extern int LocationExpected;				// �����Ŀ̶�����
extern int TempLocationExpected;
extern int FloatBallSize;				// ������ͼ���е����سߴ�
extern int FirstImage;	

// ȫ��ͼ��
extern IplImage* ImgMeterRough;									// for meter
extern IplImage* ImgMeterCorrectedShow;	
extern IplImage* ImgMeterShowWithLine;	

// ͨ�Ų���
extern int ComPortMeter;
extern int BaudRateMeter;
extern HANDLE hComMeter;