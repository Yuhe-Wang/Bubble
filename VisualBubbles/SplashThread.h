#pragma once

#include "SplashScreen.h"

// CSplashThread

class CSplashThread : public CWinThread
{
	DECLARE_DYNCREATE(CSplashThread)

protected:
	CSplashThread();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CSplashThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	CSplashScreen* m_pSplashDlg;

protected:
	DECLARE_MESSAGE_MAP()

	
};


