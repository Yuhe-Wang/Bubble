// SplashThread.cpp : 实现文件
//

#include "stdafx.h"
#include "VisualBubbles.h"
#include "SplashThread.h"


// CSplashThread

IMPLEMENT_DYNCREATE(CSplashThread, CWinThread)

CSplashThread::CSplashThread()
{
}

CSplashThread::~CSplashThread()
{
}

BOOL CSplashThread::InitInstance()
{
	//::AttachThreadInput(m_nThreadID, AfxGetApp()->m_nThreadID, TRUE );
	m_pSplashDlg=new CSplashScreen;
	m_pSplashDlg->Create(IDD_SPLASH_SCREEN);
	m_pSplashDlg->ShowWindow(SW_SHOW);

	return TRUE;
}

int CSplashThread::ExitInstance()
{
	m_pSplashDlg->DestroyWindow();
	delete m_pSplashDlg;

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CSplashThread, CWinThread)
END_MESSAGE_MAP()


// CSplashThread 消息处理程序
