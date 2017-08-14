
// MeterControlDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CMeterControlDlg �Ի���
class CMeterControlDlg : public CDialogEx
{
public:
	static UINT ThreadWorks(LPVOID pParam);
	static BOOL MyDoEvents();
	void FreeResource();
	afx_msg LRESULT OnShowPicMeter(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnShowPicLineExpected(WPARAM wParam, LPARAM lParam); 

// ����
public:
	CMeterControlDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_METERCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
