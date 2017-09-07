#pragma once

#include "SplashScreen.h"

// CSplashThread

class CSplashThread : public CWinThread
{
	DECLARE_DYNCREATE(CSplashThread)

protected:
	CSplashThread();           // 动态创建所使用的受保护的构造函数
	virtual ~CSplashThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	CSplashScreen* m_pSplashDlg;

protected:
	DECLARE_MESSAGE_MAP()

	
};


