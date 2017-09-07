#pragma once
#include "afxcmn.h"


// CSplashScreen 对话框

class CSplashScreen : public CDialog
{
	DECLARE_DYNAMIC(CSplashScreen)

public:
	CSplashScreen(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSplashScreen();

// 对话框数据
	enum { IDD = IDD_SPLASH_SCREEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	int m_cur_load; //loading curse

	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	CProgressCtrl m_load_process;
};
