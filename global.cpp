#include "stdafx.h"
#include <opencv2/opencv.hpp>

using namespace cv;

CString ErrInfo;

// �����ھ��
HWND hWndMain;	

// �߳̿���
int StartStates;
int ThreadClosed;
int StopThread;

// �������
int CameraOn;			// ����Ƿ�����

// �����������
int CameraSerialNumber;       //������к�
int CameraExposureTimeAbs;    //����ع�ʱ��ľ���ֵ
int ImageStartX;
int ImageStartY;
int ImageWidth;
int ImageHeight;

// �������Ƶ�ͼ���з������ʾͼ��
int ImageSizeXMeterShow;            //ROLͼ��Ŀ�
int ImageSizeYMeterShow;			//ROLͼ��ĸ�
int SuggestMetersLocationTopY;		// ������ini�ļ��е������ƶ���Y���꣬���������Ʊ궨ʱ��д�������ļ�
int SuggestMetersLocationLeftX;

int LocationExpected;				// �����Ŀ̶�����
int TempLocationExpected;			// ��ʱ�������õĿ̶�����
int FloatBallSize;					// ������ͼ���е����سߴ�
int FirstImage;						// û���������ͼ��ǰΪ0�� ֮��Ϊ1

// ȫ��ͼ��
IplImage* ImgMeterRough = NULL;									// for meter
IplImage* ImgMeterCorrectedShow = NULL;	
IplImage* ImgMeterShowWithLine =  NULL;	


// ͨ�Ų���
int ComPortMeter;
int BaudRateMeter;
HANDLE hComMeter = INVALID_HANDLE_VALUE;

//������־
