// SplashScren.cpp : implementation
//

#include "stdafx.h"
#include "VisualBubbles.h"
#include "SplashScreen.h"

// CSplashScreen dialog
#define MAX_PROGRESS 30

IMPLEMENT_DYNAMIC(CSplashScreen, CDialog)

CSplashScreen::CSplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScreen::IDD, pParent)
{
	
}

CSplashScreen::~CSplashScreen()
{
}

void CSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_LOADING, m_load_process);
}


BEGIN_MESSAGE_MAP(CSplashScreen, CDialog)
	
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CSplashScreen::OnTimer(UINT_PTR nIDEvent)
{
	if(m_cur_load<MAX_PROGRESS) ++m_cur_load;
	else m_cur_load=0;
	m_load_process.SetPos(m_cur_load);

	CDialog::OnTimer(nIDEvent);
}

BOOL CSplashScreen::OnInitDialog()
{
	//ModifyStyleEx(WS_EX_APPWINDOW,0);
	CDialog::OnInitDialog();

	m_load_process.SetRange(0,MAX_PROGRESS);
	m_load_process.SetPos(0);
	m_cur_load=0;
	SetTimer(2,50,NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}
