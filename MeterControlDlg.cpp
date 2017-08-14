
// MeterControlDlg.cpp : 实现文件
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



// CMeterControlDlg 对话框




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
//监控广播
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


// CMeterControlDlg 消息处理程序

BOOL CMeterControlDlg::OnInitDialog()
{
	int retval;

	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//------------------------------------------------------------------
	// 获取窗口句柄
	hWndMain = AfxGetMainWnd()->m_hWnd;
	ThreadClosed = 1;

	// 初始化参数
	if (InitParameters() != 0)
	{
		MessageBox(_T("从MeterControl.ini读入初始化数据错误，系统将不能正确运行\n") + ErrInfo, _T("错误"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}

	//分配空间
	retval = AllocImages();
	if (retval != 0)
	{
		MessageBox(_T("系统内存分配错误，系统将不能正确运行\n"), _T("错误"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}

	// 流量计控制箱参数
	if (LoadMeterPara() != 0)
	{
		MessageBox(_T("读取流量计标定数据错误，系统将不能正确运行\n") + ErrInfo, _T("错误"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}

	// 准备显示当前流量设置图像
	FirstImage = 0;

	// 设置流量计标定窗口参数：滚动条，位置初值
	m_ScrollBar.SetScrollRange(80, ImageSizeYMeterShow - 50);
	m_ScrollBar.SetScrollPos(LocationExpected);

	// 打开流量控制通信端口
	if (OpenComMeter() != 0)
	{
		MessageBox(_T("流量控制通信端口打开错误，系统将不能正确运行\n") + ErrInfo, _T("错误"), MB_OK);
		::PostMessage(hWndMain, WM_CLOSE, 0, 0);
		::PostMessage(hWndMain, WM_QUIT, 0, 0);
		FreeResource();
		this->OnCancel();
		return FALSE;
	}
	//// 判断通信
	//if (TestMeterCom() != 0)
	//{
	//	MessageBox(_T("与流量控制通信错误，系统将不能正确运行\n") + ErrInfo, _T("错误"), MB_OK);
	//	//::PostMessage(hWndMain, WM_CLOSE, 0, 0);
	//	::PostMessage(hWndMain, WM_QUIT, 0, 0);
	//	FreeResource();
	//	this->OnCancel();
	//	return FALSE;
	//}

	// 初始化相机
	retval = InitCamera();
	if (retval == 1)
	{
		MessageBox(_T("初始化相机错误，系统将不能正确运行，建议检查相机，然后重新启动软件\n") + ErrInfo, _T("错误"), MB_OK);
		ErrInfo = _T("");
		if (CloseCamera() != 0)
		{
			MessageBox(_T("关闭已初始化相机过程中发生错误\n") + ErrInfo, _T("错误"), MB_OK);
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

	
	
	// 启动线程
	StartStates = 0;
	StopThread = 0;
	AfxBeginThread(ThreadWorks, this);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMeterControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMeterControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMeterControlDlg::FreeResource()
{
	// 关闭线程
	StopThread = 1;
	while (ThreadClosed == 0)
	{
		MyDoEvents();
	}

	// 关闭串口
	CloseComPorts();

	// 关闭相机
	if (CloseCamera() != 0)
	{
		MessageBox(_T("关闭相机过程中发生错误\n") + ErrInfo, _T("错误"), MB_OK);
		ErrInfo = _T("");
	}

	//释放空间
	FreeImages();
}


// 独立线程，处理OCR事务和流量计控制，将结果发送到相应窗口
UINT CMeterControlDlg::ThreadWorks(LPVOID pParam)
{
	int t1, t2;

	ThreadClosed = 0;
	while(StopThread != 1)
	{
		// 流量控制
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
//临时释放CPU控制权
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

// 显示流量计图片
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

// 显示流量计设置图片
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
		ErrInfo = _T("将标定结果保存到硬盘文件 ") + CString(filename) + _T(" 时出错\n");
		return;
	}

	LocationExpected = TempLocationExpected;
	fwrite(&LocationExpected, sizeof(int), 1, fp);
	fclose(fp);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);

	MessageBox(_T("预期流量值修改生效\n"), MB_OK);
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
  