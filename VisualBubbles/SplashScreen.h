#pragma once
#include "afxcmn.h"


// CSplashScreen �Ի���

class CSplashScreen : public CDialog
{
	DECLARE_DYNAMIC(CSplashScreen)

public:
	CSplashScreen(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSplashScreen();

// �Ի�������
	enum { IDD = IDD_SPLASH_SCREEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	int m_cur_load; //loading curse

	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	CProgressCtrl m_load_process;
};
