
// MeterControl.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMeterControlApp:
// �йش����ʵ�֣������ MeterControl.cpp
//

class CMeterControlApp : public CWinApp
{
public:
	CMeterControlApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMeterControlApp theApp;