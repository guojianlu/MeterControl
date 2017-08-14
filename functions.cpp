
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <pylon/PylonIncludes.h>
#include "global.h"

using namespace GenApi;
using namespace Pylon;
using namespace std;
using namespace cv;


//----------------------------------------------------------------
#define USE_GIGE
//----------------------------------------------------------------
#if defined( USE_1394 )
// settings to use  Basler 1394 cameras
#include <pylon/1394/Basler1394Camera.h>
typedef Pylon::CBasler1394Camera Camera_t;
using namespace Basler_IIDC1394CameraParams;
using namespace Basler_IIDC1394StreamParams;
#elif defined ( USE_GIGE )
// settings to use Basler GigE cameras
#include <pylon/gige/BaslerGigeCamera.h>
typedef Pylon::CBaslerGigECamera Camera_t;
using namespace Basler_GigECameraParams;
using namespace Basler_GigEStreamParams;
#else
#error Camera type is not specified. For example, define USE_GIGE for using GigE cameras
#endif


// ��ʼ���������   camera meter, alarm0, alarm1, alarm2, alarm3����
CTlFactory *TlFactory;
DeviceInfoList_t devices;
IPylonDevice *Camera = NULL;
IStreamGrabber *StreamGrabber = NULL;
GenApi::INodeMap *CameraNodeMap = NULL;
GenApi::INodeMap *pGrabberNodeMap = NULL;
uint8_t *pBuffer = NULL;
StreamBufferHandle hBuffer = NULL;

