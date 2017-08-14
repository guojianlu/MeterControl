
// MeterControlDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MeterControl.h"
#include "MeterControlDlg.h"
#include "afxdialogex.h"
#include "global.h"
#include "CvvImage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// extern functions
extern int InitParameters();
extern int LoadMeterPara();
extern int InitCamera();
extern int CloseCamera();
extern int AllocImages(); 
extern void FreeImages();
extern int OpenComMeter();
extern void CloseComPorts();
extern int ControlMeters();
extern int TestMeterCom();
extern void CreateImageWithLine(int linelocation);



// CMeterControlDlg �Ի���




CMeterControlDlg::CMeterControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMeterControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMeterControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCROLLBAR_SET, m_ScrollBar);
}
//��ع㲥
BEGIN_MESSAGE_MAP(CMeterControlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWPICMETER, OnShowPicMeter)	
	ON_MESSAGE(WM_SHOWPICLINEEXPECTED, OnShowPicLineExpected)	
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SET, &CMeterControlDlg::OnClickedButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CMeterControlDlg::OnClickedButtonCancel)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_START, &CMeterControlDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CMeterControlDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CMeterControlDlg ��Ϣ�������

BOOL CMeterControlDlg::OnInitDialog()
{
	int retval;

	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//------------------------------------------------------------------
	// ��ȡ���ھ��
	hWndMain = AfxGetMainWnd()->m_hWnd;
	ThreadClosed = 1;

	// ��ʼ������
	if (InitParameters() != 0)
	{
		MessageBox(_T("��MeterControl.ini�����ʼ�����ݴ���ϵͳ��������ȷ����\n") + ErrInfo, _T("����"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}

	//����ռ�
	retval = AllocImages();
	if (retval != 0)
	{
		MessageBox(_T("ϵͳ�ڴ�������ϵͳ��������ȷ����\n"), _T("����"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}

	// �����ƿ��������
	if (LoadMeterPara() != 0)
	{
		MessageBox(_T("��ȡ�����Ʊ궨���ݴ���ϵͳ��������ȷ����\n") + ErrInfo, _T("����"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}

	// ׼����ʾ��ǰ��������ͼ��
	FirstImage = 0;

	// ���������Ʊ궨���ڲ�������������λ�ó�ֵ
	m_ScrollBar.SetScrollRange(80, ImageSizeYMeterShow - 50);
	m_ScrollBar.SetScrollPos(LocationExpected);

	// ����������ͨ�Ŷ˿�
	if (OpenComMeter() != 0)
	{
		MessageBox(_T("��������ͨ�Ŷ˿ڴ򿪴���ϵͳ��������ȷ����\n") + ErrInfo, _T("����"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}
	//// �ж�ͨ��
	//if (TestMeterCom() != 0)
	//{
	//	MessageBox(_T("����������ͨ�Ŵ���ϵͳ��������ȷ����\n") + ErrInfo, _T("����"), MB_OK);
	//	//::PostMessage(hWndMain, WM_CLOSE, 0, 0);
	//	::PostMessage(hWndMain, WM_QUIT, 0, 0);
	//	FreeResource();
	//	this->OnCancel();
	//	return FALSE;
	//}

	// ��ʼ�����
	retval = InitCamera();
	if (retval == 1)
	{
		MessageBox(_T("��ʼ���������ϵͳ��������ȷ���У������������Ȼ�������������\n") + ErrInfo, _T("����"), MB_OK);
		ErrInfo = _T("");
		if (CloseCamera() != 0)
		{
			MessageBox(_T("�ر��ѳ�ʼ����������з�������\n") + ErrInfo, _T("����"), MB_OK);
			ErrInfo = _T("");
		}
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}
	
	GetDlgItem(IDC_BUTTON_CLOSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);

	
	
	// �����߳�
	StartStates = 0;
	StopThread = 0;
	AfxBeginThread(ThreadWorks, this);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMeterControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMeterControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMeterControlDlg::FreeResource()
{
	// �ر��߳�
	StopThread = 1;
	while (ThreadClosed == 0)
	{
		MyDoEvents();
	}

	// �رմ���
	CloseComPorts();

	// �ر����
	if (CloseCamera() != 0)
	{
		MessageBox(_T("�ر���������з�������\n") + ErrInfo, _T("����"), MB_OK);
		ErrInfo = _T("");
	}

	//�ͷſռ�
	FreeImages();
}


// �����̣߳�����OCR����������ƿ��ƣ���������͵���Ӧ����
UINT CMeterControlDlg::ThreadWorks(LPVOID pParam)
{
	int t1, t2;

	ThreadClosed = 0;
	while(StopThread != 1)
	{
		// ��������
		t1 = GetTickCount();

		ControlMeters();
		t2 = GetTickCount();

		//t1 = GetTickCount();
		//t2 = t1;
		while (t2 - t1 < 100)
		{
			MyDoEvents();
			t2 = GetTickCount();
		}
	}

	ThreadClosed = 1;
	return 0;
}


//-----------------------------------------------------------------------------
//��ʱ�ͷ�CPU����Ȩ
//-----------------------------------------------------------------------------
BOOL CMeterControlDlg::MyDoEvents()

{
	MSG msg;

	//CheckMessageQueue() 
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))  {
		if (msg.message == WM_QUIT)
		{
			return FALSE; 
		}
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	} 

	return TRUE; 
}

// ��ʾ������ͼƬ
LRESULT CMeterControlDlg::OnShowPicMeter(WPARAM wParam, LPARAM lParam)
{
	CDC *pDC = GetDlgItem(IDC_PIC_REAL)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(IDC_PIC_REAL)->GetClientRect(&rect);
	rect.right = rect.left + ImageSizeXMeterShow;
	rect.bottom = rect.top + ImageSizeYMeterShow;
	CvvImage cimg;
	cimg.CopyOf(ImgMeterCorrectedShow);

	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);

	return 0;
}

// ��ʾ����������ͼƬ
LRESULT CMeterControlDlg::OnShowPicLineExpected(WPARAM wParam, LPARAM lParam)
{
	CDC *pDC = GetDlgItem(IDC_PIC_SET)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(IDC_PIC_SET)->GetClientRect(&rect);
	rect.right = rect.left + ImageSizeXMeterShow;
	rect.bottom = rect.top + ImageSizeYMeterShow;
	CvvImage cimg;
	cimg.CopyOf(ImgMeterShowWithLine);

	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);

	return 0;
}

void CMeterControlDlg::OnClose()
{
	FreeResource();

	CDialogEx::OnClose();
}

void CMeterControlDlg::OnClickedButtonSet()
{
	char filename[100];
	FILE *fp;

	sprintf_s(filename, "MeterData.dat");
	fopen_s(&fp, filename, "wb");
	if (fp == NULL)
	{
		ErrInfo = _T("���궨������浽Ӳ���ļ� ") + CString(filename) + _T(" ʱ����\n");
		return;
	}

	LocationExpected = TempLocationExpected;
	fwrite(&LocationExpected, sizeof(int), 1, fp);
	fclose(fp);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);

	MessageBox(_T("Ԥ������ֵ�޸���Ч\n"), MB_OK);
}


void CMeterControlDlg::OnClickedButtonCancel()
{
	TempLocationExpected = LocationExpected;
	CreateImageWithLine(LocationExpected);
	::PostMessage(hWndMain, WM_SHOWPICLINEEXPECTED, 0, 0);
}


void CMeterControlDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch (nSBCode)
	{
	case SB_LINEUP:
		TempLocationExpected -= 1;
		if (TempLocationExpected < 0)
		{
			TempLocationExpected = 0;
		}
		break;
	case SB_LINEDOWN:
		TempLocationExpected += 1;
		if (TempLocationExpected >= ImageSizeYMeterShow)
		{
			TempLocationExpected = ImageSizeYMeterShow - 1;
		}
		break;
	case SB_PAGEUP:
		TempLocationExpected -= 10;
		if (TempLocationExpected < 0)
		{
			TempLocationExpected = 0;
		}
		break;
	case SB_PAGEDOWN:
		TempLocationExpected += 10;
		if (TempLocationExpected >= ImageSizeYMeterShow)
		{
			TempLocationExpected = ImageSizeYMeterShow - 1;
		}
		break;
	case SB_TOP:
		TempLocationExpected = ImageSizeYMeterShow - 1;
		break;
	case SB_BOTTOM:
		TempLocationExpected = 0;
		break;
	case SB_THUMBPOSITION:
		TempLocationExpected = nPos;
		break;
	}
	m_ScrollBar.SetScrollPos(TempLocationExpected);
	if (FirstImage == 1)
	{

		CreateImageWithLine(TempLocationExpected);
	    ::PostMessage(hWndMain, WM_SHOWPICLINEEXPECTED, 0, 0);
	}
	
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CMeterControlDlg::OnBnClickedButtonStart()
{
	StartStates = 1;
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_CANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_CLOSE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	
}


void CMeterControlDlg::OnBnClickedButtonClose()
{
	StartStates = 0;
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_CANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_CLOSE)->EnableWindow(FALSE);

}
  