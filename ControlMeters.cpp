
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "global.h"

using namespace std;
using namespace cv;

//������ģ�鶨��ĺ���
extern int CatchImage(int save);
extern void CreateImageShowMeter();
extern int ReadCom(HANDLE hCom, unsigned char data[], int number);
extern int WriteCom(HANDLE hCom, unsigned char data[], int number);
extern void CreateImageWithLine(int linelocation);

// ���Ϳ����������������
// ���أ�0������1ͨ��ʧ��
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
			break;		// �ɹ�
		}
	}

	if (errortimes == 3)
	{
		return 1;
	}

	return 0;
}

// ���Ϳ����������������
// ���أ�0������1ͨ��ʧ��
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
			break;		// �ɹ�
		}
	}

	if (errortimes == 3)
	{
		return 1;
	}

	return 0;
}

// ����������
// ���أ�0������1��ͨ��ʧ�ܣ�2����ȡͼ��ʧ��
int ControlMeters()
{
	int startx, endx;
	int i, j, ii, jj, x, y;
	int offset;
	int sumbright, minbright, minj;

	if (CatchImage(0) != 0)		// ��ȡͼ��
	{
		return 1;
	}
	CreateImageShowMeter();				// ������ʾͼ��
	
	// ��ǰ���õĿ̶�ͼ��ֻ��ʾһ��
	if (FirstImage == 0)
	{
		CreateImageWithLine(LocationExpected);
		//��ʾ����������ͼƬ
		::PostMessage(hWndMain, WM_SHOWPICLINEEXPECTED, 0, 0);
		FirstImage = 1;
	}
	
	// �������ʾͼ����ʾ
	::PostMessage(hWndMain, WM_SHOWPICMETER, 0, 0);
	
	// ��⸡��λ�ã����͵������ָ��
	// ������ͼ��ͼ��ImgMeterCorrectedShow, ͼ��ߴ磺ImageSizeXMeterShow * ImageSizeXMeterShow
	// Ŀ��λ�ã�LocationExpected

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

// �����������ƣ���ȷ����0�����󷵻�1
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
			break;		// �ɹ�
		}
	}

	if (errortimes == 3)
	{
		return 1;
	}

	return 0;
}