//----------------------------------------------------------------------------------
// ��ʼ������
//----------------------------------------------------------------------------------
int InitParameters()
{
	int count;

	ImageWidth = 1280;
	ImageHeight = 960;
	ImageStartX = 0;
	ImageStartY = 0;

	FloatBallSize = 20;

	try
	{
		//�ӳ�ʼ���ļ��ж�ȡ�������
		ifstream fin("MeterControl.ini");

		string infostring, datastr;
		size_t k;
		count = 0;
		while (getline(fin, infostring))
		{
			count ++;
			// --------------------------------------
			//       config meter camera 0 parameters
			// --------------------------------------

			//CameraSerialNumber
			k = infostring.find("CameraSerialNumber=");
			if (k != string::npos)
			{
				k += strlen("CameraSerialNumber=");
				datastr = infostring.substr(k);
				CameraSerialNumber = std::atoi(datastr.c_str()); 
			}
			//CameraExposureTimeAbs
			k = infostring.find("CameraExposureTimeAbs=");
			if (k != string::npos)
			{
				k += strlen("CameraExposureTimeAbs=");
				datastr = infostring.substr(k);
				CameraExposureTimeAbs = std::atoi(datastr.c_str()); 
			}

			// --------------------------------------
			//       communication parameters
			// --------------------------------------

			//ComPortMeter
			k = infostring.find("ComPortMeter=");
			if (k != string::npos)
			{
				k += strlen("ComPortMeter=");
				datastr = infostring.substr(k);
				ComPortMeter = std::atoi(datastr.c_str()); 
			}
			//BaudRateMeter
			k = infostring.find("BaudRateMeter=");
			if (k != string::npos)
			{
				k += strlen("BaudRateMeter=");
				datastr = infostring.substr(k);
				BaudRateMeter = std::atoi(datastr.c_str()); 
			}
			// --------------------------------------
			//   Meter Calibration parameters  �̶Ȳ���
			// --------------------------------------
			//ImageSizeXMeterShow
			k = infostring.find("ImageSizeXMeterShow=");
			if (k != string::npos)
			{
				k += strlen("ImageSizeXMeterShow=");
				datastr = infostring.substr(k);
				ImageSizeXMeterShow = std::atoi(datastr.c_str()); 
			}
			//ImageSizeYMeterShow
			k = infostring.find("ImageSizeYMeterShow=");
			if (k != string::npos)
			{
				k += strlen("ImageSizeYMeterShow=");
				datastr = infostring.substr(k);
				ImageSizeYMeterShow = std::atoi(datastr.c_str()); 
			}
			//SuggestMetersLocationTopY
			k = infostring.find("SuggestMetersLocationTopY=");
			if (k != string::npos)
			{
				k += strlen("SuggestMetersLocationTopY=");
				datastr = infostring.substr(k);
				SuggestMetersLocationTopY = std::atoi(datastr.c_str()); 
			}
			//SuggestMetersLocationLeftX
			k = infostring.find("SuggestMetersLocationLeftX=");
			if (k != string::npos)
			{
				k += strlen("SuggestMetersLocationLeftX=");
				datastr = infostring.substr(k);
				SuggestMetersLocationLeftX = std::atoi(datastr.c_str()); 
			}
		}

		fin.close();
		if (count == 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	catch( exception &e )
	{
		// Error handling
		ErrInfo = e.what();
		return 1;
	}
}


//----------------------------------------------------------------------------------
// ��ʼ�������������Ӧ���������
// ����ֵ����ȷ0�� �����0
//----------------------------------------------------------------------------------
int InitCamera()
{
	int cameranum;
	int j;
	char strcameraid[100];

	try
	{
		// Get the transport layer factory
		TlFactory = &CTlFactory::GetInstance();
		
		devices.clear ();
		cameranum = TlFactory->EnumerateDevices( devices );
		if ( cameranum == 0 )
		{
			ErrInfo = _T("ϵͳδ�������");
			return 1;
		}

        // ͨ��������к�ƥ�������λ
		sprintf_s(strcameraid, sizeof(strcameraid), "%d", CameraSerialNumber);
		CameraOn = 0;
		for (j = 0; j < cameranum; j++)
		{
			if (strcmp(devices[j].GetSerialNumber(), strcameraid) == 0)
			{
				Camera = TlFactory->CreateDevice( devices[j] );
				CameraOn = 1;
				break;
			}
		}
		if (j == cameranum)
		{
			ErrInfo = _T("ϵͳδ����ָ��������������������ܼ�������");
			return 1;
		}

		//------------------------------------------------------
		// Config the cameras
		//------------------------------------------------------
		if (CameraOn == 1)
		{
			// Get the first stream grabber object of the selected camera
			StreamGrabber = Camera->GetStreamGrabber( 0 );

			// Open the camera
			Camera->Open();

			// Open the stream grabber
			StreamGrabber->Open();

			//�����������
			CameraNodeMap = Camera->GetNodeMap();

			// AOI
			CIntegerPtr ptrAoiWidth = CameraNodeMap->GetNode ("Width");
			CIntegerPtr ptrAoiHeight = CameraNodeMap->GetNode ("Height");
			CIntegerPtr ptrOffsetX = CameraNodeMap->GetNode ("OffsetX");
			CIntegerPtr ptrOffsetY = CameraNodeMap->GetNode ("OffsetY");

			ptrAoiWidth->SetValue(ImageWidth);
			ptrAoiHeight->SetValue(ImageHeight);
			ptrOffsetX->SetValue(ImageStartX);
			ptrOffsetY->SetValue(ImageStartY);
			
			//��֡ģʽ
			CEnumerationPtr ptrAcqMode = CameraNodeMap->GetNode("AcquisitionMode");
			if ( ! ptrAcqMode ) {
				ErrInfo = _T("There is no acquisition mode parameter");
				return 1;
			}
			if ( ! IsWritable( ptrAcqMode ) ) {
				ErrInfo = _T("The acquisition mode parameter is not writable");
				return 1;
			}
			ptrAcqMode->FromString("SingleFrame");
				
			
			//�ع�ʱ��
			CFloatPtr ptrExposureTime = CameraNodeMap->GetNode("ExposureTimeAbs");
			if ( ! ptrExposureTime ) {
				ErrInfo = _T("There is no exposure time parameter");
				return 1;
			}
			if ( ! IsWritable( ptrExposureTime ) ) {
				ErrInfo = _T("The exposure time parameter is not writable");
				return 1;
			}
			ptrExposureTime->SetValue( CameraExposureTimeAbs );

			//�ⴥ��
			CEnumerationPtr ptrTriggerMode = CameraNodeMap->GetNode("TriggerMode");
			if ( ! ptrTriggerMode ) {
				ErrInfo = _T("There is no TriggerMode parameter");
				return 1;
			}
			if ( ! IsWritable( ptrTriggerMode ) ) {
				ErrInfo = _T("The TriggerMode parameter is not writable");
				return 1;
			}
			ptrTriggerMode->FromString( "Off" );


			// Create a image buffer
 			CIntegerPtr ptrPayloadSize = CameraNodeMap->GetNode ("PayloadSize");
			const size_t ImageSize = (size_t) ( ptrPayloadSize->GetValue () );
	        
			if (pBuffer == NULL)    
			{
				pBuffer = new uint8_t[ ImageSize ];
				if (pBuffer == NULL)
				{
					ErrInfo = _T("�ڴ�����޷�Ϊ��������㹻�ڴ�");
					return 1;
				}
			}

			// get Node Map for streamgrabber configuration
			pGrabberNodeMap = StreamGrabber->GetNodeMap ();

			// We won't use image buffers greater than ImageSize
			CIntegerPtr ptrMaxBufferSize = pGrabberNodeMap->GetNode ("MaxBufferSize");
			ptrMaxBufferSize->SetValue (ImageSize);

			// We won't queue more than c_nBuffers image buffer at a time
			CIntegerPtr ptrMaxNumBuffer = pGrabberNodeMap->GetNode ("MaxNumBuffer");
			ptrMaxNumBuffer->SetValue ( 1 );

			// Allocate all resources for grabbing. Critical parameters like image
			// size now must not be changed until FinishGrab() is called.
			StreamGrabber->PrepareGrab();

			// Buffers used for grabbing must be registered at the stream grabber.
			// The registration returns a handle to be used for queuing the buffer
			hBuffer = StreamGrabber->RegisterBuffer( pBuffer, ImageSize );

			// Put buffer into the grab queue for grabbing
			StreamGrabber->QueueBuffer( hBuffer, NULL );
		}
	}
    catch(GenICam::GenericException &e )
    {
        ErrInfo = e.GetDescription();
		ErrInfo = ErrInfo + CString("\n");
        return 1;
    }

	return 0;
}

//---------------------------------------------------------------------------------------
// �ر���������ͷ���Ӧ���ڴ�ռ�
//---------------------------------------------------------------------------------------
int CloseCamera()
{	
	GrabResult Result;			// grab result

	try  
	{
		if (Camera && Camera->IsOpen ())
		{
			StreamGrabber->CancelGrab();

			while ( StreamGrabber->GetWaitObject().Wait(1000) )
			{
				StreamGrabber->RetrieveResult( Result );
/*				if ( Result.Status() == Canceled )
				{
					fout << "Got canceled buffer " << k << endl;
				}
*/			}
			// Deregister and free buffers
			StreamGrabber->DeregisterBuffer( hBuffer );
			delete [] pBuffer;
			pBuffer = NULL;

			StreamGrabber->FinishGrab();
			StreamGrabber->Close();
			Camera->Close();
			TlFactory->DestroyDevice (Camera);
			Camera = NULL;
		}
	}
	catch( GenICam::GenericException &e )
	{
		// Error handling
		ErrInfo = e.GetDescription();
		ErrInfo = _T("�ر���������������̴���\n") + ErrInfo;
	}
	
	return 0;
}

//---------------------------------------------------------------------------------------
// ���ƻ�ȡͼ��
/// save==1, ��ͼ�񱣴浽��ǰĿ¼�� \output\image ��Ŀ¼��
//---------------------------------------------------------------------------------------
int CatchImage(int save)
{
	char filename[100];
	char str[200];
	int xsize, ysize, cnt;
	int i;
	CTime t;
	int  year, month, day, hour, minute, second;
	int retval;

	//----------------------------------------------------------
	// ���������ȡͼ��
	//----------------------------------------------------------

	// Start acquisition on the camera
	CCommandPtr ptrAcquisitionStart = CameraNodeMap->GetNode ("AcquisitionStart");
	ptrAcquisitionStart->Execute ();
	//    if  (! waitObjects.WaitForAny (1000))    
	if (! StreamGrabber->GetWaitObject().Wait(1000) )
	{
		// timeout occurred, should never happen when using INFINITE
		ErrInfo = _T("ͼ���ȡ���̳�ʱ");
		StreamGrabber->CancelGrab();
		for ( GrabResult r; StreamGrabber->RetrieveResult( r ););
		return 1;
	}

	GrabResult Result;			// grab result

	StreamGrabber->RetrieveResult(Result);
	if ( Result.Succeeded() )
	{
		xsize = Result.GetSizeX();
		ysize = Result.GetSizeY();
		const uint8_t *pImageBuffer = (uint8_t *) Result.Buffer();

		cnt = xsize * ysize;

		for (i = 0; i < cnt; i++)
		{
			*(ImgMeterRough->imageData + i) = pImageBuffer[i];
		}

		StreamGrabber->QueueBuffer( Result.Handle(), NULL );

		// save images
		if (save == 1)
		{

			t = CTime::GetCurrentTime(); 
			year = t.GetYear();
			month = t.GetMonth();
			day = t.GetDay();
			hour = t.GetHour();
			minute = t.GetMinute();
			second = t.GetSecond();
			
			sprintf_s(filename, sizeof(filename), "output\\image\\ImgMeterRough_%04d%02d%02d%02d%02d%02d.bmp", year, month, day, hour, minute, second);
			retval = cvSaveImage(filename, ImgMeterRough);

			if (retval == 0) 
			{
				ErrInfo = _T("�޷�����ͼ��:") + CString(filename);
				return 1;
			}
		}			
		return 0;
	}
	else
	{
		// Error Handling
		sprintf_s(str, sizeof(str), "��ȡ������ͼ�񲻳ɹ�\n");
	
		ErrInfo =  CString(str);
		ErrInfo = ErrInfo + CString(Result.GetErrorDescription());

		return 1;
	}
}

//---------------------------------------------------------------------------------------
// ��ԭʼͼ���н�ȡ����������Ȥ����
// ���Ͻ����꣺��SuggestMetersLocationLeftX, SuggestMetersLocationTopY��
// ͼ��ߴ磺��ImageSizeXMeterShow, ImageSizeYMeterShow)
//---------------------------------------------------------------------------------------
void CreateImageShowMeter()
{
	char filename[200];
	CvRect rect;

	rect = cvRect(SuggestMetersLocationLeftX, SuggestMetersLocationTopY, ImageSizeXMeterShow, ImageSizeYMeterShow);
	cvSetImageROI(ImgMeterRough, rect);
	cvCopy(ImgMeterRough, ImgMeterCorrectedShow);
	cvResetImageROI(ImgMeterRough);
	sprintf_s(filename, sizeof(filename), "output\\ImgShowMeter.bmp");
	//cvSaveImage(filename, ImgMeterCorrectedShow);
}

//---------------------------------------------------------------------------------------
// ���ɴ����ߵ�ϣ������ͼ��
// ���ȹ����ɫͼ���ٻ�����
//---------------------------------------------------------------------------------------
void CreateImageWithLine(int linelocation)
{
	int i;
	CvPoint pt1, pt2;
	char filename[200];

	// copy from mono bmp to RGB bmp
	for (i = 0; i < ImgMeterCorrectedShow->imageSize; i ++)
	{
		ImgMeterShowWithLine->imageData[i * 3] = ImgMeterCorrectedShow->imageData[i];
		ImgMeterShowWithLine->imageData[i * 3 + 1] = ImgMeterCorrectedShow->imageData[i];
		ImgMeterShowWithLine->imageData[i * 3 + 2] = ImgMeterCorrectedShow->imageData[i];
	}

	// line
	if (linelocation >= (ImageSizeYMeterShow - 46))
	{
		linelocation = ImageSizeYMeterShow - 46;
	}
	else if (linelocation <= 87){
		linelocation = 87;
	}
	pt1.x = 0;
	pt1.y = linelocation;
	pt2.x = ImageSizeXMeterShow - 1;
	pt2.y = linelocation;
	cvLine(ImgMeterShowWithLine, pt1, pt2, cvScalar(255, 0, 0), 3);

	// save image
	sprintf_s(filename, sizeof(filename), "output\\ImgShowMeterWithLine.bmp");
	//cvSaveImage(filename, ImgMeterShowWithLine);
}


//----------------------------------------------------------------------
// ΪӦ���е�����ͼ�����ռ�
//----------------------------------------------------------------------
int AllocImages()
{
	ImgMeterRough = cvCreateImage( (cvSize(ImageWidth, ImageHeight)), IPL_DEPTH_8U, 1);
	if (ImgMeterRough == NULL)
	{
		return 1;
	}

	ImgMeterCorrectedShow = cvCreateImage( (cvSize(ImageSizeXMeterShow, ImageSizeYMeterShow)), IPL_DEPTH_8U, 1);
	if (ImgMeterCorrectedShow == NULL)
	{
		return 1;
	}

	ImgMeterShowWithLine = cvCreateImage( (cvSize(ImageSizeXMeterShow, ImageSizeYMeterShow)), IPL_DEPTH_8U, 3);
	if (ImgMeterShowWithLine == NULL)
	{
		return 1;
	}

	return 0;
}


//----------------------------------------------------------------------
// ΪӦ���е�����ͼ���ͷſռ�
//----------------------------------------------------------------------
void FreeImages()
{
	if (ImgMeterRough != NULL)
	{
		cvReleaseImage(&ImgMeterRough);
	}

	if (ImgMeterCorrectedShow != NULL)
	{
		cvReleaseImage(&ImgMeterCorrectedShow);
	}

	if (ImgMeterShowWithLine != NULL)
	{
		cvReleaseImage(&ImgMeterShowWithLine);
	}
}

//----------------------------------------------------------------------
// װ�������ƿ��Ʋ���
//----------------------------------------------------------------------
int LoadMeterPara()
{
	FILE *fp;
	char filename[100];
	char str[200];

	// ����Meter�趨����
	sprintf_s(filename, sizeof(filename), "MeterData.dat");
	fopen_s(&fp, filename, "rb");
	if (fp == NULL)
	{
		sprintf_s(str, sizeof(str), "�����Ʊ궨�����ļ� %s �򿪴���\n", filename);
		ErrInfo = CString(str);
		return 1;
	}


	fread(&LocationExpected, sizeof(int), 1, fp);
	TempLocationExpected = LocationExpected;

	fclose(fp);
	return 0;
}
