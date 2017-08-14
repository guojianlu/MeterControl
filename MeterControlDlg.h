
// MeterControlDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CMeterControlDlg 对话框
class CMeterControlDlg : public CDialogEx
{
public:
	static UINT ThreadWorks(LPVOID pParam);
	static BOOL MyDoEvents();
	void FreeResource();
	afx_msg LRESULT OnShowPicMeter(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnShowPicLineExpected(WPARAM wParam, LPARAM lParam); 

// 构造
public:
	CMeterControlDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_METERCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
		afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CScrollBar m_ScrollBar;

	afx_msg void OnClose();
	afx_msg void OnClickedButtonSet();
	afx_msg void OnClickedButtonCancel();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonClose();
};
